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

/* note: no header guard */
OPTION(host, OPT_STR, "localhost")
OPTION(public_addr, OPT_ADDR, entity_addr_t())
OPTION(cluster_addr, OPT_ADDR, entity_addr_t())
OPTION(num_client, OPT_INT, 1)
OPTION(monmap, OPT_STR, "")
OPTION(mon_host, OPT_STR, "")
OPTION(daemonize, OPT_BOOL, false)
OPTION(admin_socket, OPT_STR, "")
OPTION(log_file, OPT_STR, "")
OPTION(log_sym_dir, OPT_STR, "")
OPTION(log_sym_history, OPT_INT, 10)
OPTION(log_to_stderr, OPT_INT, LOG_TO_STDERR_ALL)
OPTION(log_to_syslog, OPT_BOOL, false)
OPTION(log_per_instance, OPT_BOOL, false)
OPTION(clog_to_monitors, OPT_BOOL, true)
OPTION(clog_to_syslog, OPT_BOOL, false)
OPTION(pid_file, OPT_STR, "")
OPTION(chdir, OPT_STR, "/")
OPTION(max_open_files, OPT_LONGLONG, 0)
OPTION(debug, OPT_INT, 0)
OPTION(debug_lockdep, OPT_INT, 0)
OPTION(debug_context, OPT_INT, 0)
OPTION(debug_mds, OPT_INT, 1)
OPTION(debug_mds_balancer, OPT_INT, 1)
OPTION(debug_mds_locker, OPT_INT, 1)
OPTION(debug_mds_log, OPT_INT, 1)
OPTION(debug_mds_log_expire, OPT_INT, 1)
OPTION(debug_mds_migrator, OPT_INT, 1)
OPTION(debug_buffer, OPT_INT, 0)
OPTION(debug_timer, OPT_INT, 0)
OPTION(debug_filer, OPT_INT, 0)
OPTION(debug_objecter, OPT_INT, 0)
OPTION(debug_rados, OPT_INT, 0)
OPTION(debug_rbd, OPT_INT, 0)
OPTION(debug_journaler, OPT_INT, 0)
OPTION(debug_objectcacher, OPT_INT, 0)
OPTION(debug_client, OPT_INT, 0)
OPTION(debug_osd, OPT_INT, 0)
OPTION(debug_filestore, OPT_INT, 1)
OPTION(debug_journal, OPT_INT, 1)
OPTION(debug_bdev, OPT_INT, 1)         // block device
OPTION(debug_ms, OPT_INT, 0)
OPTION(debug_mon, OPT_INT, 1)
OPTION(debug_monc, OPT_INT, 0)
OPTION(debug_paxos, OPT_INT, 0)
OPTION(debug_tp, OPT_INT, 0)
OPTION(debug_auth, OPT_INT, 1)
OPTION(debug_finisher, OPT_INT, 1)
OPTION(debug_heartbeatmap, OPT_INT, 1)
OPTION(key, OPT_STR, "")
OPTION(keyfile, OPT_STR, "")
OPTION(keyring, OPT_STR, "/etc/ceph/keyring,/etc/ceph/keyring.bin")
OPTION(heartbeat_interval, OPT_INT, 5)
OPTION(heartbeat_file, OPT_STR, "")
OPTION(ms_tcp_nodelay, OPT_BOOL, true)
OPTION(ms_initial_backoff, OPT_DOUBLE, .2)
OPTION(ms_max_backoff, OPT_DOUBLE, 15.0)
OPTION(ms_nocrc, OPT_BOOL, false)
OPTION(ms_die_on_bad_msg, OPT_BOOL, false)
OPTION(ms_dispatch_throttle_bytes, OPT_U64, 100 << 20)
OPTION(ms_bind_ipv6, OPT_BOOL, false)
OPTION(ms_rwthread_stack_bytes, OPT_U64, 1024 << 10)
OPTION(ms_tcp_read_timeout, OPT_U64, 900)
OPTION(ms_inject_socket_failures, OPT_U64, 0)
OPTION(mon_data, OPT_STR, "")
OPTION(mon_sync_fs_threshold, OPT_INT, 5)   // sync() when writing this many objects; 0 to disable.
OPTION(mon_tick_interval, OPT_INT, 5)
OPTION(mon_subscribe_interval, OPT_DOUBLE, 300)
OPTION(mon_osd_auto_mark_in, OPT_BOOL, true)    // automatically mark new osds 'in'
OPTION(mon_osd_down_out_interval, OPT_INT, 300) // seconds
OPTION(mon_lease, OPT_FLOAT, 5)       // lease interval
OPTION(mon_lease_renew_interval, OPT_FLOAT, 3) // on leader, to renew the lease
OPTION(mon_lease_ack_timeout, OPT_FLOAT, 10.0) // on leader, if lease isn't acked by all peons
OPTION(mon_clock_drift_allowed, OPT_FLOAT, .050) // allowed clock drift between monitors
OPTION(mon_clock_drift_warn_backoff, OPT_FLOAT, 5) // exponential backoff for clock drift warnings
OPTION(mon_accept_timeout, OPT_FLOAT, 10.0)    // on leader, if paxos update isn't accepted
OPTION(mon_pg_create_interval, OPT_FLOAT, 30.0) // no more than every 30s
OPTION(mon_osd_full_ratio, OPT_INT, 95) // what % full makes an OSD "full"
OPTION(mon_osd_nearfull_ratio, OPT_INT, 85) // what % full makes an OSD near full
OPTION(mon_globalid_prealloc, OPT_INT, 100)   // how many globalids to prealloc
OPTION(mon_osd_report_timeout, OPT_INT, 900)    // grace period before declaring unresponsive OSDs dead
OPTION(mon_force_standby_active, OPT_BOOL, true) // should mons force standby-replay mds to be active
OPTION(paxos_propose_interval, OPT_DOUBLE, 1.0)  // gather updates for this long before proposing a map update
OPTION(paxos_min_wait, OPT_DOUBLE, 0.05)  // min time to gather updates for after period of inactivity
OPTION(paxos_observer_timeout, OPT_DOUBLE, 5*60) // gather updates for this long before proposing a map update
OPTION(clock_offset, OPT_DOUBLE, 0) // how much to offset the system clock in Clock.cc
OPTION(auth_supported, OPT_STR, "none")
OPTION(auth_mon_ticket_ttl, OPT_DOUBLE, 60*60*12)
OPTION(auth_service_ticket_ttl, OPT_DOUBLE, 60*60)
OPTION(mon_client_hunt_interval, OPT_DOUBLE, 3.0)   // try new mon every N seconds until we connect
OPTION(mon_client_ping_interval, OPT_DOUBLE, 10.0)  // ping every N seconds
OPTION(client_cache_size, OPT_INT, 16384)
OPTION(client_cache_mid, OPT_FLOAT, .75)
OPTION(client_cache_stat_ttl, OPT_INT, 0) // seconds until cached stat results become invalid
OPTION(client_cache_readdir_ttl, OPT_INT, 1)  // 1 second only
OPTION(client_use_random_mds, OPT_BOOL, false)
OPTION(client_mount_timeout, OPT_DOUBLE, 30.0)
OPTION(client_unmount_timeout, OPT_DOUBLE, 10.0)
OPTION(client_tick_interval, OPT_DOUBLE, 1.0)
OPTION(client_trace, OPT_STR, "")
OPTION(client_readahead_min, OPT_LONGLONG, 128*1024)  // readahead at _least_ this much.
OPTION(client_readahead_max_bytes, OPT_LONGLONG, 0)  //8 * 1024*1024
OPTION(client_readahead_max_periods, OPT_LONGLONG, 4)  // as multiple of file layout period (object size * num stripes)
OPTION(client_snapdir, OPT_STR, ".snap")
OPTION(client_mountpoint, OPT_STR, "/")
OPTION(client_notify_timeout, OPT_INT, 10) // in seconds
OPTION(client_oc, OPT_BOOL, true)
OPTION(client_oc_size, OPT_INT, 1024*1024* 200)    // MB * n
OPTION(client_oc_max_dirty, OPT_INT, 1024*1024* 100)    // MB * n  (dirty OR tx.. bigish)
OPTION(client_oc_target_dirty, OPT_INT, 1024*1024* 8) // target dirty (keep this smallish)
// note: the max amount of "in flight" dirty data is roughly (max - target)
OPTION(client_oc_max_sync_write, OPT_U64, 128*1024)   // sync writes >= this use wrlock
OPTION(fuse_use_invalidate_cb, OPT_BOOL, false) // use fuse 2.8+ invalidate callback to keep page cache consistent
OPTION(objecter_tick_interval, OPT_DOUBLE, 5.0)
OPTION(objecter_mon_retry_interval, OPT_DOUBLE, 5.0)
OPTION(objecter_timeout, OPT_DOUBLE, 10.0)    // before we ask for a map
OPTION(objecter_inflight_op_bytes, OPT_U64, 1024*1024*100) //max in-flight data (both directions)
OPTION(journaler_allow_split_entries, OPT_BOOL, true)
OPTION(journaler_write_head_interval, OPT_INT, 15)
OPTION(journaler_prefetch_periods, OPT_INT, 10)   // * journal object size
OPTION(journaler_prezero_periods, OPT_INT, 5)     // * journal object size
OPTION(journaler_batch_interval, OPT_DOUBLE, .001)   // seconds.. max add'l latency we artificially incur
OPTION(journaler_batch_max, OPT_U64, 0)  // max bytes we'll delay flushing; disable, for now....
OPTION(mds_max_file_size, OPT_U64, 1ULL << 40)
OPTION(mds_cache_size, OPT_INT, 100000)
OPTION(mds_cache_mid, OPT_FLOAT, .7)
OPTION(mds_mem_max, OPT_INT, 1048576)        // KB
OPTION(mds_dir_commit_ratio, OPT_FLOAT, .5)
OPTION(mds_dir_max_commit_size, OPT_INT, 90) // MB
OPTION(mds_decay_halflife, OPT_FLOAT, 5)
OPTION(mds_beacon_interval, OPT_FLOAT, 4)
OPTION(mds_beacon_grace, OPT_FLOAT, 15)
OPTION(mds_blacklist_interval, OPT_FLOAT, 24.0*60.0)  // how long to blacklist failed nodes
OPTION(mds_session_timeout, OPT_FLOAT, 60)    // cap bits and leases time out if client idle
OPTION(mds_session_autoclose, OPT_FLOAT, 300) // autoclose idle session
OPTION(mds_reconnect_timeout, OPT_FLOAT, 45)  // seconds to wait for clients during mds restart
	      //  make it (mds_session_timeout - mds_beacon_grace)
