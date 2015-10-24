#include "repl.h"
#include "repl_parsing.h"
#include "repl_history.h"


int main(int argc, char** argv) {

  history_init();
  char* line1 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  char* line2 = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
  char* line3 = "cccccccccccccccccccccccccccccccccccccccccccc";
  char* line4 = "dddddddddd";
  char* line5 = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
  char* line6 = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
  char* line7 = "gggggggggggggggggggggggggggggggggggggggggggggggggggggggg";
  char* line8 = "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh";
  char* line9 = "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii";
  char* line10 = "jjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjj";
  char* line11 = "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk";
  char* line12 = "l";
  char* line13 = "mmmmmmmmmm";
  char* line14 = "n";
  char* line15 = "o";

  history_insert(line1);
  printf("%s\n", line1);
  char xs[121];
  history_delete_last_n_items(1);
  int i = history_get_item(0, xs);
  if (i == 0) {
    printf("%s\n",xs);
  }
  
  /* history_insert(line3); */
  /* history_insert(line4); */
  /* history_insert(line5); */
  /* history_insert(line6); */
  /* history_insert(line7); */
  /* history_insert(line8); */
  /* history_insert(line9); */
  /* history_insert(line10); */
  /* history_insert(line11); */
  /* history_insert(line12); */
  /* history_insert(line13); */
  /* history_insert(line14); */
  /* history_insert(line15); */
}



