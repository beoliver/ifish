#ifndef __REPL_HISTORY_H__
#define __REPL_HISTORY_H__

/* all defininitions to be kept in 'repl.h' */

#include "repl.h"

void history_init();
void history_insert(char*, int);
int  history_get_item(int, char*);
int  history_delete_last_n_items(int);

#endif
