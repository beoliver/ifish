#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_CHARS 7
#define BUFFER_SIZE 8

/* ugly hack... consumes the rest of the line if it is longer than */
/* the length of the buffer. Will mean that we can print messages  */
/* to the user */

int read_repl_line(int buffer_size, char* buffer) {
  int i = 0;
  char* p = buffer;
  int flag = 1;
  int ch = fgetc(stdin);
  while (ch != EOF) {
    while (ch != '\n') {
      if ((i++) == (buffer_size-1)) {
	goto consume_and_return_error;
      }
      *(p++) = (char) ch;
      ch = fgetc(stdin);
      if (ch == EOF) {
	goto exit_stage;
      }
    }
    *p = '\0'; return i;
  }
 exit_stage:
    exit(0);
 consume_and_return_error:
    
       
    
    while ((c = fgetc(stdin)) != '\n') {
      if (feof(stdin)) {printf("\n"); exit(0);}
      if ((i++) == (buffer_size-1)) {
	flag = 0; break; }
      *(p++) = (char) c;
    }
    if (flag == 0) {
      while ((c = fgetc(stdin)) != '\n') {
	if (feof(stdin)) {printf("\n"); exit(0);}
      }
    } else {
      *p = '\0';
      return i;
    }
    return -1;
  }



int main(int argc, char** argv) {

  char buffer[BUFFER_SIZE];

  while (1) {

    printf("> ");

    int len = read_repl_line(BUFFER_SIZE, buffer);

    
    if (len > 0) {
      printf("length %d, %s\n", len, buffer);
    }

    
  }  
  return 0;
}

