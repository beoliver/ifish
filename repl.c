#include "repl.h"
#include "repl_parsing.h"
#include "repl_history.h"


int main(int argc, char** argv) {

  char* uname  = getenv("USER");
  int   command_count = 0;
  int   line_length;
  char  line_buffer[LINE_BUFFER_SIZE];
  char  history_buffer[LINE_BUFFER_SIZE];
  
  history_init();

 inputloop:while (1) {
   
    fprintf(stdout, "%s@ifish %d > ", uname, command_count);
    fgets(line_buffer, LINE_BUFFER_SIZE, stdin);
    if (feof(stdin)) {fputc('\n', stdout); exit(0);}
    
    command_count++ ;

    /* check that the input line is not TOO long    */
    
  replloop:if ((line_length = strlen(line_buffer)) <= MAX_LINE_LENGTH) {
      struct tokenized* t = parsing_tokenize_line(line_buffer, line_length);
      if (t == NULL) {
	/* printf("invalid command\n"); */
	goto inputloop;
      }

      if (t->special_call == USER_EXIT) {
	exit(0);
      }
            
      if (t->special_call == BUILTIN_DELETE_HISTORY) {

#ifdef DEBUG_INFO	
	printf("CALL TO DELETE HISTORY\n");
#endif
	
	/* to delete from history, we delete items   */
	/* and then add the line to the history      */
	
	int n = atoi(t->params[2]);	
	int hd = history_delete_last_n_items(n);	
	history_insert(line_buffer, line_length);

	if (hd != 0) {
	  printf("YOU DONT HAVE THAT MUCH HISTORY -- what are you trying to hide?\n");
	}
	
	parsing_free(t);
	goto inputloop;
      }

      
      if (t->special_call == BUILTIN_EXECUTE_HISTORY) {

	/* this is where we have to be carefull. It is possible           */
	/* that we enter an infinite loop by making continuous            */
	/* calls to an item in history                                    */

	/* lets say we type 'h 1' when we start the repl. Nothing happens */
	/* then we type 'h 1' again. The system will find 'h 1' as the    */
	/* last command, fork and call 'h 1' which is 'h 1', ...          */
	/* this is in theory (i'm not even going to try it)               */

	/* to try and avoid it we will do the following.                  */
	/* 1. get the item                                                */
	/* 2. test to see if it also a call to execute a history command  */
	/* if it is, then we say 'NO RECURSIVE HISTORY CALLS ALLOWED'     */
	/* of not we use the goto to enter the while loop without reading */
	/* a new line, instead use the returned history buffer as input   */
	
	int n = atoi(t->params[1]);

#ifdef DEBUG_INFO	
	printf("CALL TO EXECUTE HISTORY COMMAND : %d\n", n);
#endif

	history_insert(line_buffer, line_length);
		
	int h = history_get_item(n, line_buffer);		
	parsing_free(t);

	if (h != 0) {
	  printf("NO HISTORY ITEM: %d", n);
	  goto inputloop;
	} else {
	  line_length = strlen(line_buffer);
	  t = parsing_tokenize_line(line_buffer, line_length);
	  if (t->special_call == BUILTIN_EXECUTE_HISTORY) {
	    printf("NO RECURSIVE HISTORY CALLS ALLOWED!\n");
	    parsing_free(t);
	    goto inputloop;
	  } else {
	    parsing_free(t);
	    goto replloop;
	  }
	}
      }      

      
      /* CALLING SAFEFORK */

      pid_t pid = safefork();

      if (pid == 0) {
	execv(t->params[0], t->params);
	exit(0);
      }
      else if (pid > 0) {
	if (t->special_call == RUN_IN_BACKGROUND) {
#ifdef DEBUG_INFO
	printf("CALL TO FORK AND RUN IN BACKGROUD\n");
#endif
	printf("[%d]\n", pid);
	} else {
#ifdef DEBUG_INFO
	  printf("CALL TO FORK\n");
#endif	    
	  int returnStatus;
	  waitpid(pid, &returnStatus, 0);
	}
      }
      else {
	printf("fork error\n");
      }
      history_insert(line_buffer, line_length);      
    }
    /* END FORK SECTION  */          
    
    /* LINE WAS TOO LONG,  */
    
    else {
      /* read from buffer to empty previoius input  */
      /* VERY IMPORTANT                             */      
      while (fgetc(stdin) != '\n') {}
      line_length = 0;
      fprintf(stderr, "[OOPS!] input exceeded maximum line length of %d characters\n", MAX_LINE_LENGTH);
    }
    
  }
  
}
