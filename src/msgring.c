
#include <stdio.h>
#include <stdlib.h>

#ifndef  __linux
#include <sys/neutrino.h>   // Msg....()
#endif

#include "msgring.h"

#define  QMSG_BUFFER_SIZE  256
#define  MAXPROCS          1000

#ifndef  DEBUG
#define  DEBUG 0
#endif //DEBUG

typedef struct {
    int run;
    int pid;
    int ppid;
    int chid;
    int coid;
    //
    int base_pid;
    int base_chid;
    int fork_count;
} state_t;


state_t  state;
int      fork_pids[MAXPROCS];
int      procs  = 4;
int      rounds = 5;
int      Nsend  = 1;
int      warmup = 1;


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


void jumper(int Nsend)
{
    #if DEBUG
    printf("jumper:      pid=%d, Nsend=%d, state.coid=%X, state.chid=%X\n",
            state.pid, Nsend, state.coid, state.chid);
    #endif

    char buffer[QMSG_BUFFER_SIZE];

    if (!state.fork_count) while(1);  // Loop for ever here

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


void iterator(int rounds, int Nsend)
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


void run_iterator(int rounds, int Nsend, int procs )
{
        char txt[64], *pch;
        int  ppid, chid, fd;

        printf("Run %d rounds with %d processes\n", rounds, procs);

        fd = open("/tmp/msgring.$", O_RDONLY);
        read(fd, txt, sizeof(txt));
        close(fd);

        pch  = strtok(txt,"\t ,");
        ppid = atoi(pch);
        pch  = strtok (NULL, "\t ,");
        chid = atoi(pch);

        #if DEBUG
        printf("file read (%d %d):%s\n", ppid, chid, txt);
        #endif // DEBUG

        int coid = ConnectAttach(0, ppid, chid, _NTO_SIDE_CHANNEL, 0);
        if (coid == -1)
        {
            printf("iterator:   coid=ERROR\n");
            exit(1);
        }
        state.coid = coid;  // "fd(WRITE)"

        iterator(rounds, Nsend);
}


void print_usage(int argc, char *argv[], char *usage)
{
        printf("Usage:  %s %s\n", argv[0], usage);
        exit(0);
}


void get_opts(int argc, char *argv[])
{
        int   c;
 	char* usage = "[-W <warmup>] [-N <repetitions>]\n";

	while (( c = getopt(argc, argv, "W:N:")) != EOF) {
            switch(c) {
		case 'W':
			warmup = atoi(optarg);
			if (warmup < 0) print_usage(argc, argv, usage);
			break;
		case 'N':
			rounds = atoi(optarg);
			if (rounds < 1) print_usage(argc, argv, usage);
			break;
		default:
			print_usage(argc, argv, usage);
			break;
            }
	}
}


int main(int argc, char*argv[])
{
    get_opts(argc, argv);

    state.run        = 1;
    state.fork_count = procs - 1;
    state.base_pid   = getpid();
    #ifdef __linux
    state.chid       = 0;
    state.coid       = 0;
    #else
    state.chid       = ChannelCreate(0);  // fd(READ)
    if (state.chid  == -1)
        exit(1);
    #if 0
    // Connect to WRITE channel later by name
    state.coid       = ChannelCreate(0);  // fd(WRITE)
    if (state.coid  == -1)
        exit(1);
    #endif
    #endif // __linux
    state.base_chid  = state.coid;
    state.ppid       = 0;
    state.pid        = getpid();

    fork_pids[0] = getpid();
    for (int ix  = 1; state.fork_count-- > 0; ix++)
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

                    #if 1 // DEBUG
                    if (state.fork_count > 0)
                        printf("child(%d):    pid=%d, ppid=%d, chid=%X\n", ix, state.pid, state.ppid, state.chid);
                    else
                        printf("child(%d):    pid=%d, ppid=%d, chid=%X (last process)\n", ix, state.pid, state.ppid, state.chid);
                    #endif

                    #ifndef __linux
                    //if (state.fork_count > 0)
                    {
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
                        state.chid = chid;  // "fd(READ)"
                        state.coid = coid;  // "fd(WRITE)"
                    }
                    if (!state.fork_count)  // Last process to fork!
                    {
                        char txt[64];
                        int  fd = open("/tmp/msgring.$", O_CREAT | O_WRONLY);

                        sprintf(txt,"%d %d\n", state.pid, state.chid);
                        write(fd, txt, strlen(txt));
                        close(fd);
                        #if DEBUG
                        printf("file write:%s\n", txt);
                        #endif // DEBUG
                    }
                    #endif // __linux

                    #if DEBUG
                    printf("child(%d):   chid=%X, coid=%X\n", ix, state.chid, state.coid);
                    #endif // DEBUG
                    break;

                default:
                    // Parent process
                    printf("parent(%d):   pid=%d, new_pid(%d)=%d, chid=%X, coid=%X\n", ix-1, getpid(), ix, new_pid, state.chid, state.coid);
                    break;
            }
        }
    }
    if (!state.ppid)
    {
        // Wait for all process to wake up
        sleep(1);
        run_iterator(rounds, Nsend, procs);
    }
    else
    {
        jumper(Nsend);  // (Sub)Process should not return from jumper!
    }
    sleep(1);
    kill(0, SIGKILL);   // Kill also all sub process
    return 0;
}


int  PIPE(int pipefd[2], void *cookie, char *txt)
{
    return 0;
}
