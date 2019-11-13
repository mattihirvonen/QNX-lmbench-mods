
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef  __linux
#include <sys/neutrino.h>   // Msg....()
#endif
#include <sched.h>          // getprio(), setprio()
#include <sys/resource.h>   // getprio(), setprio()
#include "msgring.h"

#define  QMSG_BUFFER_SIZE  256
#define  MAXPROCS          1000
#define FILENAME           "/tmp/msgring.$"

#ifndef  DEBUG
#define  DEBUG 1
#endif //DEBUG


#if defined(__MINGW32__)
typedef struct timespec {
        time_t   tv_sec;        /* seconds */
        long     tv_nsec;       /* nanoseconds */
} timespec_t;
#else
typedef struct timespec timespec_t;
#endif // __MINGW32__


typedef struct {
    int run;
    int pid;
    int ppid;
    int chid;
    int coid;
} state_t;


state_t  state;
int      fork_pids[MAXPROCS];
int      procs  = 4;
int      rounds = 5;
int      Nsend  = 1;
int      warmup = 1;
int      scheduler;
int      verbose;

//-----------------------------------------------------------------------------------------------

void diff_tp( struct timespec *diff, const struct timespec *tp_start, const struct timespec *tp_end )
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

int64_t diff_tp_us( const struct timespec *tp_start, const struct timespec *tp_end )
{
    struct timespec  diff;
    int64_t          us;

    diff_tp( &diff, tp_start, tp_end );

    us  = diff.tv_nsec / 1000;
    us += diff.tv_sec  * 1000000;

    return us;
}

int64_t diff_tp_ms( const struct timespec *tp_start, const struct timespec *tp_end )
{
    struct timespec  diff;
    int64_t          ms;

    diff_tp( &diff, tp_start, tp_end );

    ms  = diff.tv_nsec / 1000000;
    ms += diff.tv_sec  * 1000;

    return ms;
}

//-----------------------------------------------------------------------------------------------
// Old code from pipe(s) method using read() and write()

#ifndef __linux
ssize_t READ(int fd, void *buf, size_t count)
{
	// info: NULL, or a pointer to a _msg_info structure where the function
	//       can store additional information about the message.

	struct  _msg_info   info;
	uint8_t msg_receive[QMSG_BUFFER_SIZE];

	int chid  = fd;
//	int rcvid = MsgReceive(chid, msg_receive, sizeof(msg_receive), &info);
	int rcvid = MsgReceive(chid, msg_receive, sizeof(msg_receive),  NULL);
	int err   = MsgReply(rcvid, EOK, NULL, 0);
        if (err == -1) {
            return err;
        }
	memcpy(buf, msg_receive, count);  /// TODO: READ goes too long!
	return count;
}


ssize_t WRITE(int fd, const void *buf, size_t count)
{
	char msg_reply[QMSG_BUFFER_SIZE];
	int  coid = fd;
	int  err  = MsgSend(coid, buf, count, msg_reply, sizeof(msg_reply));
        if (err == -1) {
            return err;
        }
//	memcpy(buf, msg_reply, QMSG_BUFFER_SIZE);  /// TODO: Reply READ goes too long!
	return count;
}
#endif // __linux

//-------------------------------------------------------------------------------------
// Old code from pipe(s) method using read() and write()

void jumper1(int Nsend)
{
    #if DEBUG
    printf("jumper:      pid=%d, Nsend=%d, state.coid=%X, state.chid=%X\n",
            state.pid, Nsend, state.coid, state.chid);
    #endif

    char buffer[QMSG_BUFFER_SIZE];

    while (state.run)
    {
        #ifndef __linux
        if (READ( state.chid, buffer, sizeof(buffer)) == -1) {
            sprintf(buffer, "jumper(pid=%d) READ state.chid=%X", getpid(), state.chid);
            perror(buffer);
            exit(1);
        }
        #if DEBUG
        printf("jumper(%d) READ ok\n",getpid());
        sleep(1);
        #endif // DEBUG
        if (WRITE(state.coid, buffer, Nsend) == -1) {
            sprintf(buffer, "jumper(pid=%d) WRITE state.coid=%X", getpid(), state.coid);
            perror(buffer);
            exit(1);
        }
        #if DEBUG
        printf("jumper(%d) WRITE ok\n",getpid());
        #endif // DEBUG
        #endif // __linux
    }
}


