#!/bin/bash

QCC='qcc -Vgcc_ntoarmv7le'
GCC='gcc'

QOPT=        # -O2
GOPT=        # -O2

if [ $OSTYPE == msys ]
then
    CC=$QCC
    OPT=$QOPT
    OUT=../bin/qnx
else
    CC=$GCC
    OPT=$GOPT
    OUT=../bin/linux
fi


#  getopt.c  lib_sched.c  lib_stats.c   lib_debug.c  -lm

do_compile() {

    cd   src
    $CC  -DHAVE_socklen_t  $OPT  -o $OUT/$1.bin  $1.c  getopt.c  PIPE.c  lib_timing.c  lib_sched.c  lib_mem.c  lib_stats.c  -lm
#   $CC  -DHAVE_socklen_t  $OPT  -o $OUT/$1.bin  $1.c  getopt.c  PIPE.c  lib_timing.c  lib_sched.c  lib_mem.c
#   $CC  -DHAVE_socklen_t  $OPT  -o $OUT/$1.bin  $1.c  getopt.c
#   mv   $1  ..
    cd   ..
}


# Use minimal "library" set
do_single() {
    cd   src
    $CC  -DHAVE_socklen_t -o $1  $1.c  lib_timing.c  PIPE.c
    mv   $1  ..
    cd   ..
}


do_build_linux() {
    do_single   msgring
#   do_compile  hello

    do_compile  lat_pipe
    do_compile  lat_ctx
    do_compile  lat_ctx_qnx

#   do_compile  clock

    do_compile  cache
    do_compile  bw_mem
    do_compile  bw_file_rd
    do_compile  lat_mem_rd
    do_compile  lat_dram_page

    do_compile  bw_mmap_rd
    do_compile  lat_pagefault

    do_compile  tlb
    do_compile  lat_ops
    do_compile  line
    do_compile  lat_syscall

    do_compile  par_mem       # ei hy�dyllinen
    do_compile  par_ops       # ei hy�dyllinen
}


do_build_qnx() {
    do_single   msgring  # lib_sched.c:(.text+0x40): multiple definition of `handle_scheduler'
#   do_single   hello

    do_compile  bw_mem
    do_compile  lat_ops
    do_compile  lat_syscall

    do_compile  lat_pipe
    do_compile  lat_ctx
    do_compile  lat_ctx_qnx

    do_compile  lat_mem_rd

    do_compile  tlb
    do_compile  lat_dram_page
    do_compile  cache
    do_compile  line
    do_compile  mhz

    do_compile  par_mem
    do_compile  par_ops
}


if [ $OSTYPE == msys ]
then
    do_build_qnx
else
    do_build_linux
fi

