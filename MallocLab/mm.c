/* 
 * mm-implicit.c -  Simple allocator based on implicit free lists, 
 *                  first fit placement, and boundary tag coalescing. 
 *
 * Each block has header and footer of the form:
 * 
 *      31                     3  2  1  0 
 *      -----------------------------------
 *     | s  s  s  s  ... s  s  s  0  0  a/f
 *      ----------------------------------- 
 * 
 * where s are the meaningful size bits and a/f is set 
 * iff the block is allocated. The list has the following form:
 *
 * begin                                                          end
 * heap                                                           heap  
 *  -----------------------------------------------------------------   
 * |  pad   | hdr(8:a) | ftr(8:a) | zero or more usr blks | hdr(8:a) |
 *  -----------------------------------------------------------------
 *          |       prologue      |                       | epilogue |
 *          |         block       |                       | block    |
 *
 * The allocated prologue and epilogue blocks are overhead that
 * eliminate edge conditions during coalescing.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
  /* Team name */
  "DynamicDuo",
  /* First member's full name */
  "JoseEscobar",
  /* First member's email address */
  "jose.escobar@colorado.edu",
  /* Second member's full name (leave blank if none) */
  "AhmedAlmutawa",
  /* Second member's email address (leave blank if none) */
  "ahmed.almutawa@colorado.edu"
};

//Helpful definitions to code with, inspired by Github/ltganesan
/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */


/* Basic constants and macros */
#define WSIZE       4	/* word size (bytes) */  
#define DSIZE       8	/* doubleword size (bytes) */
#define CHUNKSIZE   (1<<12)	/* initial heap size (bytes) */
#define MINIMUM    24  /* minimum block size */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)

//
// Pack a size and allocated bit into a word
// We mask of the "alloc" field to insure only
// the lower bit is used
//
#define PACK(size, alloc)  ((size) | (alloc))

//
// Read and write a word at address p
//
#define GET(p)       (*(int *)(p))
#define PUT(p, val)  (*(int *)(p) = (val))

//
// Read the size and allocated fields from address p
//
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

//
// Given block ptr bp, compute address of its header and footer
//
#define HDRP(bp)       ((void *)(bp) - WSIZE)
#define FTRP(bp)       ((void *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

//
// Given block ptr bp, compute address of next and previous blocks
//
#define NEXT_BLKP(bp)  ((void *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)  ((void *)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

/* Given block ptr bp, compute address of next and previous free blocks */
#define NEXT_FREEP(bp)(*(void **)(bp + DSIZE))
#define PREV_FREEP(bp)(*(void **)(bp))

/////////////////////////////////////////////////////////////////////////////
// Global Variables
/////////////////////////////////////////////////////////////////////////////
static char *heap_listp = 0; /* Pointer to the first block */
static char *free_listp = 0;/* Pointer to the first free block */

//
/* Function prototypes for internal helper routines */
static void *extendHeap(size_t words);
static void place(void *bp, size_t asize);
static void *findFit(size_t asize);
static void *coalesce(void *bp);
static void printBlock(void *bp); 
static void checkBlock(void *bp);
static void insertAtFront(void *bp); /* Linked list function */
static void removeBlock(void *bp); /* Linked list function */
//Sources: CS: A Programmer's Perspective / github.com/ltganesan / Wikipedia on malloc implementations 
//
// mm_init - Initialize the memory manager 
//
int mm_init(void) 
{//pg. 831 CS:APP
	//create initial empty heap
	if ((heap_listp = mem_sbrk(2*MINIMUM)) == NULL) 
		return -1;

	PUT(heap_listp, 0); //Used for alignment

	PUT(heap_listp + WSIZE, PACK(MINIMUM, 1)); //WSIZE pads
	PUT(heap_listp + DSIZE, 0); //for PREV pointer
	PUT(heap_listp + DSIZE+WSIZE, 0); //for NEXT pointer
	
	PUT(heap_listp + MINIMUM, PACK(MINIMUM, 1)); //block footer
	
	PUT(heap_listp+WSIZE + MINIMUM, PACK(0, 1)); //tail block
	
	free_listp = heap_listp + DSIZE; //free list pointer -> tail block

	//return -1 if heap space is unavailable
	if (extendHeap(CHUNKSIZE/WSIZE) == NULL) 
		return -1;
	return 0;
}

//
// extend_heap - Extend heap with free block and return its block pointer
//
static void *extendHeap(size_t words) 
{//Pg.831 CS:APP
  char *bp;
  size_t size;

	//allocates even amount of words for alignment
  size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
  if (size < MINIMUM)
    size = MINIMUM;
	//check that maintains minimum
  if ((long)(bp = mem_sbrk(size)) == -1) 
    return NULL;
	//check for positive/real size

   // Initialize free block header/footer and the epilogue header 
  PUT(HDRP(bp), PACK(size, 0));  //free block header 
  PUT(FTRP(bp), PACK(size, 0));  //free block footer 
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //new epilogue header 

  return coalesce(bp); //coalesce if prev_blk is free 
}

