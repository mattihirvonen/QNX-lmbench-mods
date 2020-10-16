
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef  __linux
#include <sys/neutrino.h>   // Msg....()
#include <sys/time.h>       // struct timespec
#endif


#if defined(__MINGW32__)
typedef struct timespec {
        time_t   tv_sec;        /* seconds */
        long     tv_nsec;       /* nanoseconds */
} timespec_t;
#else
typedef struct timespec timespec_t;
#endif // __MINGW32__


#ifndef  __linux
// http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.lib_ref/topic/c/clockcycles.html
// extern int clock_gettime(clockid_t __clock_id, struct timespec *__tp);

int clock_gettime( clockid_t  __clock_id, struct timespec *__tp )
{
    // Note:  __clock_id == CLOCK_MONOTONIC
    #if 1
    uint64_t  nss, ns = ClockCycles();

    ns  *= 15;                 // 66 MHz == 15.1515151515... ns
    nss  = ns;
    nss /= 1000000000;         // "integer" part of seconds
    __tp->tv_sec  = nss;
    nss *= 1000000000;
    __tp->tv_nsec = ns - nss;  // CPU core do not have hard integer divide command!
    #else
    // ToDo: optimize without div command
    #endif
    return 0;
}


int gettimeofday(struct timeval *tv, void *tz)
{
    // Note:  CLOCK_MONOTONIC
    #if 1
    uint64_t  uss, us = ClockCycles();

    us  /= 66;                 // 66 MHz == 15.1515151515... ns
    uss  = us;
    uss /= 1000000;            // "integer" part of seconds
    tv->tv_sec  = uss;
    uss *= 1000000;
    tv->tv_usec = us - uss;    // CPU core do not have hard integer divide command!
    #else
    // ToDo: optimize without div command
    #endif
    return 0;
}
#endif

