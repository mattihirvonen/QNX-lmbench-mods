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

do_build() {
    cd   src
    echo Compile  $1
    $CC  -DHAVE_socklen_t  $OPT  -o $OUT/$1.bin  $1.c  getopt.c  PIPE.c  TIMES.c  lib_timing.c  lib_sched.c  lib_mem.c  lib_stats.c \
    lib_tcp.c  lib_udp.c  lib_unix.c  lib_debug.c  -lm
#   echo .
    cd   ..
}


do_compile() {
    cd   src
    echo Compile  $1
    $CC  -DHAVE_socklen_t  $OPT  -o $OUT/$1.bin  $1.c  getopt.c  PIPE.c  TIMES.c  lib_timing.c  lib_sched.c  lib_mem.c  lib_stats.c  -lm
#   echo .
    cd   ..
}


# Use minimal "library" set
do_minimal() {
    cd   src
    $CC  -DHAVE_socklen_t  $OPT  -o $OUT/$1.bin  $1.c  getopt.c  lib_timing.c  lib_sched.c  PIPE.c  TIMES.c
    cd   ..
}


do_msgring() {
    cd   src
    $CC  -DHAVE_socklen_t -DQNXMSG  $OPT  -o  $OUT/$1.bin  $1.c  lib_timing.c  PIPE.c  TIMES.c
    cd   ..
}


do_build_linux() {
    do_msgring  sleeptest
    do_msgring  msgring
#   do_compile  hello

    do_compile  bw_mem
    do_compile  lat_ops
    do_compile  lat_syscall

#   do_compile  bw_file_read
#   do_compile  bw_mmem_rd
    do_compile  bw_pipe
#   do_compile  bw_tcp
#   do_compile  bw_unix

    do_compile  cache
    do_compile  disk
    do_compile  enough
#   do_compile  flushdisk

    do_compile  lat_cmd
#   do_compile  lat_connect
    do_compile  lat_ctx
    do_compile  lat_ctx_qnx
    do_compile  lat_dram_page
    do_compile  lat_fcntl
    do_compile  lat_fifo
#   do_compile  lat_fs         ## warning(s)
#   do_compile  lat_http
    do_compile  lat_mem_rd
    do_compile  lat_mmap
    do_compile  lat_pagefault
    do_compile  lat_pipe
    do_compile  lat_pmake
    do_compile  lat_proc
    do_compile  lat_rand
#   do_compile  lat_rpc
#   do_compile  lat_select
#   do_compile  lat_sem
    do_compile  lat_sig
#   do_compile  lat_tcp
#   do_compile  lat_udp
#   do_compile  lat_unix
#   do_compile  lat_unix_connect
    do_compile  lat_usleep

    do_compile  line
#   do_compile  lmbench
#   do_compile  lmdd           ## warning(s)
#   do_compile  lmhttp
    do_compile  memsize
    do_compile  mhz
    do_compile  msleep
    do_compile  stream
    do_compile  tlb

    do_compile  par_mem
    do_compile  par_ops
}


do_build_qnx() {
    #  Commented lines do not build properly QNX binary.
    #  (Required sources not included into compile command?)
    #  Some compiles produce warnings !!!

    do_msgring  sleeptest
    do_msgring  msgring
#   do_single   hello

    do_compile  bw_mem
    do_compile  lat_ops
    do_compile  lat_syscall

#   do_compile  bw_file_read
#   do_compile  bw_mmem_rd
    do_compile  bw_pipe
#   do_compile  bw_tcp
#   do_compile  bw_unix

    do_compile  cache
    do_compile  disk
    do_compile  enough
#   do_compile  flushdisk

    do_compile  lat_cmd
#   do_compile  lat_connect
    do_compile  lat_ctx
    do_compile  lat_ctx_qnx
    do_compile  lat_dram_page
    do_compile  lat_fcntl
    do_compile  lat_fifo
#   do_compile  lat_fs         ## warning(s)
#   do_compile  lat_http
    do_compile  lat_mem_rd
    do_compile  lat_mmap
    do_compile  lat_pagefault
    do_compile  lat_pipe
    do_compile  lat_pmake
    do_compile  lat_proc
    do_compile  lat_rand
#   do_compile  lat_rpc
#   do_compile  lat_select
#   do_compile  lat_sem
    do_compile  lat_sig
#   do_build    lat_tcp
#   do_build    lat_udp
#   do_build    lat_unix
#   do_build    lat_unix_connect
    do_compile  lat_usleep

    do_compile  line
#   do_compile  lmbench
#   do_compile  lmdd           ## warning(s)
#   do_compile  lmhttp
    do_compile  memsize
    do_compile  mhz
    do_compile  msleep
    do_compile  stream
    do_compile  tlb

    do_compile  par_mem
    do_compile  par_ops
}


# QNX cross compiling environment under windows/cygwin defines:  OSTYPE=msys
if [ $OSTYPE == msys ]
then
    do_build_qnx
else
    do_build_linux
fi