void iterator1(int rounds, int Nsend)
{
    #if DEBUG
    printf("iterator(%d): pid=%d, ppid=%d, Nsend=%d, state.chid=%X, state.coid=%X\n",
            rounds, state.pid, state.ppid, Nsend, state.chid, state.coid);
    #endif

    while (rounds-- > 0)
    {
        #ifndef __linux
        char buffer[QMSG_BUFFER_SIZE];

        if (WRITE(state.coid, buffer, Nsend) == -1) {
            perror("iterator WRITE");
            exit(1);
        }
        if (READ( state.chid, buffer, sizeof(buffer)) == -1) {
            perror("iterator READ");
            exit(1);
        }
        #if DEBUG
        printf("iterator rounds=%d\n", rounds);
        #endif // DEBUG
        #endif // __linux
    }
}

//------------------------------------------------------------------------------------------
// Tiny QNX message functions simulation
// Homma on pahasti kesken !!!!!!!!
// Pitää ratkaista MsgReply() logiikka
// - tarvitaan piippupari / message putki

#ifdef __linux

#define  _NTO_SIDE_CHANNEL  1
#define  EOK                0

#define  MAXPIPES 100
typedef  struct
{
    int   pid;
    int   fd[2];
    int   chid;
} msgpipe_t;

msgpipe_t pipetable[MAXPIPES];
int       pipecount;


int ChannelCreate(int arg);
int ConnectAttach(int arg1, int pid, int chid, int arg2, int arg3);
int MsgSend(int coid, void *txbuf, int Nsend, void *rxbuf, int rxsize);
int MsgReceive(int chid, void *rxbuf, int rxsize, int arg);
int MsgReply(int rcvid, int EOK, int arg1, int arg2);


int ChannelCreate(int arg)
{
    if (pipecount >= MAXPIPES) return -1;

    int fd[2];
    int err = pipe( fd );
    int chid = random() & 0x7fffffff;

    if (err)  return -1;

    pipetable[pipecount].pid   = getpid();
    pipetable[pipecount].fd[0] = fd[0];
    pipetable[pipecount].fd[1] = fd[1];
    pipetable[pipecount].chid  = chid;
    pipecount++;
    return chid;
}


int ConnectAttach(int arg1, int pid, int chid, int arg2, int arg3)
{
    for (int i = 0; i < pipecount; i++)
    {
        if ((pid == pipetable[i].pid) && (chid == pipetable[i].chid))
        {
            return pipetable[i].chid;
        }
    }
    return -1;
}


int find_pipetable_ix(int chid)
{
    for (int ix = 0; ix < pipecount; ix++)
    {
        if (pipetable[ix].chid == chid)
            return ix;
    }
    return -1;
}


int MsgSend(int coid, void *txbuf, int Nsend, void *rxbuf, int rxsize)
{
    int ix = find_pipetable_ix(coid);

    if (ix  < 0)  return -1;
    int err = write( pipetable[ix].fd[0], txbuf, Nsend );
    if (err < 0)  return -1;
    err = read( pipetable[ix].fd[0],rxbuf, rxsize );
    return err < 0 ? -1 : 0;
}


int MsgReceive(int chid, void *rxbuf, int rxsize, int arg)
{
    int ix = find_pipetable_ix(chid);

    if (ix  < 0)  return -1;
    int err = read( pipetable[ix].fd[1], rxbuf, rxsize );
    return err < 0 ? -1 : 0;
}


int MsgReply(int rcvid, int EOK, int arg1, int arg2)
{
    int ix = find_pipetable_ix(rcvid);
    return 0;
}


/*
int  err  = ChannelCreate(int arg);
int coid  = ConnectAttach(0, state.ppid, state.chid, _NTO_SIDE_CHANNEL, 0);
int  err  = MsgSend(coid, msg_receive, Nsend, msg_reply, sizeof(msg_reply));
int rcvid = MsgReceive(chid, msg_receive, sizeof(msg_receive),  NULL);
int  err  = MsgReply(rcvid, EOK, NULL, 0);
*/

#endif

//-----------------------------------------------------------------------------------------------

int writefile(char *filename, int pid, int chid)
{
        char txt[64];
        int  nwite, fd;

        sprintf(txt,"%d %d", pid, chid);
        fd = open(filename, O_CREAT | O_WRONLY);
        if (fd < 0)
            return -1;
        nwite = write(fd, txt, strlen(txt));
        if (nwite < 0)
            return -1;
        if (close(fd) < 0)
            return -1;
        if (verbose) {
            printf("file write  <%s>\n", txt);
        }
        return nwite;
}


int readfile(char *filename, int *ppid, int *chid)
{
        #define MAXRETRY  30

        char txt[64], *pch;
        int  nread, fd;
        int  retry_count;

        // Wait all sub process to wake up
        for(retry_count = 0; retry_count < MAXRETRY; retry_count++)
        {
            sleep(1);
            fd = open(filename, O_RDONLY);
            if (fd > 0)
                break;
            printf("readfile (re)try %d/%d\n", retry_count+1, MAXRETRY);
        }
        if (retry_count >= MAXRETRY)
            return -1;
        nread = read(fd, txt, sizeof(txt));
        if (nread < 0)
            return -1;
        if (close(fd) < 0)
            return -1;

        pch  = strtok(txt,"\t ,");
       *ppid = atoi(pch);
        pch  = strtok (NULL, "\t ,");
       *chid = atoi(pch);
        if (verbose) {
            printf("file read   (ppid=%d chid=%d)\n", *ppid, *chid);
        }
        return *ppid;
}


