#ifndef _DEBUG_H
#define _DEBUG_H

#include "stdint.h"

// e -> what we want to  evaluate
#define ASSERT(e) do { \
    if (!(e)) \
        error_check(__FILE__, __LINE__); \
} while(0)

void error_check(char *file, uint64_t line);

#endif