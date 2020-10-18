
#ifdef    QNX_SLEEPTEST
int CLOCK_GETRES(  clockid_t  __clock_id, struct timespec *__tp );
int CLOCK_GETTIME( clockid_t  __clock_id, struct timespec *__tp );
int GETTIMEOFDAY(struct timeval *tv, void *tz);
#endif // SLEEPTEST

void diff_tv( struct timeval  *diff, struct timeval  *tv_start, struct timeval  *tv_end );
void diff_tp( struct timespec *diff, struct timespec *tp_start, struct timespec *tp_end );

int64_t diff_tp_ns( struct timespec *tp_start, struct timespec *tp_end );
int64_t diff_tp_us( struct timespec *tp_start, struct timespec *tp_end );
int64_t diff_tp_ms( struct timespec *tp_start, struct timespec *tp_end );
