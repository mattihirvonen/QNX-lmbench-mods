#!/bin/bash

CC=qcc

TGT=-V

#  getopt.c  lib_sched.c  lib_stats.c   lib_debug.c  -lm

do_compile() {

    cd   src
#   $CC  -DHAVE_socklen_t  -o $1  $1.c  getopt.c  lib_timing.c  lib_sched.c
    $CC  -DHAVE_socklen_t  -o $1  $1.c  getopt.c
    mv   $1  ..
    cd   ..
}

build_msgring() {
    cd   src
    $CC  -DHAVE_socklen_t  -o $1  $1.c
    mv   $1  ..
    cd   ..
}

#do_compile  hello
#do_compile  lat_pipe
#do_compile  lat_ctx

 build_msgring  msgring
