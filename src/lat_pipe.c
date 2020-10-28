/*
 * lat_pipe.c - pipe transaction test
 *
 * usage: lat_pipe [-P <parallelism>] [-W <warmup>] [-N <repetitions>]
 *
 * Copyright (c) 1994 Larry McVoy.  Distributed under the FSF GPL with
 * additional restriction that results may published only if
 * (1) the benchmark is unmodified, and
 * (2) the version in the sccsid below is included in the report.
 * Support for this development by Sun Microsystems is gratefully acknowledged.
 */
char	*id = "$Id$\n";


#ifdef   __QNX__
#include <sys/neutrino.h>   // Msg....()

#define  QMSG_BUFFER_SIZE  256

ssize_t READ(int fd, void *buf, size_t count);
ssize_t WRITE(int fd, const void *buf, size_t count);

#define  read  READ
#define  write WRITE
#endif   //  __QNX__


#include "bench.h"

void initialize(iter_t iterations, void *cookie);
void cleanup(iter_t iterations, void *cookie);
void doit(iter_t iterations, void *cookie);
void writer(int w, int r);

typedef struct _state {
	int	pid;
	int	p1[2];
	int	p2[2];
	//
	int chid_pid;
	int chid;
	int coid;
} state_t;


int main(int ac, char **av)
{
	state_t state;
	int parallel    = 1;
	int warmup      = 1;
	int repetitions = 11;
	int c;
	char* usage = "[-P <parallelism>] [-W <warmup>] [-N <repetitions>]\n";

	while (( c = getopt(ac, av, "P:W:N:")) != EOF) {
		switch(c) {
		case 'P':
			parallel = atoi(optarg);
			if (parallel <= 0) lmbench_usage(ac, av, usage);
			break;
		case 'W':
			warmup = atoi(optarg);
			break;
		case 'N':
			repetitions = atoi(optarg);
			break;
		default:
			lmbench_usage(ac, av, usage);
			break;
		}
	}
	if (optind < ac) {
		lmbench_usage(ac, av, usage);
	}

	state.pid = 0;
    printf("base: getpid()=%d\n", getpid());

	benchmp(initialize, doit, cleanup, SHORT, parallel, warmup, repetitions, &state);
	micro("Pipe latency", get_n());
	return (0);
}


void initialize(iter_t iterations, void* cookie)
{
	char	c;
	state_t * state = (state_t *)cookie;

	if (iterations) return;

	// The process ID of the owner of the QNX message channel.
	// If pid is zero, the calling process is assumeder
	state->chid_pid = getpid();

	if (PIPE(state->p1,cookie,"p1") == -1) {
		perror("pipe p1");
		exit(1);
	}
	if (PIPE(state->p2,cookie,"p2") == -1) {
		perror("pipe p2");
		exit(1);
	}

	handle_scheduler(benchmp_childid(), 0, 1);
	switch (state->pid = fork()) {
	    case 0:
	        printf("initialize(child) : getpid()=%d\n", getpid());
	        #ifdef __QNX__
            int coid1 = ConnectAttach(0, state->p1[0], state->chid, _NTO_SIDE_CHANNEL, 0);
            int coid2 = ConnectAttach(0, state->p2[0], state->chid, _NTO_SIDE_CHANNEL, 0);
            printf("conn: getpid()=%d, state->chid_pid=%d, coid1=%d\n", getpid(), state->chid_pid, coid1);
            printf("conn: getpid()=%d, state->chid_pid=%d, coid2=%d\n", getpid(), state->chid_pid, coid2);
            state->p1[0] = coid1;
            state->p2[1] = coid2;
            #endif // __QNX__
			handle_scheduler(benchmp_childid(), 1, 1);
			signal(SIGTERM, exit);
		//	close(state->p1[1]);
		//	close(state->p2[0]);
			writer(state->p2[1], state->p1[0]);
			return;

	    case -1:
			perror("fork");
			return;

	    default:
		//	close(state->p1[0]);
		//	close(state->p2[1]);
			break;
	}

	/*
	 * One time around to make sure both processes are started.
	 */
    printf("initialize(parent): getpid()=%d\n", getpid());
	if (write(state->p1[1], &c, 1) != 1 || read(state->p2[0], &c, 1) != 1) {
		perror("(i) read/write on pipe");
		exit(1);
	}
}


void cleanup(iter_t iterations, void* cookie)
{
	state_t * state = (state_t *)cookie;

	if (iterations) return;

	if (state->pid) {
		kill(state->pid, SIGKILL);
		waitpid(state->pid, NULL, 0);
		state->pid = 0;
	}
}


void doit(register iter_t iterations, void *cookie)
{
	state_t *state = (state_t *) cookie;
	char		c;
	register int	w = state->p1[1];
	register int	r = state->p2[0];
	register char	*cptr = &c;

	while (iterations-- > 0) {
		if (write(w, cptr, 1) != 1 ||
		    read(r, cptr, 1) != 1) {
			perror("(r) read/write on pipe");
			exit(1);
		}
	}
}


// Child process
void writer(register int w, register int r)
{
	char		c;
	register char	*cptr = &c;

	for ( ;; ) {
		if (read(r, cptr, 1) != 1 ||
			write(w, cptr, 1) != 1) {
			    perror("(w) read/write on pipe");
		}
	}
}

