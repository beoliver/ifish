#ifndef __REPL_H__
#define __REPL_H__

#include "safefork.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

/* #define DEBUG */
/* #define DEBUG_STRICT */
/* #define DEBUG_INFO */
/* #define CLEAN_DATABLOCKS */

#define DEBUG_OUT stderr

#define MAX_LINE_LENGTH 120
#define LINE_BUFFER_SIZE 121

#define NOFLAG                  0
#define BUILTIN_DELETE_HISTORY  1
#define BUILTIN_EXECUTE_HISTORY 2
#define RUN_IN_BACKGROUND       3
#define USER_EXIT               4
#define UNKNOWN_COMMAND         5
#endif