//
// Practice problem 9.8
//
// find_fit - Find a fit for a block with asize bytes 
//
static void *findFit(size_t asize)
{//PP 9.8 ^^
  void *bp;
 //search through blocks
  for (bp = free_listp; GET_ALLOC(HDRP(bp)) == 0; bp = NEXT_FREEP(bp)) 
  {
  	//iterate through list
    if (asize <= (size_t)GET_SIZE(HDRP(bp)))
      return bp;
      }//find block large enough to hold desired size
  return NULL; // No fit
}

/* 
 * mm_free - Free a block 
 * This function adds a block to the free list.
 * This function takes a block pointer as a parameter.
 */
void mm_free(void *bp)
{//Pg. 833 CS:APP
  if(!bp) 
  	return; //return if the pointer is NULL
  size_t size = GET_SIZE(HDRP(bp));

  //set header and footer to unallocated. Using block pointer, set the allocation bits to 0 in both the header and footer of the block.
  PUT(HDRP(bp), PACK(size, 0)); 
  PUT(FTRP(bp), PACK(size, 0));
  
  coalesce(bp); //coalesce and add the block to the free list
}

/*
 * coalesce - boundary tag coalescing. Return ptr to coalesced block 
*/
static void *coalesce(void *bp)
{//Pg.833 CS:APP / ltganesan
  size_t prev_alloc;
  prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

//3 available options:
  //1. Previous block is allocated, next block is free
//Current block merged with next block,header of current block/footer of next block updated with combined sizes of current/next blocks
  if (prev_alloc && !next_alloc) 
  {     
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    removeBlock(NEXT_BLKP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }

 //2. Previous block is annotated next block is not
//Previous block merged with current block, head of prev/ foot of current block are updated with size of two blocks
  else if (!prev_alloc && next_alloc) 
  {   
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    bp = PREV_BLKP(bp);
    removeBlock(bp);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }

  //3. Previous and next block are not allocated
//Extend block in both directions
  else if (!prev_alloc && !next_alloc) 
  {   
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
        GET_SIZE(HDRP(NEXT_BLKP(bp)));
    removeBlock(PREV_BLKP(bp));
    removeBlock(NEXT_BLKP(bp));
    bp = PREV_BLKP(bp);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }
  
  insertAtFront(bp);
  
  return bp;
}

/*
 * mm_malloc - Allocate a block with at least size bytes of payload 
 */
void *mm_malloc(size_t size) 
{//Pg.834 CS:APP / ltganesan
	size_t asize; //adjusted block size
	size_t extendsize; //extend amount
	char *bp;

	if (size <= 0)
		return NULL;

	//adjusts size to make up for any overflow or specific alignment guidelines
	asize = MAX(ALIGN(size) + DSIZE, MINIMUM);

	//search available list to find free space
	if ((bp = findFit(asize))) 
	{
		place(bp, asize);
		return bp;
	}

	//no memory = get more memory then place
	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extendHeap(extendsize/WSIZE)) == NULL) 
		return NULL; 	//return NULL if unable to get heap space
	place(bp, asize);
	return bp;
} 

/* 
 * place - Place block of asize bytes at start of free block bp 
 */
static void place(void *bp, size_t asize)
{//PP 9.9 / lganesan
  /* Gets the size of the whole free block */
  size_t csize = GET_SIZE(HDRP(bp));

  //If block remainder after splitting would be greater than or equal to min blcok size, then split block
//place blocks 
  if ((csize - asize) >= MINIMUM) {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    removeBlock(bp);
    //move to next block
    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(csize-asize, 0));
    PUT(FTRP(bp), PACK(csize-asize, 0));
    coalesce(bp);
  }
	//if not enough space, dont split block
  else {
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
    removeBlock(bp);
  }
}


/*
 * mm_realloc - Reallocate a block
 */
void *mm_realloc(void *ptr, size_t size)
{//help from ltganesan
	size_t oldsize;
	void *newptr;
	size_t asize = MAX(ALIGN(size) + DSIZE, MINIMUM);
	 // If size <= 0 then this is just free, and we return NULL. 
	if(size <= 0) {
		free(ptr);
		return 0;
	}

	 // If oldptr is NULL, then this is just malloc. 
	if(ptr == NULL) {
		return malloc(size);
	}

	 // Get the size of the original block 
	oldsize = GET_SIZE(HDRP(ptr));
	
	 // If the size doesn't need to be changed, return orig pointer 
	if (asize == oldsize)
		return ptr;
	
	 // If the size needs to be decreased, shrink the block and 
	 // * return the same pointer 
	if(asize <= oldsize)
	{
		size = asize;

		 // If a new block couldn't fit in the remaining space, 
		 // * return the pointer 
		if(oldsize - size <= MINIMUM)
			return ptr;
		PUT(HDRP(ptr), PACK(size, 1));
		PUT(FTRP(ptr), PACK(size, 1));
		PUT(HDRP(NEXT_BLKP(ptr)), PACK(oldsize-size, 1));
		free(NEXT_BLKP(ptr));
		return ptr;
	}

	newptr = malloc(size);

	 // If realloc() fails the original block is left untouched  
	if(!newptr) {
		return 0;
	}

	 // Copy the old data. 
	if(size < oldsize) oldsize = size;
	memcpy(newptr, ptr, oldsize);

	 // Free the old block. 
	free(ptr);

	return newptr;
}
 
