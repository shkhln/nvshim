#include <assert.h>
#include <errno.h>
#include <pthread_np.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "../time.h"
#include "../../shim.h"

#ifdef __i386__
#define LINUX_GETPID           20
#define LINUX_GETTID          224
#define LINUX_FUTEX           240
#define LINUX_CLOCK_GETTIME   265
#define LINUX_GET_ROBUST_LIST 312
#define LINUX_MEMFD_CREATE    356
#endif

#ifdef __x86_64__
#define LINUX_GETPID           39
#define LINUX_GETTID          186
#define LINUX_FUTEX           202
#define LINUX_CLOCK_GETTIME   228
#define LINUX_GET_ROBUST_LIST 274
#define LINUX_MEMFD_CREATE    319
#endif

long shim_syscall_impl(long number, va_list args) {

  if (number == LINUX_GETPID) {

    LOG("%s: getpid()", __func__);

    pid_t pid = getpid();
    LOG("%s: getpid -> %d", __func__, pid);

    return pid;
  }

  if (number == LINUX_GETTID) {

    LOG("%s: gettid()", __func__);

    int tid = pthread_getthreadid_np();
    LOG("%s: gettid -> %d", __func__, tid);

    return tid;
  }

  if (number == LINUX_FUTEX) {
    errno = native_to_linux_errno(ENOSYS);
    return -1;
  }

  if (number == LINUX_CLOCK_GETTIME) {

    linux_clockid_t clock_id = va_arg(args, linux_clockid_t);
    linux_timespec* tp       = va_arg(args, linux_timespec*);

    LOG("%s: clock_gettime(%d, %p)", __func__, clock_id, tp);

    int err = shim_clock_gettime_impl(clock_id, tp);
    LOG("%s: clock_gettime -> %d", __func__, err);

    return err;
  }

  if (number == LINUX_GET_ROBUST_LIST) {

    typedef void robust_list_head;

    int get_robust_list(int, robust_list_head**, size_t*);

    int                pid        = va_arg(args, int);
    robust_list_head** list_head  = va_arg(args, robust_list_head**);
    size_t*            struct_len = va_arg(args, size_t*);

#if DEBUG
    LOG("%s: get_robust_list(%d, %p, %p)\n", __func__, pid, list_head, struct_len);
#else
    fprintf(stderr, "%s [get_robust_list]: nothing to see here, move along\n", __func__);
#endif

    int err = get_robust_list(pid, list_head, struct_len);
    LOG("%s: get_robust_list -> %d", __func__, err);

    return err;
  }

  if (number == LINUX_MEMFD_CREATE) {
    errno = native_to_linux_errno(ENOSYS);
    return -1;
  }

  UNIMPLEMENTED_ARGS("%ld, ...", number);
}

SHIM_WRAP(syscall);
