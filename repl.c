#include "repl.h"
#include <errno.h>

int main(int argc, char** argv) {

  int insert_val;
  
  int init_val = repl_history_init();

  char* line1 = "ls -lahF >> foo.txt &\n";
  char* line2 = "who -m\n";
  char* line3 = "arp -a\n";

  insert_val = repl_history_insert(line1);
  repl_history_display();
  insert_val = repl_history_insert(line2);
  repl_history_display();
  insert_val = repl_history_insert(line3);
  repl_history_display();
  if (insert_val != 0) {
    /* we could not insert */
  } else {
    /* everything was fine */
  }
  
}
