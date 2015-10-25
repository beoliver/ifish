#include "repl_parsing.h"
#include <sys/stat.h>

/* DO NOT allow history calls in the background ie "h &" */


struct tokenized* parsing_tokenize_line(char* line, int line_length) {

  if (line == NULL) {
    return NULL;
  }
    
  if (line_length == 0) {
    return NULL;
  }

  if (line[0] == '\n') {
    return NULL;
  }

  struct tokenized* tokens = malloc (sizeof(struct tokenized));
  tokens->special_call = 0;
  
  int    tok_length = 0;
  int    index      = 0;
  char*  tok_start  = line;
  char*  tok_curr   = line;
  int    char_pos;  
  
  for (char_pos = 0; char_pos < line_length+1; char_pos++) {
    
    if ((*tok_curr == ' ') || (*tok_curr == '\n') || (*tok_curr == 0)) {

      if (tok_length != 0) {
	
	int malloc_size = tok_length + 1;
	tokens->params[index] = malloc(sizeof(char)*malloc_size);
	stpncpy(tokens->params[index], tok_start, tok_length);
	/* set last byte to 0 */
	tokens->params[index][tok_length] = '\0';
	
#ifdef DEBUG_INFO
	fprintf(stderr, "token: %s\n", tokens->params[index]);
#endif
	
	tok_length = 0;
	index++;
      }
      /* increment pointer and set new start position  */
      /* this is also done if we encounter multiple while spaces */
      tok_curr++;
      tok_start = tok_curr;
    } else {
      /* *tok_curr is not whitespace */
      tok_curr++;
      tok_length++;
    }
  }
  
  tokens->params[index] = 0;

  if (index == 0) {
    /* most likely there was more than one space at beginning of line */
    /* but NO arguments */
    /* trying to catch a segfault */
    return NULL;
  }

  if (strcmp(tokens->params[0],"h")==0) {
    if (index < 2) {
      printf("no ARGUMENT provided to history\n");
      return NULL;
    }
    if (index == 2) {
      if (isnumber(tokens->params[1][0]) && (tokens->params[2] == 0)) {
	tokens->special_call = BUILTIN_EXECUTE_HISTORY;
	return tokens;
      } else {return NULL;}
    }
    if (index == 3) {
      if ((strcmp(tokens->params[1],"-d")==0) &&
	  isnumber(tokens->params[2][0]) &&
	  (tokens->params[3] == 0)) {
	tokens->special_call = BUILTIN_DELETE_HISTORY;
	return tokens;
      } else  {return NULL;}
    }
    return NULL;
  }
  if (strcmp(tokens->params[index-1],"&")==0) {
    tokens->special_call = RUN_IN_BACKGROUND;
  }
  return tokens;
}






char* search_user_path(char* param) {

  /* do we need to expose this? we could use it statically and call it from */
  /*      tokenize line.  */
  
  char* paths         = getenv("PATH");
  char* start_path_pt = paths;
  char* end_path_pt   = paths;
  int   path_length   = 0;

  /* If there are '/' file separators in the line, we treat */
  /* the line as a direct call. We return a copy of it      */

  if (index(param, '/') != NULL) {
    return strdup(param);
  }
  
  while (1) {
        
    if ((*end_path_pt == ':') || (*end_path_pt == '\0')) {
      char* path = malloc(sizeof(char) * (path_length + 2 + strlen(param)));

      if (path == NULL) {
	perror("char* search_user_path(char* param) :");
	return NULL;
      }
      
      stpncpy(path, start_path_pt, path_length);
      strcat(path, "/");
      strcat(path, param);

      /* test if executable exists */
      struct stat fileStat;
      if (stat(path, &fileStat) >= 0) {
	return path;
      } 

      if (*end_path_pt == 0) {
	/* end of paths, no match was found */
	return NULL;
      } else {
	/* keep searching... */
	/* move pointer to reference beginning of next path */
	end_path_pt++;
	start_path_pt = end_path_pt;
	path_length = 0;
      }
    } else {
      end_path_pt++;
      path_length++;
    }
  }
}
