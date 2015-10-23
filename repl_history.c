#include "repl_history.h"
#include <errno.h>

#define BITMAP_BYTE_COUNT 8
#define DATABLOCKS_BYTE_COUNT 512
#define HISTORY_SLOT_COUNT 64

struct metadata {
  struct metadata* next;
  int  blocks[15];
  int  used_block_count;
  int  line_length;
};

struct history {
  char bitmap[BITMAP_BYTE_COUNT];
  char datablocks[DATABLOCKS_BYTE_COUNT];
  int  free_block_count;
  struct metadata* meta;
};

static struct history* hist; 



/* HELPERS */

static void fprint_bitmap_block (FILE* stream, char byte) {
  /* prints the sequential bits of the char FROM LEFT TO RIGHT */
  for (char offset = 7; offset >= 0; offset--) {
    fputc( byte & (1 << offset) ? '1' : '0', stream ) ;   
  }
}


static void fprint_bitmap (FILE* stream, char* bitmap) {
#ifdef DEBUG  
  fprintf(stream, "DEBUG - BITMAP:\n\n");  
#endif
  int position = 0;
  
  for (int i = 0; i < 2; i++) {
    fputc('\n', stream);
    for (int j = 0; j < 4; j++) {
      fprint_bitmap_block(stream, bitmap[position]);
      position++;
    }
  }
  fprintf(stream,"\n\n");
}


static void fprint_8byte_datablock (FILE* stream, char* block) {
  /* assumes that block pointer is correct */
  /* does not protect from printing null   */
  for (int i = 0; i < 8; i++) { 
    if ((block[i] == 0) || (block[i] == '\n'))   {
      fputc(' ', stream);
    } else {
      fputc(block[i], stream);
    }
  }
}


static void fprint_datablocks (FILE* stream, char* datablocks) {
#ifdef DEBUG  
  fprintf(stream, "DEBUG - DATABLOCKS:\n\n");
#endif
  char* p = datablocks;
  
  for (int row_count = 0; row_count < 16; row_count++) {    
    fprintf(stream, "##");   
    for (int column_count = 0; column_count < 4; column_count++) {        
      fprint_8byte_datablock(stream, p);
      fprintf(stream, "##");
      p += 8;
    }
    fputc('\n', stream);
  }
  fputc('\n', stream);
}



/* REPL_HISTORY_INTERFACE */

int repl_history_init() {

  /* am i going over the top? I've never really been a defensive programmer */
  
  int errv;
  char* error_message;
  hist = malloc(sizeof(struct history));
  if (hist == NULL) {
    errv = errno;
    error_message = strerror(errv);
    fprintf(stderr, "repl_history_init() could not allocate history memory: %s\n",
	    error_message);
    return -1;
  }
  memset(hist->bitmap, 0, BITMAP_BYTE_COUNT);
  if (hist->bitmap[0] != 0) {
    errv = errno;
    error_message = strerror(errv);
    fprintf(stderr, "repl_history_init() could not set bitmap values to 0: %s\n",
	    error_message);
    free(hist->bitmap);
    free(hist->datablocks);
    free(hist);
    return -1;
  }
  memset(hist->datablocks, 0, DATABLOCKS_BYTE_COUNT);
  if (hist->datablocks[0] != 0) {
    errv = errno;
    error_message = strerror(errv);
    fprintf(stderr, "repl_history_init() could not set datablock values to 0: %s\n",
	    error_message);    
    free(hist->bitmap);
    free(hist->datablocks);
    free(hist);
    return -1;
  }
  hist->free_block_count = HISTORY_SLOT_COUNT;
  return 0;
}




int repl_history_insert(char* line) {
  return 0;
}

int repl_history_get(int n, char* buffer) {
  return 0;
}

int repl_history_delete(int n) {
  return 0;
}

int repl_history_teardown() {
  return 0;
}
