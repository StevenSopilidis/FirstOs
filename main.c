#include "trap.h"
#include "print.h"
#include "debug.h"

// called by assembly code
void KMain(void) {
    init_idt();
    ASSERT(0);
}