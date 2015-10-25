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

 loop:while (1) {
   
    fprintf(stdout, "%s@ifish %d > ", uname, command_count);
    fgets(line_buffer, LINE_BUFFER_SIZE, stdin);
    if (feof(stdin)) {fputc('\n', stdout); exit(0);}
    command_count++ ;

    /* check that the input line is not TOO long    */
    
    if ((line_length = strlen(line_buffer)) <= MAX_LINE_LENGTH) {
      struct tokenized* t = parsing_tokenize_line(line_buffer, line_length);
      if (t == NULL) {
	printf("invalid command\n");
	goto loop;
      }
      if (t->special_call == BUILTIN_DELETE_HISTORY) {
	int n = atoi(t->params[2]);
#ifdef DEBUG_INFO	
	printf("CALL TO DELETE HISTORY\n");
#endif
	history_delete_last_n_items(n);
	history_insert(line_buffer, line_length);
      }
      else if (t->special_call == BUILTIN_EXECUTE_HISTORY) {
	int n = atoi(t->params[1]);
#ifdef DEBUG_INFO	
	printf("CALL TO EXECUTE HISTORY COMMAND : %d\n", n);
#endif
	history_insert(line_buffer, line_length);
      }
      else if (t->special_call == RUN_IN_BACKGROUND) {
#ifdef DEBUG_INFO
	printf("CALL TO FORK AND RUN IN BACKGROUD\n");
#endif
	history_insert(line_buffer, line_length);
      } else {
#ifdef DEBUG_INFO
	printf("CALL TO FORK\n");
#endif
	history_insert(line_buffer, line_length);
      }
    } else {
      /* read from buffer to empty previoius input  */
      /* VERY IMPORTANT                             */      
      while (fgetc(stdin) != '\n') {}
      line_length = 0;
      fprintf(stderr, "[OOPS!] input exceeded maximum line length of %d characters\n", MAX_LINE_LENGTH);
    }

  }

  
}
