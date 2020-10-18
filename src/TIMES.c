
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

#ifdef    QNX_SLEEPTEST
#define   clock_getres    CLOCK_GETRES
#define   clock_gettime   CLOCK_GETTIME
#define   gettimeofday    GETTIMEOFDAY
#endif // QNX_SLEEPTEST

//----------------------------------------------------------------------------------------------------------

void diff_tv( struct timeval *diff, const struct timeval *tv_start, const struct timeval *tv_end )
{
    if (tv_start->tv_usec > tv_end->tv_usec)
    {
        diff->tv_sec  = tv_end->tv_sec - tv_start->tv_sec - 1;
        diff->tv_usec = 1000000 - (tv_start->tv_usec - tv_end->tv_usec);
    }
    else
    {
        diff->tv_sec  = tv_end->tv_sec  - tv_start->tv_sec;
        diff->tv_usec = tv_end->tv_usec - tv_start->tv_usec;
    }
}


void diff_tp( struct timespec *diff, const struct timespec *tp_start, const struct timespec *tp_end )
{
    if (tp_start->tv_nsec > tp_end->tv_nsec)
    {
        diff->tv_sec  = tp_end->tv_sec - tp_start->tv_sec - 1;
        diff->tv_nsec = 1000000000 - (tp_start->tv_nsec - tp_end->tv_nsec);
    }
    else
    {
        diff->tv_sec  = tp_end->tv_sec  - tp_start->tv_sec;
        diff->tv_nsec = tp_end->tv_nsec - tp_start->tv_nsec;
    }
}

int64_t diff_tp_ns( const struct timespec *tp_start, const struct timespec *tp_end )
{
    struct timespec  diff;
    int64_t          ns;

    diff_tp( &diff, tp_start, tp_end );

    ns  = diff.tv_sec;
    ns *= 1000000000;
    ns += diff.tv_nsec;

    return ns;
}

int64_t diff_tp_us( const struct timespec *tp_start, const struct timespec *tp_end )
{
    struct timespec  diff;
    int64_t          us;

    diff_tp( &diff, tp_start, tp_end );

    us  = diff.tv_sec;
    us *= 1000000;
    us += diff.tv_nsec / 1000;

    return us;
}

int64_t diff_tp_ms( const struct timespec *tp_start, const struct timespec *tp_end )
{
    struct timespec  diff;
    int64_t          ms;

    diff_tp( &diff, tp_start, tp_end );

    ms  = diff.tv_sec;
    ms *= 1000;
    ms += diff.tv_nsec / 1000000;

    return ms;
}

//----------------------------------------------------------------------------------------------------------

#ifndef  __linux
// http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.lib_ref/topic/c/clockcycles.html
// extern int clock_gettime(clockid_t __clock_id, struct timespec *__tp);

#include <sys/syspage.h>
/* find out how many cycles per second  */
/* SYSPAGE_ENTRY(qtime)->cycles_per_sec */
static uint64_t  cps, ns_per_cp;


int clock_getres( clockid_t   __clock_id, struct timespec *__tp )
{
    if (!ns_per_cp) {
         ns_per_cp = 1000000000UL / SYSPAGE_ENTRY(qtime)->cycles_per_sec;
    }
    return ns_per_cp;
}


int clock_gettime( clockid_t  __clock_id, struct timespec *__tp )
{
    // Note:  __clock_id == CLOCK_MONOTONIC
    #if 1
    uint64_t  nss, ns;

    #if 1
    if (!ns_per_cp) {
         ns_per_cp = 1000000000UL / SYSPAGE_ENTRY(qtime)->cycles_per_sec;
    }
    ns   = ClockCycles() * ns_per_cp;
    #else
    ns   = ClockCycles() * 15; // 66 MHz == 15.1515151515... ns
    #endif
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
    static uint64_t  MHz;

    // Note:  CLOCK_MONOTONIC
    #if 1
    uint64_t  us, uss;

    #if 1
    if (!MHz) {
         MHz = SYSPAGE_ENTRY(qtime)->cycles_per_sec / 1000000UL;
    }
    us   = ClockCycles() / MHz;
    #else
    us   = ClockCycles() / 66; // 66 MHz == 15.1515151515... ns
    #endif
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

#endif // __linux
