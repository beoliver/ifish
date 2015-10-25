
#ifndef __REPL_PARSING__
#define __REPL_PARSING__

#include "repl.h"

#define BUILTIN_DELETE_HISTORY  1
#define BUILTIN_EXECUTE_HISTORY 2
#define RUN_IN_BACKGROUND       3

struct tokenized {
  char*  params[21];
  char   special_call;
} ;

char*             search_user_path(char*);
struct tokenized* parsing_tokenize_line(char*, int);

  
#endif