OPTION(mds_tick_interval, OPT_FLOAT, 5)
OPTION(mds_dirstat_min_interval, OPT_FLOAT, 1)    // try to avoid propagating more often than this
OPTION(mds_scatter_nudge_interval, OPT_FLOAT, 5)  // how quickly dirstat changes propagate up the hierarchy
OPTION(mds_client_prealloc_inos, OPT_INT, 1000)
OPTION(mds_early_reply, OPT_BOOL, true)
OPTION(mds_use_tmap, OPT_BOOL, true)        // use trivialmap for dir updates
OPTION(mds_default_dir_hash, OPT_INT, CEPH_STR_HASH_RJENKINS)
OPTION(mds_log, OPT_BOOL, true)
OPTION(mds_log_skip_corrupt_events, OPT_BOOL, false)
OPTION(mds_log_max_events, OPT_INT, -1)
OPTION(mds_log_max_segments, OPT_INT, 30)  // segment size defined by FileLayout, above
OPTION(mds_log_max_expiring, OPT_INT, 20)
OPTION(mds_log_eopen_size, OPT_INT, 100)   // # open inodes per log entry
OPTION(mds_bal_sample_interval, OPT_FLOAT, 3.0)  // every 5 seconds
OPTION(mds_bal_replicate_threshold, OPT_FLOAT, 8000)
OPTION(mds_bal_unreplicate_threshold, OPT_FLOAT, 0)
OPTION(mds_bal_frag, OPT_BOOL, false)
OPTION(mds_bal_split_size, OPT_INT, 10000)
OPTION(mds_bal_split_rd, OPT_FLOAT, 25000)
OPTION(mds_bal_split_wr, OPT_FLOAT, 10000)
OPTION(mds_bal_split_bits, OPT_INT, 3)
OPTION(mds_bal_merge_size, OPT_INT, 50)
OPTION(mds_bal_merge_rd, OPT_FLOAT, 1000)
OPTION(mds_bal_merge_wr, OPT_FLOAT, 1000)
OPTION(mds_bal_interval, OPT_INT, 10)           // seconds
OPTION(mds_bal_fragment_interval, OPT_INT, 5)      // seconds
OPTION(mds_bal_idle_threshold, OPT_FLOAT, 0)
OPTION(mds_bal_max, OPT_INT, -1)
OPTION(mds_bal_max_until, OPT_INT, -1)
OPTION(mds_bal_mode, OPT_INT, 0)
OPTION(mds_bal_min_rebalance, OPT_FLOAT, .1)  // must be this much above average before we export anything
OPTION(mds_bal_min_start, OPT_FLOAT, .2)      // if we need less than this, we don't do anything
OPTION(mds_bal_need_min, OPT_FLOAT, .8)       // take within this range of what we need
OPTION(mds_bal_need_max, OPT_FLOAT, 1.2)
OPTION(mds_bal_midchunk, OPT_FLOAT, .3)       // any sub bigger than this taken in full
OPTION(mds_bal_minchunk, OPT_FLOAT, .001)     // never take anything smaller than this
OPTION(mds_bal_target_removal_min, OPT_INT, 5) // min balance iterations before old target is removed
OPTION(mds_bal_target_removal_max, OPT_INT, 10) // max balance iterations before old target is removed
OPTION(mds_replay_interval, OPT_FLOAT, 1.0) // time to wait before starting replay again
OPTION(mds_shutdown_check, OPT_INT, 0)
OPTION(mds_thrash_exports, OPT_INT, 0)
OPTION(mds_thrash_fragments, OPT_INT, 0)
OPTION(mds_dump_cache_on_map, OPT_BOOL, false)
OPTION(mds_dump_cache_after_rejoin, OPT_BOOL, false)
OPTION(mds_verify_scatter, OPT_BOOL, false)
OPTION(mds_debug_scatterstat, OPT_BOOL, false)
OPTION(mds_debug_frag, OPT_BOOL, false)
OPTION(mds_debug_auth_pins, OPT_BOOL, false)
OPTION(mds_debug_subtrees, OPT_BOOL, false)
OPTION(mds_kill_mdstable_at, OPT_INT, 0)
OPTION(mds_kill_export_at, OPT_INT, 0)
OPTION(mds_kill_import_at, OPT_INT, 0)
OPTION(mds_kill_link_at, OPT_INT, 0)
OPTION(mds_kill_rename_at, OPT_INT, 0)
OPTION(mds_wipe_sessions, OPT_BOOL, 0)
OPTION(mds_wipe_ino_prealloc, OPT_BOOL, 0)
OPTION(mds_skip_ino, OPT_INT, 0)
OPTION(max_mds, OPT_INT, 1)
OPTION(mds_standby_for_name, OPT_STR, "")
OPTION(mds_standby_for_rank, OPT_INT, -1)
OPTION(mds_standby_replay, OPT_BOOL, false)
OPTION(osd_data, OPT_STR, "")
OPTION(osd_journal, OPT_STR, "")
OPTION(osd_journal_size, OPT_INT, 0)         // in mb
OPTION(osd_max_write_size, OPT_INT, 90)
OPTION(osd_balance_reads, OPT_BOOL, false)
OPTION(osd_shed_reads, OPT_INT, false)     // forward from primary to replica
OPTION(osd_shed_reads_min_latency, OPT_DOUBLE, .01)       // min local latency
OPTION(osd_shed_reads_min_latency_diff, OPT_DOUBLE, .01)  // min latency difference
OPTION(osd_shed_reads_min_latency_ratio, OPT_DOUBLE, 1.5)  // 1.2 == 20% higher than peer
OPTION(osd_client_message_size_cap, OPT_U64, 500*1024L*1024L) // default to 200MB client data allowed in-memory
OPTION(osd_stat_refresh_interval, OPT_DOUBLE, .5)
OPTION(osd_pg_bits, OPT_INT, 6)  // bits per osd
OPTION(osd_pgp_bits, OPT_INT, 6)  // bits per osd
OPTION(osd_lpg_bits, OPT_INT, 2)  // bits per osd
OPTION(osd_pg_layout, OPT_INT, CEPH_PG_LAYOUT_CRUSH)
OPTION(osd_min_rep, OPT_INT, 1)
OPTION(osd_max_rep, OPT_INT, 10)
OPTION(osd_min_raid_width, OPT_INT, 3)
OPTION(osd_max_raid_width, OPT_INT, 2)
OPTION(osd_pool_default_crush_rule, OPT_INT, 0)
OPTION(osd_pool_default_size, OPT_INT, 2)
OPTION(osd_pool_default_pg_num, OPT_INT, 8)
OPTION(osd_pool_default_pgp_num, OPT_INT, 8)
OPTION(osd_map_cache_max, OPT_INT, 250)
OPTION(osd_map_message_max, OPT_INT, 100)  // max maps per MOSDMap message
OPTION(osd_op_threads, OPT_INT, 2)    // 0 == no threading
OPTION(osd_max_opq, OPT_INT, 10)
OPTION(osd_disk_threads, OPT_INT, 1)
OPTION(osd_recovery_threads, OPT_INT, 1)
OPTION(osd_op_thread_timeout, OPT_INT, 30)
OPTION(osd_backlog_thread_timeout, OPT_INT, 60*60*1)
OPTION(osd_recovery_thread_timeout, OPT_INT, 30)
OPTION(osd_snap_trim_thread_timeout, OPT_INT, 60*60*1)
OPTION(osd_scrub_thread_timeout, OPT_INT, 60)
OPTION(osd_scrub_finalize_thread_timeout, OPT_INT, 60*10)
OPTION(osd_remove_thread_timeout, OPT_INT, 60*60)
OPTION(osd_age, OPT_FLOAT, .8)
OPTION(osd_age_time, OPT_INT, 0)
OPTION(osd_heartbeat_interval, OPT_INT, 1)
OPTION(osd_mon_heartbeat_interval, OPT_INT, 30)  // if no peers, ping monitor
OPTION(osd_heartbeat_grace, OPT_INT, 20)
OPTION(osd_mon_report_interval_max, OPT_INT, 120)
OPTION(osd_mon_report_interval_min, OPT_INT, 5)  // pg stats, failures, up_thru, boot.
OPTION(osd_min_down_reporters, OPT_INT, 1)   // number of OSDs who need to report a down OSD for it to count
OPTION(osd_min_down_reports, OPT_INT, 3)     // number of times a down OSD must be reported for it to count
OPTION(osd_replay_window, OPT_INT, 45)
OPTION(osd_preserve_trimmed_log, OPT_BOOL, true)
OPTION(osd_auto_mark_unfound_lost, OPT_BOOL, false)
OPTION(osd_recovery_delay_start, OPT_FLOAT, 15)
OPTION(osd_recovery_max_active, OPT_INT, 5)
OPTION(osd_recovery_max_chunk, OPT_U64, 1<<20)  // max size of push chunk
OPTION(osd_recovery_forget_lost_objects, OPT_BOOL, false)   // off for now
OPTION(osd_max_scrubs, OPT_INT, 1)
OPTION(osd_scrub_load_threshold, OPT_FLOAT, 0.5)
OPTION(osd_scrub_min_interval, OPT_FLOAT, 300)
OPTION(osd_scrub_max_interval, OPT_FLOAT, 60*60*24)   // once a day
OPTION(osd_auto_weight, OPT_BOOL, false)
OPTION(osd_class_error_timeout, OPT_DOUBLE, 60.0)  // seconds
OPTION(osd_class_timeout, OPT_DOUBLE, 60*60.0) // seconds
OPTION(osd_class_dir, OPT_STR, "/usr/lib64/rados-classes")
OPTION(osd_check_for_log_corruption, OPT_BOOL, false)
OPTION(osd_use_stale_snap, OPT_BOOL, false)
OPTION(osd_rollback_to_cluster_snap, OPT_STR, "")
OPTION(osd_max_notify_timeout, OPT_U32, 30) // max notify timeout in seconds
OPTION(filestore, OPT_BOOL, false)
OPTION(filestore_max_sync_interval, OPT_DOUBLE, 5)    // seconds
OPTION(filestore_min_sync_interval, OPT_DOUBLE, .01)  // seconds
OPTION(filestore_fake_attrs, OPT_BOOL, false)
OPTION(filestore_fake_collections, OPT_BOOL, false)
OPTION(filestore_dev, OPT_STR, "")
OPTION(filestore_btrfs_trans, OPT_BOOL, false)
OPTION(filestore_btrfs_snap, OPT_BOOL, true)
OPTION(filestore_btrfs_clone_range, OPT_BOOL, true)
OPTION(filestore_fsync_flushes_journal_data, OPT_BOOL, false)
OPTION(filestore_fiemap, OPT_BOOL, true)     // (try to) use fiemap
OPTION(filestore_flusher, OPT_BOOL, true)
OPTION(filestore_flusher_max_fds, OPT_INT, 512)
OPTION(filestore_sync_flush, OPT_BOOL, false)
OPTION(filestore_journal_parallel, OPT_BOOL, false)
OPTION(filestore_journal_writeahead, OPT_BOOL, false)
OPTION(filestore_journal_trailing, OPT_BOOL, false)
OPTION(filestore_queue_max_ops, OPT_INT, 500)
OPTION(filestore_queue_max_bytes, OPT_INT, 100 << 20)
OPTION(filestore_queue_committing_max_ops, OPT_INT, 500)        // this is ON TOP of filestore_queue_max_*
OPTION(filestore_queue_committing_max_bytes, OPT_INT, 100 << 20) //  "
OPTION(filestore_op_threads, OPT_INT, 2)
OPTION(filestore_op_thread_timeout, OPT_INT, 60)
OPTION(filestore_op_thread_suicide_timeout, OPT_INT, 180)
OPTION(filestore_commit_timeout, OPT_FLOAT, 600)
OPTION(filestore_fiemap_threshold, OPT_INT, 4096)
OPTION(filestore_merge_threshold, OPT_INT, 10)
OPTION(filestore_split_multiple, OPT_INT, 2)
OPTION(filestore_update_collections, OPT_BOOL, false)
OPTION(journal_dio, OPT_BOOL, true)
OPTION(journal_block_align, OPT_BOOL, true)
OPTION(journal_max_write_bytes, OPT_INT, 10 << 20)
OPTION(journal_max_write_entries, OPT_INT, 100)
OPTION(journal_queue_max_ops, OPT_INT, 500)
OPTION(journal_queue_max_bytes, OPT_INT, 100 << 20)
OPTION(journal_align_min_size, OPT_INT, 64 << 10)  // align data payloads >= this.
OPTION(bdev_lock, OPT_BOOL, true)
OPTION(bdev_iothreads, OPT_INT, 1)         // number of ios to queue with kernel
OPTION(bdev_idle_kick_after_ms, OPT_INT, 100)  // ms
OPTION(bdev_el_fw_max_ms, OPT_INT, 10000)      // restart elevator at least once every 1000 ms
OPTION(bdev_el_bw_max_ms, OPT_INT, 3000)       // restart elevator at least once every 300 ms
OPTION(bdev_el_bidir, OPT_BOOL, false)          // bidirectional elevator?
OPTION(bdev_iov_max, OPT_INT, 512)            // max # iov's to collect into a single readv()/writev() call
OPTION(bdev_debug_check_io_overlap, OPT_BOOL, true)  // [DEBUG] check for any pending io overlaps
OPTION(bdev_fake_mb, OPT_INT, 0)
OPTION(bdev_fake_max_mb, OPT_INT, 0)
OPTION(debug_rgw, OPT_INT, 20)                 // log level for the Rados gateway
OPTION(rgw_cache_enabled, OPT_BOOL, false)   // rgw cache enabled
OPTION(rgw_cache_lru_size, OPT_INT, 10000)   // num of entries in rgw cache
OPTION(rgw_socket_path, OPT_STR, "")   // path to unix domain socket, if not specified, rgw will not run as external fcgi
OPTION(rgw_dns_name, OPT_STR, "")
OPTION(rgw_swift_url, OPT_STR, "")              // 
OPTION(rgw_swift_url_prefix, OPT_STR, "swift")  // 
OPTION(rgw_print_continue, OPT_BOOL, true)  // enable if 100-Continue works
OPTION(rgw_remote_addr_param, OPT_STR, "REMOTE_ADDR")  // e.g. X-Forwarded-For, if you have a reverse proxy
OPTION(rgw_op_thread_timeout, OPT_INT, 10*60)
OPTION(rgw_op_thread_suicide_timeout, OPT_INT, 60*60)
OPTION(rgw_thread_pool_size, OPT_INT, 100)
OPTION(rgw_maintenance_tick_interval, OPT_DOUBLE, 10.0)
OPTION(rgw_pools_preallocate_max, OPT_INT, 100)
OPTION(rgw_pools_preallocate_threshold, OPT_INT, 70)
OPTION(rgw_log_nonexistent_bucket, OPT_BOOL, false)
OPTION(rgw_log_object_name, OPT_STR, "%Y-%m-%d-%H-%i-%n")      // man date to see codes (a subset are supported)
OPTION(rgw_log_object_name_utc, OPT_BOOL, false)
OPTION(rgw_intent_log_object_name, OPT_STR, "%Y-%m-%d-%i-%n")  // man date to see codes (a subset are supported)
OPTION(rgw_intent_log_object_name_utc, OPT_BOOL, false)
OPTION(rbd_writeback_window, OPT_INT, 0 /*8 << 20*/) // rbd writeback window size, bytes

// This will be set to true when it is safe to start threads.
// Once it is true, it will never change.
OPTION(internal_safe_to_start_threads, OPT_BOOL, false)
