/* declare all functions static apart from the following */
/* not 100% decided on return types */

void repl_history_init();

int repl_history_insert(char*);

int repl_history_delete(int);

int repl_history_teardown();
