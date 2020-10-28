//
// Better result resolution function replacement(s) for standard library
// to measure short time periods more accurate in QNX application
// (like old unmodified lmbench test source(s) require).
//
// iMX6 IPG clock based GPT and EPIT timers are 32 bit length up-counter(s)
// IPG clock based counter wrap around approximately in 86 seconds (at 50 MHz).
// ClockCycles() kernel call returns the current value of a free-running 64-bit
// cycle counter. This is implemented on each processor as a high-performance
// mechanism for timing short intervals.
//
// Timing results have small timing errors due simple integer arithmetic.
// Examples with some ARM iMX6 IPG clock speed(s) show 1% measurement error:
// - 66.0 MHz -> 15.1515151515... ns per clock pulse rounded to 15.0 ns  (1%)
// - 49.5 MHz -> 20.2020202020... ns per clock pulse rounded to 20.0 ns  (1%)
// - 49.0 MHz -> 20.40816327..... ns per clock pulse rounded to 20.0 ns  (2%)
//
// Functions can made more absolute time accurate by scaling intermediate results
// different way. Example multiply ClockPulses  by x10 and divide
// "SYSPAGE_ENTRY(qtime)->cycles_per_sec" value by /10.
//
// These functions are suitable for in source code written performance,
// response or latency time measurements, but not for normal application
// calendar / wall clock time usage !!! Use QNX ClockPulses() function
// if possible for performance measurements.
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

#ifdef    __QNX__
#include  <sys/neutrino.h>   // Msg....()
#include  <sys/time.h>       // struct timespec
#endif // __QNX__

#include  "TIMES.h"

#ifdef    QNX_SLEEPTEST
#define   clock_getres    CLOCK_GETRES
#define   clock_gettime   CLOCK_GETTIME
#define   gettimeofday    GETTIMEOFDAY
#define   clock           CLOCK
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

#ifdef  __QNX__
// http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.lib_ref/topic/c/clockcycles.html
// extern int clock_gettime(clockid_t __clock_id, struct timespec *__tp);

#include <sys/syspage.h>
/* find out how many cycles per second  */
/* SYSPAGE_ENTRY(qtime)->cycles_per_sec */
static uint64_t  kHz;


int clock_getres( clockid_t   __clock_id, struct timespec *__tp )
{
    uint64_t  ns_per_cycle = 1000000000UL / SYSPAGE_ENTRY(qtime)->cycles_per_sec;

    if (__tp) {
        __tp->tv_nsec = ns_per_cycle;
        __tp->tv_sec  = 0;
    }
    return 0;
}


int clock_gettime( clockid_t  __clock_id, struct timespec *__tp )
{
    // Note(s):
    //   __clock_id == CLOCK_MONOTONIC
    //   Time result wrap around within 5 hours

    #if 1
    uint64_t  nss, ns;

    #if 1
    if (!kHz) {
         kHz = SYSPAGE_ENTRY(qtime)->cycles_per_sec / 1000ULL;
    }
    ns   = ClockCycles();
    ns  *= 1000000;
    ns  /= kHz;
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
    if (!kHz) {
         kHz = SYSPAGE_ENTRY(qtime)->cycles_per_sec / 1000ULL;
    }
    us   = ClockCycles();
    us  *= 1000;
    us  /= kHz;
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

    if (!kHz) {
         kHz = SYSPAGE_ENTRY(qtime)->cycles_per_sec / 1000ULL;
    }
    us   = ClockCycles();
    us  *= 1000;
    us  /= kHz;
    return us;
}

#endif // __QNX__
