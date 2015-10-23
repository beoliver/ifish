

/* declare all functions static apart from the following */
/* not 100% decided on return types */

#ifndef __REPL_HISTORY_H__
#define __REPL_HISTORY_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


int repl_history_init();
/* allocate memory for the history struct */
/* return 1 on success, 0 on failure */

int repl_history_insert(char*);
/* try to insert a line into history */
/* return 1 on success, 0 on failure */

int repl_history_get(int, char*);
/* try to copy the Nth most recent line from history into the buffer passed */
/* return 1 on success, 0 on failure */

int repl_history_delete(int);
/* try to delete the Nth most recent lines from history */
/* return 1 on success, 0 on failure */

int repl_history_teardown();
/* free memory etc... */

void repl_history_display();

#endif
