#ifndef mocha_log_h
#define mocha_log_h

#include <stdio.h>

#define MOCHA_LOG(...) { printf(__VA_ARGS__); printf("\n"); fflush(stdout); }

#endif
