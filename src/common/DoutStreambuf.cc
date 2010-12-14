// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2010 Dreamhost
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include "config.h"
#include "common/DoutStreambuf.h"
#include "common/errno.h"
#include "common/Mutex.h"

#include <values.h>
#include <assert.h>
#include <errno.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string.h>
#include <syslog.h>

///////////////////////////// Globals /////////////////////////////
// TODO: get rid of this lock using thread-local storage
extern Mutex _dout_lock;

//////////////////////// Helper functions //////////////////////////
// Try a 0-byte write to a file descriptor to see if it open.
static bool fd_is_open(int fd)
{
  char buf;
  ssize_t res = TEMP_FAILURE_RETRY(write(fd, &buf, 0));
  return (res == 0);
}

static bool empty(const char *str)
{
  if (!str)
    return true;
  if (!str[0])
    return true;
  return false;
}

static string cpp_str(const char *str)
{
  if (!str)
    return "(NULL)";
  if (str[0] == '\0')
    return "(empty)";
  return str;
}

static std::string normalize_relative(const char *from)
{
  if (from[0] == '/')
    return string(from);

  char c[512];
  char *cwd = getcwd(c, sizeof(c));
  ostringstream oss;
  oss << cwd << "/" << from;
  return oss.str();
}

/* Complain about errors even without a logfile */
static void primitive_log(const std::string &str)
{
  std::cerr << str;
  std::cerr.flush();
  syslog(LOG_USER | LOG_NOTICE, "%s", str.c_str());
}

static inline bool prio_is_visible_on_stderr(int prio)
{
  return prio <= 5;
}

static inline int dout_prio_to_syslog_prio(int prio)
{
  if (prio <= 3)
    return LOG_CRIT;
  if (prio <= 5)
    return LOG_ERR;
  if (prio <= 15)
    return LOG_WARNING;
  if (prio <= 30)
    return LOG_NOTICE;
  if (prio <= 40)
    return LOG_INFO;
  return LOG_DEBUG;
}

static int safe_write(int fd, const char *buf, signed int len)
{
  int res;

  assert(len != 0);
  while (1) {
    res = write(fd, buf, len);
    if (res < 0) {
      int err = errno;
      if (err != EINTR) {
	ostringstream oss;
	oss << __func__ << ": failed to write to fd " << fd << ": "
	    << cpp_strerror(err) << "\n";
	primitive_log(oss.str());
	return err;
      }
    }
    len -= res;
    buf += res;
    if (len <= 0)
      return 0;
  }
}

static std::string get_basename(const std::string &filename)
{
  size_t last_slash = filename.find_last_of("/");
  if (last_slash == std::string::npos)
    return filename;
  return filename.substr(last_slash + 1);
}

static std::string get_dirname(const std::string &filename)
{
  size_t last_slash = filename.find_last_of("/");
  if (last_slash == std::string::npos)
    return ".";
  if (last_slash == 0)
    return filename;
  return filename.substr(0, last_slash);
}

static int create_symlink(string oldpath, const string &newpath)
{
  // Create relative symlink if the files are in the same directory
  if (get_dirname(oldpath) == get_dirname(newpath)) {
    oldpath = string("./") + get_basename(oldpath);
  }

  while (1) {
    if (::symlink(oldpath.c_str(), newpath.c_str()) == 0)
      return 0;
    int err = errno;
    if (err == EEXIST) {
      // Handle EEXIST
      if (::unlink(newpath.c_str())) {
	err = errno;
	ostringstream oss;
	oss << __func__ << ": failed to remove '" << newpath << "': "
	    << cpp_strerror(err) << "\n";
	primitive_log(oss.str());
	return err;
      }
    }
    else {
      // Other errors
      ostringstream oss;
      oss << __func__ << ": failed to symlink(oldpath='" << oldpath
	  << "', newpath='" << newpath << "'): " << cpp_strerror(err) << "\n";
      primitive_log(oss.str());
      return err;
    }
  }
}

///////////////////////////// DoutStreambuf /////////////////////////////
template <typename charT, typename traits>
DoutStreambuf<charT, traits>::DoutStreambuf()
  : flags(0), ofd(-1)
{
  // Initialize get pointer to zero so that underflow is called on the first read.
  this->setg(0, 0, 0);

  // Initialize output_buffer
  _clear_output_buffer();
}

