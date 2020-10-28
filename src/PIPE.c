

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>      // getpid()
#include <unistd.h>         // getpid()

#ifdef   __QNX__
#include <sys/neutrino.h>   // Msg....()
#endif

#ifndef state_t     // !!!!!!
typedef struct {
    int chid, chid_pid, pid;
} state_t;
#endif // state_t

#ifndef   size_t
//typedef int  size_t;
//typedef int ssize_t;
#define QMSG_BUFFER_SIZE 256
//#define NULL             0
#define EOK              0
#endif // size_t

// This "verbose" is "dupe" because verbose is defined else where.
// GCC/QCC allow duplicate definition and merges variables with same name
// (if user have allowed this feature).
int  verbose_level;
int  debug_level;


int  PIPE(int pipefd[2], void *cookie, char *txt)
{
	// QNX's own benchmark use:
	// - messages receiver task create communication cannel (chid, server)
	// - messages transmitter task connect to channel       (coid, client)

	state_t *state = (state_t *)cookie;

	#ifdef  __QNX__

	// fd(READ)  connect ID
	state->chid_pid  = getpid();
	state->chid      = ChannelCreate(0);
	if (verbose_level) {
            printf("pipe %13s: getpid()=%d, state->chid=%2d, state->chid_pid=%d\n",
                    txt, getpid(), state->chid, state->chid_pid);
	}
	if (state->chid == -1)   return -1;
	pipefd[0] = state->chid;

    #if 0
	// fd(WRITE)  channel ID
	state->coid = ConnectAttach(0, getppid(), chid, _NTO_SIDE_CHANNEL, 0);
    printf("pipe: getpid()=%d, state->chid_pid=%d %13s: coid=%d\n", getpid(), state->chid_pid, txt, state->coid);
	if (state->coid == -1)   return -1;
	pipefd[1] = state->coid;
	#endif

	return 0;

	#else // __QNX__

	int status =  pipe(pipefd);
	if (verbose_level) {
	    if (cookie)
            printf("pipe %13s: getpid()=%d, fdin=%2d, fdout=%2d, state->chid_pid=%d\n",
	                txt ? txt:"", getpid(), pipefd[0], pipefd[1], state->chid_pid);
	    else
	        printf("pipe %13s: getpid()=%d, fdin=%2d, fdout=%2d\n",
	                txt? txt:"", getpid(), pipefd[0], pipefd[1]);
	}
	return status;

	#endif //  __QNX__
}

//--------------------------------------------------------------------------------------------------------
// QNX replacements for read() and write() PIPE()'s file descriptors

#ifdef  __QNX__

ssize_t READ(int fd, void *buf, size_t count)
{
	// info: NULL, or a pointer to a _msg_info structure where the function
	//       can store additional information about the message.

	struct  _msg_info   info;
	uint8_t msg_receive[QMSG_BUFFER_SIZE];

	int chid  = fd;
	int rcvid = MsgReceive(chid, buf, count, &info);
//	int rcvid = MsgReceive(chid, buf, count,  NULL);
	int err   = MsgReply(rcvid, EOK, NULL, 0);
	return (err == -1) ? err : count;
}


ssize_t WRITE(int fd, const void *buf, size_t count)
{
	char msg_reply[QMSG_BUFFER_SIZE];

	int  chid = fd;
	int  err  = MsgSend(chid, buf, count, msg_reply, sizeof(msg_reply));
	return (err == -1) ? err : count;
}

#endif // __QNX__

