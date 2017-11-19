#include <stdio.h>

#include "smsh_print.h"

/**
 *  smallsh wrapper for printf, also flushes stdout.
 */
void smsh_printf(char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
}

