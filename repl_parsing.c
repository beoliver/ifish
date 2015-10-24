#include "repl_parsing.h"
#include <sys/stat.h>


char* search_user_path(char* param) {
  
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