// This function is called when the output buffer is filled.
// In this function, the buffer should be written to wherever it should
// be written to (in this case, the streambuf object that this is controlling).
template <typename charT, typename traits>
typename DoutStreambuf<charT, traits>::int_type
DoutStreambuf<charT, traits>::overflow(DoutStreambuf<charT, traits>::int_type c)
{
  {
    // zero-terminate the buffer
    charT* end_ptr = this->pptr();
    *end_ptr++ = '\0';
    *end_ptr++ = '\0';
//    char buf[1000];
//    hex2str(obuf, end_ptr - obuf, buf, sizeof(buf));
//    printf("overflow buffer: '%s'\n", buf);
  }

  // Loop over all lines in the buffer.
  int prio = 100;
  charT* start = obuf;
  while (true) {
    char* end = strchrnul(start, '\n');
    if (start == end) {
      if (*start == '\0')
	break;
      // skip zero-length lines
      ++start;
      continue;
    }
    if (*start == '\1') {
      // Decode some control characters
      ++start;
      unsigned char tmp = *((unsigned char*)start);
      prio = tmp - 11;
      ++start;
    }
    *end = '\n';
    char next = *(end+1);
    *(end+1) = '\0';

    // Now 'start' points to a NULL-terminated string, which we want to
    // output with priority 'prio'
    int len = strlen(start);
    if (flags & DOUTSB_FLAG_SYSLOG) {
      //printf("syslogging: '%s' len=%d\n", start, len);
      syslog(LOG_USER | dout_prio_to_syslog_prio(prio), "%s", start);
    }
    if (flags & DOUTSB_FLAG_STDOUT) {
      // Just write directly out to the stdout fileno. There's no point in
      // using something like fputs to write to a temporary buffer,
      // because we would just have to flush that temporary buffer
      // immediately.
      if (safe_write(STDOUT_FILENO, start, len))
	flags &= ~DOUTSB_FLAG_STDOUT;
    }
    if (flags & DOUTSB_FLAG_STDERR) {
      // Only write to stderr if the message is important enough.
      if (prio_is_visible_on_stderr(prio)) {
	if (safe_write(STDERR_FILENO, start, len))
	  flags &= ~DOUTSB_FLAG_STDERR;
      }
    }
    if (flags & DOUTSB_FLAG_OFILE) {
      if (safe_write(ofd, start, len))
	flags &= ~DOUTSB_FLAG_OFILE;
    }

    *(end+1) = next;
    start = end + 1;
  }

  _clear_output_buffer();

  // A value different than EOF (or traits::eof() for other traits) signals success.
  // If the function fails, either EOF (or traits::eof() for other traits) is returned or an
  // exception is thrown.
  return traits_ty::not_eof(c);
}

template <typename charT, typename traits>
void DoutStreambuf<charT, traits>::handle_stderr_closed()
{
  assert(_dout_lock.is_locked());
  flags &= ~DOUTSB_FLAG_STDERR;
}

template <typename charT, typename traits>
void DoutStreambuf<charT, traits>::handle_stdout_closed()
{
  assert(_dout_lock.is_locked());
  flags &= ~DOUTSB_FLAG_STDOUT;
}

template <typename charT, typename traits>
void DoutStreambuf<charT, traits>::read_global_config()
{
  assert(_dout_lock.is_locked());
  flags = 0;

  if (g_conf.log_to_syslog) {
    flags |= DOUTSB_FLAG_SYSLOG;
  }
  if (g_conf.log_to_stdout) {
    if (fd_is_open(STDOUT_FILENO)) {
      flags |= DOUTSB_FLAG_STDOUT;
    }
  }
  if (fd_is_open(STDERR_FILENO)) {
    flags |= DOUTSB_FLAG_STDERR;
  }
  if (g_conf.log_to_file) {
    if (_read_ofile_config()) {
      flags |= DOUTSB_FLAG_OFILE;
    }
  }
}

template <typename charT, typename traits>
void DoutStreambuf<charT, traits>::
set_flags(int flags_)
{
  assert(_dout_lock.is_locked());
  flags = flags_;
}

template <typename charT, typename traits>
void DoutStreambuf<charT, traits>::
set_prio(int prio)
{
  charT* p = this->pptr();
  *p++ = '\1';
  unsigned char val = (prio + 11);
  *p++ = val;
  this->pbump(2);
}

