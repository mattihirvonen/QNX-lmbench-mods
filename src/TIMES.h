
void diff_tv( struct timeval  *diff, const struct timeval  *tv_start, const struct timeval  *tv_end );

void diff_tp( struct timespec *diff, const struct timespec *tp_start, const struct timespec *tp_end );

int64_t diff_tp_ns( const struct timespec *tp_start, const struct timespec *tp_end );
int64_t diff_tp_us( const struct timespec *tp_start, const struct timespec *tp_end );
int64_t diff_tp_ms( const struct timespec *tp_start, const struct timespec *tp_end );
