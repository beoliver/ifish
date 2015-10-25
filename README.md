# ifish

ifish (ifi sh) is a VERY basic shell implementation. It is basically a repl with 512 bytes
of memory that forks (see the rather strange ```safefork.c```) for system calls. 

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
All other functions in ```repl_history``` are static.

```repl_parsing``` handles both searching the users path and tokenizing lines. It provides the following interface:
```
struct tokenized* parsing_tokenize_line(char*)
void parsing_free(struct tokenized*)
```
When a line is passed to ```parsing_tokenize_line``` a struct of type ```struct tokenized*``` is created.
This has two fields ```char* params[21]``` and ```char special_call```.
The linebuffer is read and copied into into an array of type ```char* params[21]```
If the first symbol is ```h``` then the tokens are parsed to make sure that they are a valid **history** call.
If so, the ```token->special_call``` field is set to either ```BUILTIN_DELETE_HISTORY``` or ```BUILTIN_EXECUTE_HISTORY]```
and the token struct is returned.
If there are semantic errors then ```NULL``` is returned.
If the first token is **"exit"** or **"quit"** then the ```token->special_call```
field is set to ```USER_EXIT``` and the struct returned.

If none of these cases were true, then the **path** to the first token is found and the first pointer in the
list of tokens is replaced.

If the last token is the symbol ```&``` the ```token->special_call``` field is set to ```RUN_IN_BACKGROUND```

