
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
    #ifndef __linux
    uint64_t cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;

    printf("QNX ClockCycles(): clock pulses per second = %lld\n\n", cps);
    #endif
}

//-----------------------------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
    struct timespec  tp_start, tp_end, tp_diff;
    struct timeval   tv_start, tv_end, tv_diff;

    clock_gettime( CLOCK_MONOTONIC, &tp_start );
    gettimeofday( &tv_start, NULL );

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

    printf("...done\n\n");

    clock_gettime( CLOCK_MONOTONIC, &tp_end );
    gettimeofday( &tv_end, NULL );
    diff_tp( &tp_diff, &tp_start, &tp_end );
    diff_tv( &tv_diff, &tv_start, &tv_end );

    printf("Results:\n");
    if (decimals) {
        printf("- clock_gettime: %3d.%09d seconds\n",    (int)tp_diff.tv_sec, (int)(tp_diff.tv_nsec) );
        printf("- gettimeofday:  %3d.%06d    seconds\n", (int)tv_diff.tv_sec, (int)(tv_diff.tv_usec) );
    }
    else {
        printf("- clock_gettime: %3d.%03d seconds\n", (int)tp_diff.tv_sec, (int)(tp_diff.tv_nsec/1000000) );
        printf("- gettimeofday:  %3d.%03d seconds\n", (int)tv_diff.tv_sec, (int)(tv_diff.tv_usec/1000)    );
    }
}
