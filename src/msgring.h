
#include	<assert.h>
#include        <ctype.h>
#include        <stdio.h>
#include	<math.h>
#ifndef WIN32
#include        <unistd.h>
#endif
#include        <stdlib.h>
#include        <fcntl.h>
#include        <signal.h>
#include        <errno.h>
#ifndef WIN32
#include        <strings.h>
#endif
#include        <sys/types.h>
#ifndef WIN32
#include        <sys/mman.h>
#endif
#include        <sys/stat.h>
#ifndef WIN32
#include        <sys/wait.h>
#include	<time.h>
#include        <sys/time.h>
#include        <sys/socket.h>
#include        <sys/un.h>
#include        <sys/resource.h>
#define PORTMAP
#include	<rpc/rpc.h>
#endif
#ifdef HAVE_pmap_clnt_h
#include	<rpc/pmap_clnt.h>
#endif
#include	<rpc/types.h>
#ifdef HAVE_pmap_clnt_h
#include	<rpc/pmap_clnt.h>
#endif

#include 	<stdarg.h>
#ifndef HAVE_uint
typedef unsigned int uint;
#endif

#ifndef S_IREAD
#define S_IREAD S_IRUSR
#endif
#ifndef S_IWRITE
#define S_IWRITE S_IWUSR
#endif
#ifndef S_IEXEC
#define S_IEXEC S_IXUSR
#endif

#ifndef HAVE_uint64
#ifdef HAVE_uint64_t
typedef uint64_t uint64;
#else /* HAVE_uint64_t */
typedef unsigned long long uint64;
#endif /* HAVE_uint64_t */
#endif /* HAVE_uint64 */

#ifndef HAVE_int64
#ifdef HAVE_int64_t
typedef int64_t int64;
#else /* HAVE_int64_t */
typedef long long int64;
#endif /* HAVE_int64_t */
#endif /* HAVE_int64 */

#ifndef HAVE_socklen_t
typedef int socklen_t;
#endif

#ifndef HAVE_off64_t
typedef int64 off64_t;
#endif

#define NO_PORTMAPPER

