#!/bin/bash

QCC='qcc -Vgcc_ntoarmv7le'
GCC='gcc'

if [ $OSTYPE == msys ]
then
    CC=$QCC
else
    CC=$GCC
fi


#  getopt.c  lib_sched.c  lib_stats.c   lib_debug.c  -lm

do_compile() {

    cd   src
    $CC  -DHAVE_socklen_t  -o $1  $1.c  getopt.c  PIPE.C  lib_timing.c  lib_sched.c  lib_mem.c
#   $CC  -DHAVE_socklen_t  -o $1  $1.c  getopt.c
    mv   $1  ..
    cd   ..
}

do_single() {
    cd   src
    $CC  -DHAVE_socklen_t -o $1  $1.c
    mv   $1  ..
    cd   ..
}

 do_single   msgring
#do_compile  hello

 do_compile  lat_pipe
 do_compile  lat_ctx
 do_compile  lat_ctx_qnx

 do_compile  cache
 do_compile  bw_mem
 do_compile  bw_file_rd
 do_compile  lat_mem_rd
 do_compile  lat_dram_page

 do_compile  bw_mmap_rd
 do_compile  lat_pagefault
