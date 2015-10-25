
#ifndef __REPL_PARSING__
#define __REPL_PARSING__

#include "repl.h"

struct tokenized {
  char*  params[21];
  char   special_call;
} ;

char*             search_user_path(char*);
struct tokenized* parsing_tokenize_line(char*, int);

  
#endif
