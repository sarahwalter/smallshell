#ifndef SMSH_PARSE_H
#define SMSH_PARSE_H

#include "smsh_types.h"

/**
 * Parses the provided string into a command structure.
 */
struct smsh_command smsh_parse(char* input);

#endif