void jumper(int Nsend)
{
    if (verbose) {
            printf("jumper:      pid=%d, Nsend=%d, state.coid=%X, state.chid=%X\n",
                    state.pid, Nsend, state.coid, state.chid);
    }

    register int chid = state.chid;
    register int coid = state.coid;

    while (state.run)
    {
        #ifndef __linux
	// info: NULL, or a pointer to a _msg_info structure where the function
	//       can store additional information about the message.

	struct   _msg_info   info;
	uint8_t  msg_receive[QMSG_BUFFER_SIZE];
	char     msg_reply[QMSG_BUFFER_SIZE];

        // Receive message from previous process
	int rcvid = MsgReceive(chid, msg_receive, sizeof(msg_receive),  NULL);
//	int rcvid = MsgReceive(chid, msg_receive, sizeof(msg_receive), &info);

	// Release previous process from dead lock
        int   err = MsgReply(rcvid, EOK, NULL, 0);

        // Forfard message to next process
        err |= MsgSend(coid, msg_receive, Nsend, msg_reply, sizeof(msg_reply));

        // Error should not newer happen!
        if  (err == -1) {
            return exit(1);
        }
        #endif // __linux
    }
}


void iterator(int rounds, int Nsend)
{
    if (verbose) {
        printf("iterator(%d): pid=%d, ppid=%d, Nsend=%d, state.chid=%X, state.coid=%X\n",
                rounds, state.pid, state.ppid, Nsend, state.chid, state.coid);
    }

    register int chid = state.chid;
    register int coid = state.coid;

    while (rounds-- > 0)
    {
        #ifndef __linux
	struct   _msg_info   info;
	uint8_t  msg_transmit[QMSG_BUFFER_SIZE];
	uint8_t  msg_receive[QMSG_BUFFER_SIZE];
	char     msg_reply[QMSG_BUFFER_SIZE];

	// Send message to process ring
	int  err  = MsgSend(coid, msg_transmit, Nsend, msg_reply, sizeof(msg_reply));

//	sched_yieald();

        // Wait message return from process ring
	int rcvid = MsgReceive(chid, msg_receive, sizeof(msg_receive),  NULL);
//	int rcvid = MsgReceive(chid, msg_receive, sizeof(msg_receive), &info);
             err |= MsgReply(rcvid, EOK, NULL, 0);

        // Error should not newer happen!
        if  (err == -1) {
            kill(0, SIGKILL);
            return exit(1);
        }
        #if 0  // DEBUG
        printf("iterator rounds=%d\n", rounds);
        #endif // DEBUG
        #endif // __linux
    }
}


int  run_iterator(int Nsend )
{
        timespec_t  tp_start, tp_end;

        int  ppid, chid, fd;

        // Get last process's PID and channel ID for connection
        if (readfile(FILENAME, &ppid, &chid) < 0) {
            printf("iterator:    readfile ERROR\n");
            kill(0, SIGKILL);
            exit(1);
        }

        #ifndef __linux
        int coid = ConnectAttach(0, ppid, chid, _NTO_SIDE_CHANNEL, 0);
        if (coid == -1) {
            printf("iterator:    coid ERROR\n");
            kill(0, SIGKILL);
            exit(1);
        }
        state.coid = coid;  // "fd(WRITE)"
        #endif // __linux

        printf("\n");
        printf("Run %d rounds with %d processes\n\n", rounds, procs);
        printf("Start test...\n");

        clock_gettime(CLOCK_MONOTONIC, &tp_start);
        iterator(rounds, Nsend);
        clock_gettime(CLOCK_MONOTONIC, &tp_end);
        int us = diff_tp_us(&tp_start, &tp_end);
        return us;
}


void print_usage(int argc, char *argv[], char *usage)
{
        printf("Usage:  %s %s\n", argv[0], usage);
        exit(0);
}