template <typename charT, typename traits>
int DoutStreambuf<charT, traits>::handle_pid_change()
{
  assert(_dout_lock.is_locked());
  if (!(flags & DOUTSB_FLAG_OFILE))
    return 0;

  string new_opath(_calculate_opath());
  if (opath == new_opath)
    return 0;

  if (!isym_path.empty()) {
    // Re-create the instance symlink
    int ret = create_symlink(new_opath, isym_path);
    if (ret) {
      ostringstream oss;
      oss << __func__ << ": failed to (re)create instance symlink";
      primitive_log(oss.str());
      return ret;
    }
  }

  if (!rsym_path.empty()) {
    // Re-create the rank symlink
    int ret = create_symlink(new_opath, rsym_path);
    if (ret) {
      ostringstream oss;
      oss << __func__ << ": failed to (re)create rank symlink";
      primitive_log(oss.str());
      return ret;
    }
  }

  int ret = ::rename(opath.c_str(), new_opath.c_str());
  if (ret) {
    int err = errno;
    ostringstream oss;
    oss << __func__ << ": failed to rename '" << opath << "' to "
        << "'" << new_opath << "': " << cpp_strerror(err) << "\n";
    primitive_log(oss.str());
    return err;
  }

  opath = new_opath;

  return 0;
}

template <typename charT, typename traits>
int DoutStreambuf<charT, traits>::create_rank_symlink(int n)
{
  assert(_dout_lock.is_locked());

  if (!(flags & DOUTSB_FLAG_OFILE))
    return 0;

  ostringstream rss;
  std::string symlink_dir(_get_symlink_dir());
  rss << symlink_dir << "/" << g_conf.type << "." << n;
  string rsym_path_(rss.str());
  int ret = create_symlink(opath, rsym_path_);
  if (ret) {
    ostringstream oss;
    oss << __func__ << ": failed to create rank symlink with n = "
	<< n << "\n";
    primitive_log(oss.str());
    return ret;
  }

  rsym_path = rsym_path_;
  return 0;
}

template <typename charT, typename traits>
std::string DoutStreambuf<charT, traits>::config_to_str() const
{
  assert(_dout_lock.is_locked());
  ostringstream oss;
  oss << "g_conf.log_to_syslog = " << g_conf.log_to_syslog << "\n";
  oss << "g_conf.log_to_stdout = " << g_conf.log_to_stdout << "\n";
  oss << "g_conf.log_to_file = " << g_conf.log_to_file << "\n";
  oss << "g_conf.log_file = '" << cpp_str(g_conf.log_file) << "'\n";
  oss << "g_conf.log_dir = '" << cpp_str(g_conf.log_dir) << "'\n";
  oss << "g_conf.g_conf.log_per_instance = '"
      << g_conf.log_per_instance << "'\n";
  oss << "flags = 0x" << std::hex << flags << std::dec << "\n";
  oss << "ofd = " << ofd << "\n";
  oss << "opath = '" << opath << "'\n";
  oss << "isym_path = '" << isym_path << "'\n";
  oss << "rsym_path = '" << rsym_path << "'\n";
  oss << "log_sym_history = " << g_conf.log_sym_history  << "\n";
  return oss.str();
}

// This is called to flush the buffer.
// This is called when we're done with the file stream (or when .flush() is called).
template <typename charT, typename traits>
typename DoutStreambuf<charT, traits>::int_type
DoutStreambuf<charT, traits>::sync()
{
  //std::cout << "flush!" << std::endl;
  typename DoutStreambuf<charT, traits>::int_type
    ret(this->overflow(traits_ty::eof()));
  if (ret == traits_ty::eof())
    return -1;

  return 0;
}

template <typename charT, typename traits>
typename DoutStreambuf<charT, traits>::int_type
DoutStreambuf<charT, traits>::underflow()
{
  // We can't read from this
  // TODO: some more elegant way of preventing callers from trying to get input from this stream
  assert(0);
}

template <typename charT, typename traits>
void DoutStreambuf<charT, traits>::_clear_output_buffer()
{
  // Set up the put pointer.
  // Overflow is called when this buffer is filled
  this->setp(obuf, obuf + OBUF_SZ - 5);
}

template <typename charT, typename traits>
std::string DoutStreambuf<charT, traits>::_calculate_opath() const
{
  assert(_dout_lock.is_locked());

  // If g_conf.log_file was specified, that takes the highest priority
  if (!empty(g_conf.log_file)) {
    return normalize_relative(g_conf.log_file);
  }

  string log_dir;
  if (empty(g_conf.log_dir))
    log_dir = normalize_relative(".");
  else
    log_dir = normalize_relative(g_conf.log_dir);

  if (g_conf.log_per_instance) {
    char hostname[255];
    memset(hostname, 0, sizeof(hostname));
    int ret = gethostname(hostname, sizeof(hostname));
    if (ret) {
      int err = errno;
      ostringstream oss;
      oss << __func__ << ": error calling gethostname: " << cpp_strerror(err) << "\n";
      primitive_log(oss.str());
      return "";
    }
    ostringstream oss;
    oss << log_dir << "/" << hostname << "." << getpid();
    return oss.str();
  }
  else {
    ostringstream oss;
    oss << log_dir << "/" << g_conf.type << "." << g_conf.id << ".log";
    return oss.str();
  }
}

