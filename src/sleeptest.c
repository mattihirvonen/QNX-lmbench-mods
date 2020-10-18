
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>         // strcmp()
#include <unistd.h>         // sleep()
#include <time.h>
#include <sys/time.h>
#include "TIMES.h"

#ifndef  __linux
#include <sys/neutrino.h>
#include <sys/syspage.h>    // SYSPAGE_ENTRY()
#endif

//-----------------------------------------------------------------------------------------------
// Dummy satify for  lib_timing.c::handle_scheduler()  request

int handle_scheduler(int x, int y, int z)
{
    return 0;
}

//-----------------------------------------------------------------------------------------------

void QNX_print_cps( void )
{
    #ifdef   QNX_SLEEPTEST
    uint64_t cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;

    printf("QNX ClockCycles(): clock pulses per second = %lld\n\n", cps);
    #endif
}

//-----------------------------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
    struct  timespec  tp_start, tp_end, tp_diff, tp_res;    // Std. library
    struct  timeval   tv_start, tv_end, tv_diff;            // Std. library
    #ifdef  QNX_SLEEPTEST
    struct  timespec  tp_START, tp_END, tp_DIFF, tp_RES;    // TIMES.c
    struct  timeval   tv_START, tv_END, tv_DIFF;            // TIMES.c
    #endif

    clock_getres(  CLOCK_MONOTONIC, &tp_res);               // Std. library...
    clock_gettime( CLOCK_MONOTONIC, &tp_start );
    gettimeofday( &tv_start, NULL );
    #ifdef  QNX_SLEEPTEST
    CLOCK_GETRES(  CLOCK_MONOTONIC, &tp_RES);               // TIMES.c...
    CLOCK_GETTIME( CLOCK_MONOTONIC, &tp_START );
    GETTIMEOFDAY( &tv_START, NULL );
    #endif

    QNX_print_cps();

    int  i, decimals = 0, sleeptime = 10;

    for (i = 1; i < argc; i++) {

        int delay = atoi( argv[i] );
        if (delay)
            sleeptime = delay;

        if (!strcmp("-d",argv[i]))
            decimals = 1;
    }
    printf("Sleep test %d seconds...\n", sleeptime);

    sleep( sleeptime );

    printf("...done\n");

    clock_gettime( CLOCK_MONOTONIC, &tp_end );
    gettimeofday( &tv_end, NULL );
    #ifdef  QNX_SLEEPTEST
    CLOCK_GETTIME( CLOCK_MONOTONIC, &tp_END );
    GETTIMEOFDAY( &tv_END, NULL );
    #endif

    diff_tp( &tp_diff, &tp_start, &tp_end );
    diff_tv( &tv_diff, &tv_start, &tv_end );
    #ifdef  QNX_SLEEPTEST
    diff_tp( &tp_DIFF, &tp_START, &tp_END );
    diff_tv( &tv_DIFF, &tv_START, &tv_END );
    #endif

    printf("\nResults:\n");
    if (decimals) {
        printf("- clock_getres:  %3d.%09d seconds\n",    (int)tp_res.tv_sec,  (int)tp_res.tv_nsec  );
        printf("- clock_gettime: %3d.%09d seconds\n",    (int)tp_diff.tv_sec, (int)tp_diff.tv_nsec );
        printf("- gettimeofday:  %3d.%06d    seconds\n", (int)tv_diff.tv_sec, (int)tv_diff.tv_usec );
        #ifdef  QNX_SLEEPTEST
        printf("\nResults (by ClockCycles function):\n");
        printf("- CLOCK_GETRES:  %3d.%09d seconds\n",    (int)tp_RES.tv_sec,  (int)tp_RES.tv_nsec  );
        printf("- CLOCK_GETTIME: %3d.%09d seconds\n",    (int)tp_DIFF.tv_sec, (int)tp_DIFF.tv_nsec );
        printf("- GETTIMEOFDAY:  %3d.%06d    seconds\n", (int)tv_DIFF.tv_sec, (int)tv_DIFF.tv_usec );
        #endif
    }
    else {
        printf("- clock_getres:  %7d nsec\n",(int)tp_res.tv_nsec );
        printf("- clock_gettime: %3d.%03d sec\n", (int)tp_diff.tv_sec, (int)(tp_diff.tv_nsec/1000000) );
        printf("- gettimeofday:  %3d.%03d sec\n", (int)tv_diff.tv_sec, (int)(tv_diff.tv_usec/1000)    );
        #ifdef  QNX_SLEEPTEST
        printf("\nResults (by ClockCycles function):\n");
        printf("- CLOCK_GETRES:  %7d nsec\n",(int)tp_RES.tv_nsec );
        printf("- CLOCK_GETTIME: %3d.%03d sec\n", (int)tp_DIFF.tv_sec, (int)(tp_DIFF.tv_nsec/1000000) );
        printf("- GETTIMEOFDAY:  %3d.%03d sec\n", (int)tv_DIFF.tv_sec, (int)(tv_DIFF.tv_usec/1000)    );
        #endif
    }
}
