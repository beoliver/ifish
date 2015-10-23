#include "repl_history.h"
#include <errno.h>

#define BITMAP_BYTE_COUNT 8
#define DATABLOCKS_BYTE_COUNT 512
#define HISTORY_SLOT_COUNT 64
#define DATABLOCK_BYTE_COUNT 8

#define DEBUG
/* #define CLEAN_DATABLOCKS */

struct metadata {
  struct metadata* next;
  int  blockIndexes[15];
  int  used_block_count;
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


static int required_blocks(int len) {
  /* how many 8 byte blocks are required to store a string of length len */
  return (len % DATABLOCK_BYTE_COUNT) ? 1+(len / DATABLOCK_BYTE_COUNT) : (len / DATABLOCK_BYTE_COUNT);
}

static void delete_blocks_at(char* datablocks, int* indices, int indices_length) {
  /* indices are in the range of 0 - 63 (i.e indices[n] 'refers' to a datablock index) */
  /* we don't actually need to write over our freed blocks, as they are not in the bit map */
  /* they will eventually be written over. As we use strncpy(block, lineChunk, 8) the last block */
  /* will end with 0's (check man pages) */
  /* The ONLY time we would over write them is to */
  /* pretty print the debug view of memory (which is not a good reason imho) */
  for (int i = 0; i < indices_length; i++) {
    memset(datablocks+(indices[i]*8), 0, 8);
  }
}


static void flip_bit_at(char* bitmap, int index) {
  /* index is in the range of 0 - 63 (i.e index 'refers' to a datablock index) */
  char* byte = bitmap + (index / 8); 
  *byte = *byte ^ (1 << (7 - (index % 8)));
}

static void flip_bits_at(char* bitmap, int* indices, int indices_length) {
  /* indices are in the range of 0 - 63 (i.e indices[n] 'refers' to a datablock index) */
  for (int i = 0; i < indices_length; i++) {
    flip_bit_at(bitmap, indices[i]);
  }
}



static int free_oldest_item() {
  if (hist->meta == NULL) {
    /* nothing to do */
    return 0;
  }  
  flip_bits_at(hist->bitmap, hist->meta->blockIndexes, hist->meta->used_block_count);
#ifdef CLEAN_DATABLOCKS
  delete_blocks_at(hist->datablocks, hist->meta->blockIndexes, hist->meta->used_block_count);
#endif
  hist->free_block_count += hist->meta->used_block_count;  
  /* and now we can remove the meta pointer */
  struct metadata* temp = hist->meta;
  hist->meta = hist->meta->next;
  free(temp->blockIndexes);
  free(temp);
  return 0;
}

static void find_and_allocate_n_bits(char* bitmap, int n, int* index_buffer) {

  /* this function ONLY modifies the BITMAP and the INDEX_BUFFER */
  /* This function will ONLY WORK if : */
     /* 1. there exist N or more BITS set to 0 */
     /* 2. index_buffer has memory allocated for N or more INT's */
  /* this is ONLY guaranteed by first testing the memory and freeing DATABLOCKS AND BITS before */
  /* IT IS NOT THIS FUNCTIONS JOB TO DELETE BITS. */
  /* IT IS NOT THIS FUNCTIONS JOB TO INSERT DATABLOCKS */
  
  /* read through the bitmap, when we find a free bit, we do two things */
  /*   1. FLIP it */
  /*   2. add index to index_buffer */

#ifdef DEBUG  
  assert(index_buffer != NULL);
  assert(bitmap != NULL);
#endif
  
  int block_index = 0;
  int* arrayp = index_buffer;

  for (int byte_pos = 0; byte_pos < BITMAP_BYTE_COUNT; byte_pos++) {
    for (char offset = 7; offset >= 0; offset--) {
      char* byte = bitmap + byte_pos;
      char  mask = 1 << offset;
      if ((*byte & mask) == 0) {
	/* block is CONSIDERED free */
	/* flip the bit in the bitmap and write the block_index to index_buffer */
	*byte = *byte ^ mask;
	*arrayp = block_index;
	/* test to see if n == 0, if so just return as work is complete */
	if ((--n) == 0) {
	  return;
	}
	arrayp++;
      }
      /* we allways increment the block_index */
      block_index++ ;
    }
  }
}




static void write_line_to_datablocks(char* datablocks, char* line, int blocks_required, int* index_buffer) {
  
  /* we ASSUME that we will never overwrite data, i.e all block_indexes required are free */
  
#ifdef DEBUG
  assert(datablocks != NULL);
  assert(line != NULL);
  assert(index_buffer != NULL);
#endif
  
  int block_offset;
  char* blockp;
  char* linep = line;
  
  for (int i = 0; i < blocks_required; i++) {
    /* at the moment indexes are in the range 0 - 63 */
    block_offset = 8 * index_buffer[i];
    /* block_offset is in the range 0 - 504 (multiples of 8) */
    blockp = datablocks + block_offset;
    
#ifdef CLEAN_DATABLOCKS    
    assert(*blockp == 0);
#endif
    
    strncpy(blockp, linep, 8);
    linep += 8;
  }  
}



static void insert(char line[121], int required_blocks) {
  struct metadata* m = hist->meta;
  while (m != NULL) {
      m = m->next;
    }
  m = malloc (sizeof(struct metadata));
  m->used_block_count = required_blocks;
  find_and_allocate_n_bits(hist->bitmap, required_blocks, m->blockIndexes);
  write_line_to_datablocks(hist->datablocks, line, required_blocks, m->blockIndexes);
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


void repl_history_display() {
  if (hist == NULL) {
    fprintf(stderr, "NO HISTORY TO DISPLAY... ");
  } else {
    fprint_bitmap (stderr, hist->bitmap);
    fprint_datablocks (stderr, hist->datablocks);
  }
}



int repl_history_insert(char line[121]) {
  int errv;
  char* error_message;
  int rq = required_blocks(strlen(line));
#ifdef DEBUG
  fprintf(stderr, "required blocks for line: %s = %d\n", line, rq);
#endif
  if (rq <= hist->free_block_count) {
    /* we can insert straight away */
    insert(line, rq);
    return 0;
  } else {
    /* we need to free memory before inserting */
    int fr;
    while (rq > hist->free_block_count) {
      int fbc = hist->free_block_count;
      fr = free_oldest_item();
      if ((fr != 0) || (fbc == hist->free_block_count)) {
	error_message = strerror(errv);
	fprintf(stderr, "repl_history_insert() failed while freeing memory: %s\n",
		error_message);
	return -1;
      }      
    }
  }
  /* we can now insert */
  insert(line, rq);
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
