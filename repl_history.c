#include "repl_history.h"

/* --------------------------------------------------------------------------- */

struct mem_blocks {
  char bitmap[8];
  char blocks[512];
  int  space;
} ;

struct mem_meta {
  struct mem_meta* next;
  int  block_addrs[15];
  int  size;
} ;

/* --------------------------------------------------------------------------- */

/* use two sepeare structs. as all history managment is done */
/* within the scope of topdown_history.c, we don't need to */
/* worry too much about passing state. */

static struct mem_blocks*   mem_b;
static struct mem_meta*     mem_m;

/* --------------------------------------------------------------------------- */

static void print_char_as_binary (char byte, FILE* stream) {
  /* prints the sequential bits of the char from left to right */
  for (char offset = 7; offset >= 0; offset--) {
    fputc( byte & (1 << offset) ? '1' : '0', stream ) ;   
  }
}

static void print_bitmap () {
#ifdef DEBUG  
  fprintf(DEBUG_OUT, "DEBUG - BITMAP:\n\n");
#endif  
  int position = 0;  
  for (int i = 0; i < 2; i++) {
    fputc('\n', DEBUG_OUT);
    for (int j = 0; j < 4; j++) {
      print_char_as_binary(mem_b->bitmap[position], DEBUG_OUT);
      position++;
    }
  }
  fprintf(DEBUG_OUT,"\n\n");
}

/* --------------------------------------------------------------------------- */

static void print_datablocks () {
#ifdef DEBUG  
  fprintf(DEBUG_OUT, "DEBUG - DATABLOCKS:\n\n");
#endif  
  char* b = mem_b->blocks;  
  for (int row_count = 0; row_count < 16; row_count++) {    
    fprintf(DEBUG_OUT, "##");   
    for (int column_count = 0; column_count < 4; column_count++) {
      for (int i = 0; i < 8; i++) {
	if ((b[i] == 0) || (b[i] == '\n'))   {
	  fputc(' ', DEBUG_OUT);
	} else {
	  fputc(b[i], DEBUG_OUT);
	}
      }	  
      fprintf(DEBUG_OUT, "##");
      b += 8;
    }
    fputc('\n', DEBUG_OUT);
  }
  fputc('\n', DEBUG_OUT);
}

/* --------------------------------------------------------------------------- */

static void print_meta_nodes () {
  if (mem_m == NULL) {
    fprintf(DEBUG_OUT, "NO NODES TO PRINT\n");
  } else {
    int node_count = 1;
    struct mem_meta* temp = mem_m;
    while (temp != NULL) {
      fprintf(DEBUG_OUT, "NODE @ position %d has SIZE of %d BLOCKS : first index is %d\n",
	      node_count, temp->size, temp->block_addrs[0]);
      temp = temp->next;
      node_count++;
    }
  }
}

/* --------------------------------------------------------------------------- */

static void alloc_n_blocks(int n, int* meta_index_buffer) {
  
  /* Where n is the number of bits to allocate.                    */
  /* it is ASSUMED that THERE EXISTS AT LEAST n bits with value 0. */
  /* Searches the bitmap for n values equal to 0. When a 0 valued  */
  /* bit is found in the bitmap it is set to 1. The bits INDEX is  */
  /* stored in the meta_index_buffer.                              */
  /* the mem_b space field (int) is decremented                    */
  
  /* NOTE: the index VALUE is in the range 0-63                    */
  /* It is an 'block index' that all other functions must          */
  /* decide how to interpret                                       */

  char* bitmap = mem_b->bitmap; 
  int block_index = 0;
  int* arrayp = meta_index_buffer;
  for (int byte_pos = 0; byte_pos < 8; byte_pos++) {
    for (char offset = 7; offset >= 0; offset--) {
      char* byte = bitmap + byte_pos;
      char  mask = 1 << offset;
      if ((*byte & mask) == 0) {
	/* flip the bit in the bitmap, write the meta_index_buffer */
	/* and decrement mem_b space (int) field by one            */
	*byte = *byte ^ mask;
	*arrayp = block_index;
	mem_b->space--;
#ifdef DEBUG_INFO
	fprintf(DEBUG_OUT, "index saved as: %d\n", block_index);
#endif
	if ((--n) == 0) {
	  return;
	}
	/* move meta_index_buffer pointer forwards */
	arrayp++;
      }
      /* we allways increment the block_index */
      block_index++ ;
    }
  }
}

