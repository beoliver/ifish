#include "repl_parsing.h"
#include <sys/stat.h>

static char* search_user_path(char*);


/* free the token (remember that we have an array of pointers) */

void parsing_free(struct tokenized* t) {
  if (t == NULL) {
    return;  
  }
  for (int i = 0; i < 21; i++) {
    free(t->params[i]);
    free(t);
    return;
  }
}



/* we DO NOT allow history calls in the background ie "h &" */

struct tokenized* parsing_tokenize_line(char* line, int line_length) {
  
  if ((line == NULL) || (line_length == 0) || (line[0] == '\n')) {
    return NULL;
  }

  struct tokenized* tokens = malloc (sizeof(struct tokenized));
  int   token_length       = 0;
  int   arg_index          = 0;
  char* token_start        = line;
  char* p                  = line;

  for (int i = 0; i < line_length+1; i++) {
    if ((isspace(*p) || (*p == 0)) && (token_length > 0)) {     
      tokens->params[arg_index] = malloc(sizeof(char) * (token_length + 1));
      stpncpy(tokens->params[arg_index], token_start, token_length);
      token_length = 0;
      arg_index++;
    }
    p++;                /* we increment pointer every time */
    if (isspace(*(p-1))) {
      token_start = p;  /* move position pointer 'p' forwards,set 'token_start' */
    } else {
      token_length++;   /* still matching against a token */
    }
  }
 
  if (arg_index == 0) {
    parsing_free(tokens);
    return NULL;
  }

  if ((strcmp(tokens->params[0],"exit")==0) ||
      (strcmp(tokens->params[0],"quit")==0)) {    
    tokens->flag = USER_EXIT;
    return tokens;
  }

  /* need find out where to 'end' the array of pointers       */
  /* need find out if we should run in background             */

  char* last_ampas = rindex(tokens->params[arg_index-1], '&');
    
  if (last_ampas == NULL) {
    tokens->params[arg_index] = 0;
    tokens->flag = NOFLAG;
  }
  /* from now on we know that there was a bakground flag '&' */
  else if  (strlen(tokens->params[arg_index-1]) == 1) {
    tokens->params[arg_index-1] = 0;
    tokens->flag = RUN_IN_BACKGROUND;
  } else {
    *last_ampas = 0;
    tokens->params[arg_index] = 0;
    tokens->flag = RUN_IN_BACKGROUND;
  }

  /* we now want to check for a call to history.  */

  
  if (strcmp(tokens->params[0],"h")==0) {
    
    if (arg_index < 2) {
      parsing_free(tokens);
      return NULL;
    }
    
    else if (arg_index == 2) {
      if (isnumber(tokens->params[1][0]) && (tokens->params[2] == 0)) {
	if (tokens->flag == RUN_IN_BACKGROUND) {
	  parsing_free(tokens);
	  return NULL;
	}
	tokens->flag = BUILTIN_EXECUTE_HISTORY;
	return tokens;
      } else {
	parsing_free(tokens);
	return NULL;
      }
    }
    
    else if (arg_index == 3) {
      if ((strcmp(tokens->params[1],"-d")==0) &&
	  isnumber(tokens->params[2][0]) &&
	  (tokens->params[3] == 0)) {
	if (tokens->flag == RUN_IN_BACKGROUND) {
	  parsing_free(tokens);
	  return NULL;
	}
	tokens->flag = BUILTIN_DELETE_HISTORY;
	return tokens;
      } else  {
	parsing_free(tokens);
	return NULL;
      }
    }
    
    else {
      parsing_free(tokens);
      return NULL;
    }
  }

  /* end of testing for history */    
  /* builtin calls have been returned... we can now parse the first arg */
  /* as this will be a forked call */

#ifdef DEBUG_INFO
  printf("testing first token for path\n");
#endif

  if (tokens == NULL) {
#ifdef DEBUG_INFO
  printf("TOKENS == NULL\n");
#endif
  return NULL;
  }
  
  char* path = search_user_path(tokens->params[0]);

#ifdef DEBUG_INFO
    printf("PATH == : %s\n", path);
#endif

  if (path == NULL) {
    tokens->flag = UNKNOWN_COMMAND;
    return tokens;
  }

  if (path == tokens->params[0]) {
#ifdef DEBUG_INFO
    printf("PATH == PARAMS[0] : %s\n", path);
#endif
    return tokens;
  }

  /* finally, we switch the old token with the full path */
  
  free(tokens->params[0]);  
  tokens->params[0] = path;
  return tokens;  
}






static char* search_user_path(char* param) {

  /* do we need to expose this? we could use it statically and call it from */
  /*      tokenize line.  */
  
  char* paths         = getenv("PATH");
  char* start_path_pt = paths;
  char* end_path_pt   = paths;
  int   path_length   = 0;

  /* If there are '/' file separators in the line, we treat */
  /* the line as a direct call. We return a copy of it      */

  if (index(param, '/') != NULL) {
    return param;
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
#ifdef DEBUG_INFO
    printf("(stat(path, &fileStat) >= 0) : %s\n", path);
#endif
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
