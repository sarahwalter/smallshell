#ifndef SMSH_PRINT_H
#define SMSH_PRINT_H

#include <stdarg.h>
#include "smsh_types.h"

/**
 * printf wrapper that also calls fflush.
 */
void smsh_printf(char* fmt, ...);

#endif