/* --------------------------------------------------------------------------- */

/* we use XOR which allows for simple flipping. We could think about using NAND */
/* which would mean that we ONLY deallocate bits set to 1. */

static void dealloc_n_blocks(int n, int* meta_index_buffer) {
  
  /* where n is the number of items to read from meta_index_buffer */
  /* THE BIT VALUES ARE NOT TESTED. Any bit located at a position  */
  /* refered to by an index in meta_index_buffer will be fliped.   */
  /* the mem_b space field (int) is incremented                    */
  
  char* bitmap = mem_b->bitmap;
  for (int i = 0; i < n; i++) {
    
    /* As the meta_index_buffer values are in the range 0-63 we    */
    /* must divide the values by 8 to map them to the 8 byte       */
    /* bitmap. This gives us the byte that we need. By taking the  */
    /* values modulo 8, we calculate the offset (bit position)     */
    /* shift 0000001 left by 7-offset (left to right) and xor      */
    
    char* byte = bitmap + (meta_index_buffer[i] / 8);
    *byte = *byte ^ (1 << (7 - (meta_index_buffer[i] % 8)));
    mem_b->space++;

#ifdef CLEAN_DATABLOCKS
    /* overwrite the associated datablocks with 0. Not actually needed, */
    /* as we use the bitmap and the meta data. But in case it is */
    /* 'required'  for the oblig the flag can be set */
    memset(mem_b->blocks+(meta_index_buffer[i] * 8), 0, 8);
#endif
    
  }
}

/* --------------------------------------------------------------------------- */

static void write_line_to_mem_blocks(char* line, int n, int* meta_index_buffer) {
  
  /* where n is the number of items to read from meta_index_buffer */
  /* multiply meta_index_buffer values by 8 to get relative byte   */
  /* position in the datablock memory (512 bytes in total)         */
  
#ifdef DEBUG_STRICT
  assert(mem_b->blocks != NULL);
  assert(line != NULL);
  assert(meta_index_buffer != NULL);
#endif
  
  char* offset_blockp;
  char* linep = line;
  
  /* use strncpy(char * dst, const char * src, size_t len)         */
  /* from the man pages: If src is less than len characters long,  */
  /* the REMAINDER OF DST IS FILLED WITH `\0' CHARACTERS.          */
  /* Otherwise, dst is not terminated.                             */
  
  /* means that we never need to set datablocks to 0 when deleting */
  
  for (int i = 0; i < n; i++) {	
    offset_blockp = mem_b->blocks+(8 * meta_index_buffer[i]);
    strncpy(offset_blockp, linep, 8);
    linep += 8;
  }
}

/* --------------------------------------------------------------------------- */

static void get_line_from_mem_blocks(int n, int* meta_index_buffer, char* buffer) {

  /* ASSUME buffer is at least 121 bytes long */
  
  int i;
  
  for (i = 0; i < n; i++) {
    strncpy(buffer+(i*8), mem_b->blocks+(meta_index_buffer[i]*8), 8);
  }
  
  memset(buffer+(i*8),'\0',1);
}

/* --------------------------------------------------------------------------- */

static int delete_most_recent() {
  if (mem_m == NULL) {
    return (-1);
  }
  struct mem_meta* temp = mem_m;
  mem_m = mem_m->next;
  dealloc_n_blocks(temp->size, temp->block_addrs);
  free(temp);

  return 0;
}

/* --------------------------------------------------------------------------- */

void history_init() {
  mem_b = malloc(sizeof(struct mem_blocks));
  memset(mem_b->bitmap, 0, 8);
  memset(mem_b->blocks, 0, 512);
  mem_b->space = 64; /* should this be set to 63?  */
  mem_m = NULL;  
}