template <typename charT, typename traits>
std::string DoutStreambuf<charT, traits>::_get_symlink_dir() const
{
  if (!empty(g_conf.log_sym_dir))
    return normalize_relative(g_conf.log_sym_dir);
  else
    return get_dirname(opath);
}

template <typename charT, typename traits>
bool DoutStreambuf<charT, traits>::_read_ofile_config()
{
  int ret;

  isym_path = "";
  rsym_path = "";
  opath = _calculate_opath();
  if (opath.empty()) {
    ostringstream oss;
    oss << __func__ << ": _calculate_opath failed.\n";
    primitive_log(oss.str());
    return false;
  }

  if (empty(g_conf.log_file) && g_conf.log_per_instance) {
    // Calculate instance symlink path (isym_path)
    ostringstream iss;
    std::string symlink_dir(_get_symlink_dir());
    iss << symlink_dir << "/" << g_conf.type << "." << g_conf.id;
    isym_path = iss.str();

    // Rotate isym_path
    ret = _rotate_files(isym_path);
    if (ret) {
      ostringstream oss;
      oss << __func__ << ": failed to rotate instance symlinks";
      primitive_log(oss.str());
      return ret;
    }

    // Create isym_path
    ret = create_symlink(opath, isym_path);
    if (ret) {
      ostringstream oss;
      oss << __func__ << ": failed to create instance symlink";
      primitive_log(oss.str());
      return ret;
    }
  }

  assert(ofd == -1);
  ofd = open(opath.c_str(),
	    O_CREAT | O_WRONLY | O_APPEND, S_IWUSR | S_IRUSR);
  if (ofd < 0) {
    int err = errno;
    ostringstream oss;
    oss << "failed to open log file '" << opath << "': "
	<< cpp_strerror(err) << "\n";
    primitive_log(oss.str());
    return false;
  }

  return true;
}

template <typename charT, typename traits>
int DoutStreambuf<charT, traits>::_rotate_files(const std::string &base)
{
  // Given a file name base, and a directory like this:
  // base
  // base.1
  // base.2
  // base.3
  // base.4
  // unrelated_blah
  // unrelated_blah.1
  //
  // We'll take the following actions:
  // base		  rename to base.1
  // base.1		  rename to base.2
  // base.2		  rename to base.3
  // base.3		  (unlink)
  // base.4		  (unlink)
  // unrelated_blah	  (do nothing)
  // unrelated_blah.1	  (do nothing)

  signed int i;
  for (i = -1; i < INT_MAX; ++i) {
    ostringstream oss;
    oss << base;
    if (i != -1)
      oss << "." << i;
    string path(oss.str());

    if (::access(path.c_str(), R_OK | W_OK))
      break;
  }
  signed int max_symlink = i - 1;

  for (signed int j = max_symlink; j >= -1; --j) {
    ostringstream oss;
    oss << base;
    if (j != -1)
      oss << "." << j;
    string path(oss.str());

    signed int k = j + 1;
    if (k >= g_conf.log_sym_history) {
      if (::unlink(path.c_str())) {
	int err = errno;
	ostringstream ess;
	ess << __func__ << ": failed to unlink '" << path << "': "
	    << cpp_strerror(err) << "\n";
	primitive_log(ess.str());
	return err;
      }
      //*_dout << "---- " << getpid() << " removed " << path << " ----"
      //       << std::endl;
    }
    else {
      ostringstream pss;
      pss << base << "." << k;
      string new_path(pss.str());
      if (::rename(path.c_str(), new_path.c_str())) {
	int err = errno;
	ostringstream ess;
	ess << __func__ << ": failed to rename '" << path << "' to "
	    << "'" << new_path << "': " << cpp_strerror(err) << "\n";
	primitive_log(ess.str());
	return err;
      }
//      *_dout << "---- " << getpid() << " renamed " << path << " -> "
//      << newpath << " ----" << std::endl;
    }
  }
  return 0;
}

// Explicit template instantiation
template class DoutStreambuf <char>;