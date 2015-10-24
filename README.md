# ifish

ifish (ifi sh) is a VERY basic shell implementation. It is basically a repl with 512 bytes of memory that forks (see the rather strange ```safefork.c```) for system calls. 

The structure is broken into three main 'modules'
```
repl_history
repl_parsing
repl
```
```repl_history``` uses a couple of static structs to keep track of memory. It provides the following interface:
```
void history_init()
void history_insert(char*)
int  history_get_item(int, char*)
int  history_delete_last_n_items(int)
```
and if I get round to it
```int history_teardown()```
All other functions in ```repl_history``` are static.

```repl_parsing``` handles both searching the users path and tokenizing lines. It provides the following interface:
```
char*        parsing_search_path(char*)
repl_params* parsing_tokenize(char*)
```
