// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software 
 * Foundation.  See file COPYING.
 * 
 */


#ifndef CEPH_FILESTORE_H
#define CEPH_FILESTORE_H

#include "ObjectStore.h"
#include "JournalingObjectStore.h"

#include "common/Timer.h"
#include "common/WorkQueue.h"

#include "common/Mutex.h"
#include "HashIndex.h"
#include "IndexManager.h"

#include "Fake.h"

#include <map>
#include <deque>
using namespace std;

#include <ext/hash_map>
using namespace __gnu_cxx;


// fake attributes in memory, if we need to.

class FileStore : public JournalingObjectStore,
                  public md_config_obs_t
{
  static const uint32_t on_disk_version = 2;
  string basedir, journalpath;
  std::string current_fn;
  std::string current_op_seq_fn;
  uint64_t fsid;
  
  bool btrfs;
  uint64_t blk_size;
  bool btrfs_trans_start_end;
  bool btrfs_clone_range;
  bool btrfs_snap_create;
  bool btrfs_snap_destroy;
  bool btrfs_snap_create_v2;
  bool btrfs_wait_sync;
  bool ioctl_fiemap;
  int fsid_fd, op_fd;

  int basedir_fd, current_fd;
  deque<uint64_t> snaps;

  // fake attrs?
  FakeAttrs attrs;
  bool fake_attrs;

  // fake collections?
  FakeCollections collections;
  bool fake_collections;

  // Indexed Collections
  IndexManager index_manager;
  int get_index(coll_t c, Index *index);
  int init_index(coll_t c);
  
  Finisher ondisk_finisher;

  // helper fns
  int get_cdir(coll_t cid, char *s, int len);
  
  int lock_fsid();

  // sync thread
  Mutex lock;
  bool force_sync;
  Cond sync_cond;
  uint64_t sync_epoch;

  Mutex sync_entry_timeo_lock;
  SafeTimer timer;

  list<Context*> sync_waiters;
  bool stop;
  void sync_entry();
  struct SyncThread : public Thread {
    FileStore *fs;
    SyncThread(FileStore *f) : fs(f) {}
    void *entry() {
      fs->sync_entry();
      return 0;
    }
  } sync_thread;

  void sync_fs(); // actuall sync underlying fs

  // -- op workqueue --
  struct Op {
    uint64_t op;
    list<Transaction*> tls;
    Context *onreadable, *onreadable_sync;
    uint64_t ops, bytes;
  };
  class OpSequencer : public Sequencer_impl {
    Mutex qlock; // to protect q, for benefit of flush (peek/dequeue also protected by lock)
    list<Op*> q;
    list<uint64_t> jq;
    Cond cond;
  public:
    Sequencer *parent;
    Mutex apply_lock;  // for apply mutual exclusion
    
    void queue_journal(uint64_t s) {
      Mutex::Locker l(qlock);
      jq.push_back(s);
    }
    void dequeue_journal() {
      Mutex::Locker l(qlock);
      jq.pop_front();
      cond.Signal();
    }
    void queue(Op *o) {
      Mutex::Locker l(qlock);
      q.push_back(o);
    }
    Op *peek_queue() {
      assert(apply_lock.is_locked());
      return q.front();
    }
    Op *dequeue() {
      assert(apply_lock.is_locked());
      Mutex::Locker l(qlock);
      Op *o = q.front();
      q.pop_front();
      cond.Signal();
      return o;
    }
    void flush() {
      Mutex::Locker l(qlock);

      // get max for journal _or_ op queues
      uint64_t seq = 0;
      if (!q.empty())
	seq = q.back()->op;
      if (!jq.empty() && jq.back() > seq)
	seq = jq.back();

      if (seq) {
	// everything prior to our watermark to drain through either/both queues
	while ((!q.empty() && q.front()->op <= seq) ||
	       (!jq.empty() && jq.front() <= seq))
	  cond.Wait(qlock);
      }
    }

    OpSequencer() : qlock("FileStore::OpSequencer::qlock", false, false),
		    apply_lock("FileStore::OpSequencer::apply_lock", false, false) {}
    ~OpSequencer() {
      assert(q.empty());
    }
  };
  Sequencer default_osr;
  deque<OpSequencer*> op_queue;
  uint64_t op_queue_len, op_queue_bytes;
  Cond op_throttle_cond;
  Finisher op_finisher;
  uint64_t next_finish;
  map<uint64_t, pair<Context*,Context*> > finish_queue;

  ThreadPool op_tp;
  struct OpWQ : public ThreadPool::WorkQueue<OpSequencer> {
    FileStore *store;
    OpWQ(FileStore *fs, time_t timeout, time_t suicide_timeout, ThreadPool *tp)
      : ThreadPool::WorkQueue<OpSequencer>("FileStore::OpWQ", timeout, suicide_timeout, tp), store(fs) {}

    bool _enqueue(OpSequencer *osr) {
      store->op_queue.push_back(osr);
      return true;
    }
    void _dequeue(OpSequencer *o) {
      assert(0);
    }
    bool _empty() {
      return store->op_queue.empty();
    }
    OpSequencer *_dequeue() {
      if (store->op_queue.empty())
	return NULL;
      OpSequencer *osr = store->op_queue.front();
      store->op_queue.pop_front();
      return osr;
    }
    void _process(OpSequencer *osr) {
      store->_do_op(osr);
    }
    void _process_finish(OpSequencer *osr) {
      store->_finish_op(osr);
    }
    void _clear() {
      assert(store->op_queue.empty());
    }
  } op_wq;

  void _do_op(OpSequencer *o);
  void _finish_op(OpSequencer *o);
  Op *build_op(list<Transaction*>& tls,
	       Context *onreadable, Context *onreadable_sync);
  void queue_op(OpSequencer *osr, Op *o);
  void op_queue_reserve_throttle(Op *o);
  void _op_queue_reserve_throttle(Op *o, const char *caller = 0);
  void _op_queue_release_throttle(Op *o);
  void _journaled_ahead(OpSequencer *osr, Op *o, Context *ondisk);
  friend class C_JournaledAhead;

  // flusher thread
  Cond flusher_cond;
  list<uint64_t> flusher_queue;
  int flusher_queue_len;
  void flusher_entry();
  struct FlusherThread : public Thread {
    FileStore *fs;
    FlusherThread(FileStore *f) : fs(f) {}
    void *entry() {
      fs->flusher_entry();
      return 0;
    }
  } flusher_thread;
  bool queue_flusher(int fd, uint64_t off, uint64_t len);

  int open_journal();


  PerfCounters *logger;

public:
  void start_logger(int whoami, utime_t tare);
  void stop_logger();

  int lfn_find(coll_t cid, const hobject_t& oid, IndexedPath *path);
  int lfn_getxattr(coll_t cid, const hobject_t& oid, const char *name, void *val, size_t size);
  int lfn_setxattr(coll_t cid, const hobject_t& oid, const char *name, const void *val, size_t size);
  int lfn_removexattr(coll_t cid, const hobject_t& oid, const char *name);
  int lfn_listxattr(coll_t cid, const hobject_t& oid, char *names, size_t len);
  int lfn_truncate(coll_t cid, const hobject_t& oid, off_t length);
  int lfn_stat(coll_t cid, const hobject_t& oid, struct stat *buf);
  int lfn_open(coll_t cid, const hobject_t& oid, int flags, mode_t mode);
  int lfn_open(coll_t cid, const hobject_t& oid, int flags);
  int lfn_link(coll_t c, coll_t cid, const hobject_t& o) ;
  int lfn_unlink(coll_t cid, const hobject_t& o);

 public:
  FileStore(const std::string &base, const std::string &jdev);

  int _detect_fs();
  int _sanity_check_fs();
  
  bool test_mount_in_use();
  int write_version_stamp();
  int version_stamp_is_valid(uint32_t *version);
  int update_version_stamp();
  int read_op_seq(const char *fn, uint64_t *seq);
  int write_op_seq(int, uint64_t seq);
  int mount();
  int umount();
  int get_max_object_name_length();
  int wipe_subvol(const char *s);
  int mkfs();
  int mkjournal();

  int statfs(struct statfs *buf);

  int do_transactions(list<Transaction*> &tls, uint64_t op_seq);
  unsigned apply_transaction(Transaction& t, Context *ondisk=0);
  unsigned apply_transactions(list<Transaction*>& tls, Context *ondisk=0);
  int _transaction_start(uint64_t bytes, uint64_t ops);
  void _transaction_finish(int id);
  unsigned _do_transaction(Transaction& t);

  int queue_transaction(Sequencer *osr, Transaction* t);
  int queue_transactions(Sequencer *osr, list<Transaction*>& tls, Context *onreadable, Context *ondisk=0,
			 Context *onreadable_sync=0);

  // ------------------
  // objects
  int pick_object_revision_lt(hobject_t& oid) {
    return 0;
  }
  bool exists(coll_t cid, const hobject_t& oid);
  int stat(coll_t cid, const hobject_t& oid, struct stat *st);
  int read(coll_t cid, const hobject_t& oid, uint64_t offset, size_t len, bufferlist& bl);
  int fiemap(coll_t cid, const hobject_t& oid, uint64_t offset, size_t len, bufferlist& bl);

  int _touch(coll_t cid, const hobject_t& oid);
  int _write(coll_t cid, const hobject_t& oid, uint64_t offset, size_t len, const bufferlist& bl);
  int _zero(coll_t cid, const hobject_t& oid, uint64_t offset, size_t len);
  int _truncate(coll_t cid, const hobject_t& oid, uint64_t size);
  int _clone(coll_t cid, const hobject_t& oldoid, const hobject_t& newoid);
  int _clone_range(coll_t cid, const hobject_t& oldoid, const hobject_t& newoid, uint64_t srcoff, uint64_t len, uint64_t dstoff);
  int _do_clone_range(int from, int to, uint64_t srcoff, uint64_t len, uint64_t dstoff);
  int _do_copy_range(int from, int to, uint64_t srcoff, uint64_t len, uint64_t dstoff);
  int _remove(coll_t cid, const hobject_t& oid);

  void _start_sync();

  void start_sync();
  void start_sync(Context *onsafe);
  void sync();
  void _flush_op_queue();
  void flush();
  void sync_and_flush();

  int snapshot(const string& name);

  // attrs
  int getattr(coll_t cid, const hobject_t& oid, const char *name, void *value, size_t size);
  int getattr(coll_t cid, const hobject_t& oid, const char *name, bufferptr &bp);
  int getattrs(coll_t cid, const hobject_t& oid, map<string,bufferptr>& aset, bool user_only = false);

  int _getattr(coll_t cid, const hobject_t& oid, const char *name, bufferptr& bp);
  int _getattrs(coll_t cid, const hobject_t& oid, map<string,bufferptr>& aset, bool user_only = false) ;
  int _getattr(const char *fn, const char *name, bufferptr& bp);
  int _getattrs(const char *fn, map<string,bufferptr>& aset, bool user_only = false);

  int _setattr(coll_t cid, const hobject_t& oid, const char *name, const void *value, size_t size);
  int _setattrs(coll_t cid, const hobject_t& oid, map<string,bufferptr>& aset);
  int _rmattr(coll_t cid, const hobject_t& oid, const char *name);
  int _rmattrs(coll_t cid, const hobject_t& oid);

  int collection_getattr(coll_t c, const char *name, void *value, size_t size);
  int collection_getattr(coll_t c, const char *name, bufferlist& bl);
  int collection_getattrs(coll_t cid, map<string,bufferptr> &aset);

  int _collection_setattr(coll_t c, const char *name, const void *value, size_t size);
  int _collection_rmattr(coll_t c, const char *name);
  int _collection_setattrs(coll_t cid, map<string,bufferptr> &aset);
  int _collection_rename(const coll_t &cid, const coll_t &ncid);

  // collections
  int list_collections(vector<coll_t>& ls);
  int collection_version_current(coll_t c, uint32_t *version);
  int collection_stat(coll_t c, struct stat *st);
  bool collection_exists(coll_t c);
  bool collection_empty(coll_t c);
  int collection_list_partial(coll_t c, snapid_t seq, vector<hobject_t>& o, int count, collection_list_handle_t *handle);
  int collection_list(coll_t c, vector<hobject_t>& o);

  int _create_collection(coll_t c);
  int _destroy_collection(coll_t c);
  int _collection_add(coll_t c, coll_t ocid, const hobject_t& o);
  int _collection_remove(coll_t c, const hobject_t& o);

  void trim_from_cache(coll_t cid, const hobject_t& oid, uint64_t offset, size_t len) {}
  int is_cached(coll_t cid, const hobject_t& oid, uint64_t offset, size_t len) { return -1; }

  virtual const char** get_tracked_conf_keys() const;
  virtual void handle_conf_change(const struct md_config_t *conf,
			  const std::set <std::string> &changed);
private:
  bool m_filestore_btrfs_clone_range;
  bool m_filestore_btrfs_snap;
  bool m_filestore_btrfs_trans;
  bool m_filestore_fake_attrs;
  bool m_filestore_fake_collections;
  float m_filestore_commit_timeout;
  bool m_filestore_fiemap;
  bool m_filestore_flusher;
  bool m_filestore_fsync_flushes_journal_data;
  bool m_filestore_journal_parallel;
  bool m_filestore_journal_trailing;
  bool m_filestore_journal_writeahead;
  std::string m_filestore_dev;
  int m_filestore_fiemap_threshold;
  bool m_filestore_sync_flush;
  int m_filestore_flusher_max_fds;
  double m_filestore_max_sync_interval;
  double m_filestore_min_sync_interval;
  bool m_filestore_update_collections;
  bool m_journal_dio;
  std::string m_osd_rollback_to_cluster_snap;
  bool m_osd_use_stale_snap;
  int m_filestore_queue_max_ops;
  int m_filestore_queue_max_bytes;
  int m_filestore_queue_committing_max_ops;
  int m_filestore_queue_committing_max_bytes;
};

#endif