// mm_checkheap - Check the heap for consistency 

void mm_checkheap(int verbose) 
{
  void *bp = heap_listp; 

  if (verbose)
    printf("Heap (%p):\n", heap_listp);

  if ((GET_SIZE(HDRP(heap_listp)) != MINIMUM) || 
      !GET_ALLOC(HDRP(heap_listp)))
    printf("Bad prologue header\n");
  checkBlock(heap_listp); 

  for (bp = free_listp; GET_ALLOC(HDRP(bp))==0; bp = NEXT_FREEP(bp)) 
  {
    if (verbose) 
      printBlock(bp);
    checkBlock(bp);
  }

  if (verbose)
    printBlock(bp);

  if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
    printf("Bad epilogue header\n");
}

static void printBlock(void *bp)
{//additions from ltganesan
  int hsize, halloc, fsize, falloc;

  hsize = GET_SIZE(HDRP(bp));
  halloc = GET_ALLOC(HDRP(bp));
  fsize = GET_SIZE(FTRP(bp));
  falloc = GET_ALLOC(FTRP(bp));

  if (hsize == 0) 
  {
    printf("%p: EOL\n", bp);
    return;
  }
  
   // Prints out header and footer info if it's an allocated block.
   // * Prints out header and footer info and next and prev info
   // * if it's a free block.
  
  if (halloc)
    printf("%p: header:[%d:%c] footer:[%d:%c]\n", bp,
      hsize, (halloc ? 'a' : 'f'),
      fsize, (falloc ? 'a' : 'f'));
  else
    printf("%p:header:[%d:%c] prev:%p next:%p footer:[%d:%c]\n",
      bp, hsize, (halloc ? 'a' : 'f'), PREV_FREEP(bp),
      NEXT_FREEP(bp), fsize, (falloc ? 'a' : 'f'));
}


static void checkBlock(void *bp)
{
  // Check if the next and prev pointers are within heap bounds
  if (NEXT_FREEP(bp)< mem_heap_lo() || NEXT_FREEP(bp) > mem_heap_hi())
    printf("Error: next pointer %p is not within heap bounds \n"
        , NEXT_FREEP(bp));
  if (PREV_FREEP(bp)< mem_heap_lo() || PREV_FREEP(bp) > mem_heap_hi())
    printf("Error: prev pointer %p is not within heap bounds \n"
        , PREV_FREEP(bp));

  if ((size_t)bp % 8)
    printf("Error: %p is not doubleword aligned\n", bp);

  
  if (GET(HDRP(bp)) != GET(FTRP(bp)))
    printf("Error: header does not match footer\n");
}

/*
 * calloc - Allocate the block and set it to zero.
 * This function allocates a block of given size and sets it to 0.
 * First, malloc the size payload desired. Then, set memory to 0.
 *
 * This function takes 2 parameters: number f elements in an array and 
 * the size of each element.
 * It returns the block pointer to the newly allocated block.
*/
void *calloc (size_t nmemb, size_t size)
{//from ltganesan
        size_t bytes = nmemb * size;
        void *newptr;

        newptr = malloc(bytes);
        memset(newptr, 0, bytes);

        return newptr;
}

/*
 * insertAtFront - Inserts a block at the front of the free list
 * This function takes a block pointer of the block to add to the 
 * free list as a parameter.
 */
static void insertAtFront(void *bp)
{
	NEXT_FREEP(bp) = free_listp; //Sets next ptr to start of free list
	PREV_FREEP(free_listp) = bp; //Sets current's prev to new block
	PREV_FREEP(bp) = NULL; // Sets prev pointer to NULL
	free_listp = bp; // Sets start of free list as new block
}

/*
 * removeBlock - Removes a block from the free list
 * This function takes a block pointer of the block to remove as a
 * parameter.
 */
static void removeBlock(void *bp)
{
	 // If there's a previous block, set its next pointer to the nxt block
	if (PREV_FREEP(bp)) 
		NEXT_FREEP(PREV_FREEP(bp)) = NEXT_FREEP(bp);
	else
		free_listp = NEXT_FREEP(bp); 
	//if non exists, set block prev_ptr to prev block
	PREV_FREEP(NEXT_FREEP(bp)) = PREV_FREEP(bp);
}
