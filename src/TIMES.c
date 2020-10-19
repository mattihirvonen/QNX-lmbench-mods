//
// Better result resolution function replacement(s) for standard library
// to measure short time periods more accurate in QNX application
// (like unmodified lmbench tests require).
//
// Timing results have small timing errors due simple integer arithmetic.
// Examples with some ARM iMX6 IPG clock speed(s) show 1% measurement error:
// - 66.0 MHz -> 15.1515151515... ns per clock pulse rounded to 15.0 ns
// - 49.5 MHz -> 20.2020202020... ns per clock pulse rounded to 20.0 ns
//
// These functions are suitable for in source code written performance,
// response or latency time measurements, but not in normal application
// calendar / wall clock time usage !!! Use QNX ClockPulses() function
// if possible for this kind performance measurements.
//
// Replace QNX functions (with only CLOCK_MONOTONIC support):
// - clock_getres()
// - clock_gettime()
// - gettimeofday()
// - clock()
//
#define   __STDC_FORMAT_MACROS
#include  <inttypes.h>

#include  <stdio.h>
#include  <stdlib.h>
#include  <time.h>

#ifndef   __linux
#include  <sys/neutrino.h>   // Msg....()
#include  <sys/time.h>       // struct timespec
#endif

#include  "TIMES.h"

#ifdef    QNX_SLEEPTEST
#define   clock_getres    CLOCK_GETRES
#define   clock_gettime   CLOCK_GETTIME
#define   gettimeofday    GETTIMEOFDAY
#endif // QNX_SLEEPTEST

//----------------------------------------------------------------------------------------------------------

void diff_tv( struct timeval *diff, struct timeval *tv_start, struct timeval *tv_end )
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


void diff_tp( struct timespec *diff, struct timespec *tp_start, struct timespec *tp_end )
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

int64_t diff_tp_ns( struct timespec *tp_start, struct timespec *tp_end )
{
    struct timespec  diff;
    int64_t          ns;

    diff_tp( &diff, tp_start, tp_end );

    ns  = diff.tv_sec;
    ns *= 1000000000;
    ns += diff.tv_nsec;

    return ns;
}

int64_t diff_tp_us( struct timespec *tp_start, struct timespec *tp_end )
{
    struct timespec  diff;
    int64_t          us;

    diff_tp( &diff, tp_start, tp_end );

    us  = diff.tv_sec;
    us *= 1000000;
    us += diff.tv_nsec / 1000;

    return us;
}

int64_t diff_tp_ms( struct timespec *tp_start, struct timespec *tp_end )
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
static uint64_t  MHz;


int clock_getres( clockid_t   __clock_id, struct timespec *__tp )
{
    if (!ns_per_cp) {
         ns_per_cp = 1000000000UL / SYSPAGE_ENTRY(qtime)->cycles_per_sec;
    }
    if (__tp) {
        __tp->tv_nsec = ns_per_cp;
        __tp->tv_sec  = 0;
    }
    return 0;
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


/*
To determine the time in seconds, the value returned by clock()
should be divided by the value of the macro CLOCKS_PER_SEC.
CLOCKS_PER_SEC is defined to be one million in <time.h>.
*/
clock_t clock( void )
{
    // Note:  CLOCK_MONOTONIC
    uint64_t  us;

    if (!MHz) {
         MHz = SYSPAGE_ENTRY(qtime)->cycles_per_sec / 1000000UL;
    }
    us = ClockCycles() / MHz;
    return us;
}

#endif // __linux