/* --------------------------------------------------------------------------- */

void history_insert(char* line) {
  
  int len = strlen(line);
  int blocks_required = (len % 8) ? 1+(len / 8) : (len / 8);
  
#ifdef DEBUG_INFO
  fprintf(DEBUG_OUT, "new insert has %d chars and requires %d blocks\n", len, blocks_required);
  fprintf(DEBUG_OUT, "there are currently %d free blocks\n", mem_b->space);
#endif  
#ifdef DEBUG_STRICT
  assert ((len <= 120) && (blocks_required <= 15)) ;
#endif  

  if (mem_b->space < blocks_required) {
    
    /* need to dealloc space and remove nodes before continuing   */
    /* REMEMBER -- the OLDEST item is the last in the linked list */
    
    while (mem_b->space < blocks_required) {
#ifdef DEBUG_STRICT		
      assert (mem_m != NULL);
#endif
      /* need to get to the last element                        */
      
      struct mem_meta* temp1 = NULL;
      struct mem_meta* temp2 = mem_m;
      
      while (temp2->next != NULL) {
	temp1 = temp2;
	temp2 = temp2->next;
      }
      
      if (temp1 != NULL) {
	/* then temp2 IS NOT the root struct                    */
	/* let's delete temp2                                   */
	dealloc_n_blocks(temp2->size, temp2->block_addrs);
	/* NEVER TRY TO 'FREE' THE BLOCK_ADDRS !!! */
	free(temp2);
	temp1->next = NULL;
      } else {
	/* then temp2 IS the root struct     */
#ifdef DEBUG_STRICT
	assert(temp2 == mem_m);
#endif
	dealloc_n_blocks(mem_m->size, mem_m->block_addrs);
	free(mem_m);
	mem_m = NULL;
      }
    }
  }
  /* we now have the required blocks. */
  /* insert into the HEAD of the linked list */
  struct mem_meta* node = malloc(sizeof(struct mem_meta));	
  if (node == NULL) {
    perror("meta malloc :");
  }
  node->size = blocks_required;
  node->next = NULL;

  alloc_n_blocks(blocks_required, node->block_addrs);
  write_line_to_mem_blocks(line, blocks_required, node->block_addrs);
			   
#ifdef DEBUG_STRICT
  assert (node->next == NULL);
#endif
  
  struct mem_meta* temp = mem_m;
  mem_m = node;
  node->next = temp;
  
#ifdef DEBUG_STRICT
  assert (node->next == temp);
#endif

#ifdef DEBUG
  print_bitmap();
  print_datablocks();
#endif
  
#ifdef DEBUG_INFO
  print_meta_nodes();
#endif  
}

/* --------------------------------------------------------------------------- */

int history_get_item(int n, char buffer[121]) {

  /* where n is the nth YOUNGEST item in memory */ 
  
  /* make sure that we do not allow recursive calls. */
  /* If the first thing we type is 'h 0'             */
  /* then we will return the call to 'h 0'           */
  /* this will call again, and again ...             */
    
  struct mem_meta* temp = mem_m;
  if (temp == NULL) {
    fprintf(stderr, "no history exists yet... bold move cotton");    
    return (-1);
  }
  
  while (n > 0) {
    temp = temp->next;
    if (temp == NULL) {
      fprintf(stderr, "maximm history limit: %d", n);
      return (-1);
    }
    n--;
  }

  /* get the blocks given temp->block_addrs */
  get_line_from_mem_blocks(temp->size, temp->block_addrs, buffer);

#ifdef DONT_FORK_BOMB_YOURSELF
  /* check that return value is not a call to history */
#endif

  return 0;
  
}

/* --------------------------------------------------------------------------- */

int history_delete_last_n_items(int n) {
  /* delete from head of list... */   
  while (n > 0) {
    int ret = delete_most_recent();
    if (ret == (-1)) {
      return (-1);
    }
  }
  return 0;
}

/* --------------------------------------------------------------------------- */