void get_opts(int argc, char *argv[])
{
        int   c;
	char* usage = "[-W <warmup>] [-N <repetitions>] [P <processes>] [-R] [-F]\n";

	while (( c = getopt(argc, argv, "W:N:P:FRv")) != EOF) {
            switch(c) {
		case 'W':
			warmup = atoi(optarg);
			if (warmup < 0) print_usage(argc, argv, usage);
			break;
		case 'N':
			rounds = atoi(optarg);
			if (rounds < 1) print_usage(argc, argv, usage);
			break;
		case 'P':
			procs = atoi(optarg);
			if (procs < 2) print_usage(argc, argv, usage);
			break;
		case 'R':
			scheduler = SCHED_RR;
			break;
		case 'F':
			scheduler = SCHED_FIFO;
			break;
		case 'v':
			verbose  += 1;
			break;
		default:
			print_usage(argc, argv, usage);
			break;
            }
	}
}


void init_state(void)
{
    state.run        = 1;
    state.ppid       = 0;
    state.pid        = getpid();
    state.chid       = ChannelCreate(0);  // fd(READ)
    if (state.chid  == -1)
        exit(1);

    remove(FILENAME);  // Remove old fork chain's "PID chid" file
}


// https://pubs.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_08.html#tag_02_08_04_01
int set_scheduling(int policy, int priority)
{
    struct sched_param  param;

    printf("Use %s task switching policy\n", policy == SCHED_FIFO ? "SCHED_FIFO" : "SCHED_RR");

    // Get current (dafault parameters)
    int ret = sched_getparam( getpid(), &param);

    param.sched_priority = priority;                       // priority:  0 ... 99 (Posix max 31)
    ret = sched_setscheduler(getpid(), policy, &param);    // policy:    SCHED_FIFO or SCHED_RR

    // Sample command to read current scheduling policy
    policy = sched_getscheduler(getpid());

    return ret;  // Dummy, avoid warning of variable set but not used
}


void fork_msgring(int procs)
{
    int fork_count = procs - 1;

    fork_pids[0] = getpid();
    for (int ix  = 1; fork_count-- > 0; ix++)
    {
        if (getpid() == fork_pids[ix-1])
        {
            int new_pid = fork();
            switch (new_pid)
            {
                case -1:
                    exit(1);
                    break;

                case 0:
                    // Child process
                    fork_pids[ix] = getpid();
                    state.pid     = getpid();
                    state.ppid    = getppid();

                    if (verbose) {
                        if (fork_count > 0)
                            printf("child(%d):    pid=%d, ppid=%d, chid=%X\n", ix, state.pid, state.ppid, state.chid);
                        else
                            printf("child(%d):    pid=%d, ppid=%d, chid=%X (last process)\n", ix, state.pid, state.ppid, state.chid);
                    }

                    // fd(READ) channel ID
                    int chid = ChannelCreate(0);
                    if (chid == -1)
                    {
                        printf("child(%d):   chid=ERROR\n", ix);
                        exit(1);
                    }
                    // fd(WRITE) channel ID
                    int coid = ConnectAttach(0, state.ppid, state.chid, _NTO_SIDE_CHANNEL, 0);
                    if (coid == -1)
                    {
                        printf("child(%d):   coid=ERROR state.ppid=%d state.chid=%X\n", ix, state.ppid, state.chid);
                        exit(1);
                    }
                    state.chid = chid;  //  "fd(READ)"
                    state.coid = coid;  //  "fd(WRITE)"

                    if (!fork_count)  { //  Last forked process?
                        writefile(FILENAME, state.pid, state.chid);
                    }

                    if (verbose) {
                        printf("child(%d):   chid=%X, coid=%X\n", ix, state.chid, state.coid);
                    }
                    break;

                default:
                    // Parent process
                    if (verbose) {
                        printf("parent(%d):   pid=%d, new_pid(%d)=%d, chid=%X, coid=%X\n", ix-1, getpid(), ix, new_pid, state.chid, state.coid);
                    }
                    break;
            }
        }
    }
}


int main(int argc, char*argv[])
{
    get_opts(argc, argv);
    init_state();

    fork_msgring(procs);
    if (scheduler)
        set_scheduling(scheduler, 10);  // SCHED_FIFO or SCHED_RR

    if (state.ppid) {
        jumper(Nsend);  // (Sub)Process should not return from jumper()!
    }
    else {
        int us = run_iterator(Nsend);

        printf("\n");
        printf("Test run time: %d us\n", us);
        printf("Msg passes:    %d (%d procs * %d rounds)\n", procs*rounds, procs, rounds);
        printf("Msg pass time: %d us / pass (%d us / %d pass)\n", us/(procs*rounds), us, procs*rounds);

        us = run_iterator(Nsend);

        printf("\n");
        printf("Test run time: %d us\n", us);
        printf("Msg passes:    %d (%d procs * %d rounds)\n", procs*rounds, procs, rounds);
        printf("Msg pass time: %d us / pass (%d us / %d pass)\n", us/(procs*rounds), us, procs*rounds);
    }
    remove(FILENAME);
    kill(0, SIGKILL);   // Kill also all sub process
    return 0;
}
