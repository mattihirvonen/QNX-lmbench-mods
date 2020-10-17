#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>       // sleep()
#include <time.h>
#include <sys/time.h>
#include "TIMES.h"

//-----------------------------------------------------------------------------------------------
// Dummy satify for  lib_timing.c::handle_scheduler()  request

int handle_scheduler(int x, int y, int z)
{
    return 0;
}

//-----------------------------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
    struct timespec  tp_start, tp_end, tp_diff;
    struct timeval   tv_start, tv_end, tv_diff;

    int sleeptime = 10;
    int command   = atoi( argv[1] );

    if (command)
        sleeptime = command;

    printf("Sleep test %d seconds...\n", sleeptime);

    clock_gettime( CLOCK_MONOTONIC, &tp_start );
    gettimeofday( &tv_start, NULL );

    sleep( sleeptime );

    clock_gettime( CLOCK_MONOTONIC, &tp_end );
    gettimeofday( &tv_end, NULL );

    diff_tp( &tp_diff, &tp_start, &tp_end );
    diff_tv( &tv_diff, &tv_start, &tv_end );

    printf("...done\n");
    printf("\n");
    printf("Results:\n");
    printf("- clock_gettime: %3d.%06d seconds\n", (int)tp_diff.tv_sec, (int)(tp_diff.tv_nsec/1000000) );
    printf("- gettimeofday:  %3d.%06d seconds\n", (int)tv_diff.tv_sec, (int)(tv_diff.tv_usec/1000)    );
}
