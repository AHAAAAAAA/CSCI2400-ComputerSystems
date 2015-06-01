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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
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


/////////////////////////////////////////////////////////////////////////////
// Constants and macros
//
// These correspond to the material in Figure 9.43 of the text
// The macros have been turned into C++ inline functions to
// make debugging code easier.
//
/////////////////////////////////////////////////////////////////////////////


#define WSIZE       4       /* word size (bytes) */  
#define DSIZE       8       /* doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)  /* initial heap size (bytes) */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */
#define MANANA      1
/*
static inline int MAX(int x, int y) {
  return x > y ? x : y;
}

//
// Pack a size and allocated bit into a word
// We mask of the "alloc" field to insure only
// the lower bit is used
//
static inline size_t PACK(size_t size, int alloc) {
  return ((size) | (alloc & 0x1));
}

//
// Read and write a word at address p
//
static inline size_t GET(void *p) { return  *(size_t *)p; }
static inline void PUT( void *p, size_t val)
{
  *((size_t *)p) = val;
}

//
// Read the size and allocated fields from address p
//
static inline size_t GET_SIZE( void *p )  { 
  return GET(p) & ~0x7;
}

static inline int GET_ALLOC( void *p  ) {
  return GET(p) & 0x1;
}

//
// Given block ptr bp, compute address of its header and footer
//
static inline void *HDRP(void *bp) {

  return ( (char *)bp) - WSIZE;
}
static inline void *FTRP(void *bp) {
  return ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE);
}

//
// Given block ptr bp, compute address of next and previous blocks
//
static inline void *NEXT_BLKP(void *bp) {
  return  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)));
}

static inline void* PREV_BLKP(void *bp){
  return  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)));
}


/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)

/* Basic constants and macros */
#define MINIMUM    24  /* minimum block size */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(int *)(p))
#define PUT(p, val)  (*(int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((void *)(bp) - WSIZE)
#define FTRP(bp)       ((void *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((void *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)  ((void *)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

/* Given block ptr bp, compute address of next and previous free blocks */
#define NEXT_FREEP(bp)(*(void **)(bp + DSIZE))
#define PREV_FREEP(bp)(*(void **)(bp))

static char *heap_listp = 0; /* Pointer to the first block */
static char *free_listp = 0;/* Pointer to the first free block */
/////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//

static char *heap_listp;  /* pointer to first block */  
static char *free_listp; // pointer to first free block

//
// function prototypes for internal helper routines
//
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp); 
static void checkblock(void *bp);
static void insertAtFront(void *bp); //linked list insert function
static void removeBlock(void *bp); //Linked list delete/remove

//
// mm_init - Initialize the memory manager 
//
int mm_init(void) 
{
//create initial empty heap 
  if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
	    return -1;


  PUT (heap_listp,0);//alignment heading
  //book solution 
  // PUT (heap_listp + (1*WSIZE), PACK(DSIZE, 1));//Prologue header
  // PUT (heap_listp + (2*WSIZE), PACK(DSIZE, 1));//prologue footer
  // PUT (heap_listp + (3*WSIZE), PACK(0,1));//Epilogue header
  // heap_listp += (2*WSIZE);
  //Itganesan solution
  PUT(heap_listp + WSIZE, PACK((DSIZE + OVERHEAD), 1)); //WSIZE is padding
  PUT(heap_listp + DSIZE, 0); //for PREVIOUS pointer
  PUT(heap_listp + DSIZE + WSIZE, 0); //for NEXT pointer
  PUT(heap_listp + (DSIZE + OVERHEAD), PACK((DSIZE + OVERHEAD), 1)); //dummy block for footer
  PUT(heap_listp + WSIZE + (DSIZE + OVERHEAD), PACK(0,1)); //dummy tail block
  free_listp = heap_listp + DSIZE; //free list pointer points at tail block

//Extend heap with CHUNKSIZE bytes
  if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
	     return -1;

  return 0;
}


//
// extend_heap - Extend heap with free block and return its block pointer
//
static void *extend_heap(size_t words) 
{
  char *bp;
  size_t size;
//Allocate even number words for alignment purposes
  size = (words %2) ? (words+1) * WSIZE : words * WSIZE;
  //Book Solution
  // if ((long)(bp = mem_sbrk(size))== -1)
	 //   return NULL;
  //Itganesan Solution 
  if(size<(DSIZE + OVERHEAD)){
  	size = (DSIZE + OVERHEAD);
  }//check to maintain minimum size
  if((long)(bp = mem_sbrk(size)) == -1){
  	return NULL;
  }//check for positive size
//Initialize free block header/footer and epilogue header
  PUT (HDRP(bp), PACK(size,0));//free block header
  PUT (FTRP(bp), PACK(size,0));//free block footer
  PUT (HDRP(NEXT_BLKP(bp)), PACK (0,1));//New epilogue header

  return coalesce(bp);//coalesce if previous block is free
}


//
// Practice problem 9.8
//
// find_fit - Find a fit for a block with asize bytes 
//
static void *find_fit(size_t asize)
{
//Search through blocks 
  void *bp;

  for(bp = free_listp; GET_ALLOC(HDRP(bp)) == 0; bp = NEXT_FREEP(bp)){
  	//iterate through free list
  	if(asize <= (size_t)GET_SIZE(HDRP(bp))){
  		//Find free block large enough to hold desired size
  		return bp;
  	}
  }
  //Book Solution
 //  bp = heap_listp;

 //  for ( ; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
	// if ( !(GET_ALLOC(HDRP(bp))) && ( asize <= GET_SIZE(HDRP(bp))))
	// 	return bp;
 //  }
  return NULL; //no fit found
}

// 
// mm_free - Free a block 
//
void mm_free(void *bp){
  if(!bp)
  	return;
  //return if pointer is NULL
  size_t size = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

//
// coalesce - boundary tag coalescing. Return ptr to coalesced block
//
static void *coalesce(void *bp) 
{
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));
//3 available options: 
//1. Previous and next block are not allocated
//Extend block in both directions
  if(!prev_alloc && !next_alloc){
  	size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
  	removeBlock(PREV_BLKP(bp));
  	removeBlock(NEXT_BLKP(bp));
  	bp = PREV_BLKP(bp);
  	PUT(HDRP(bp), PACK(size, 0));
  	PUT(FTRP(bp), PACK(size, 0));
  }

  else if (prev_alloc && !next_alloc){//2. Previous block is allocated, next block is free
//Current block merged with next block,header of current block/footer of next block updated with combined sizes of current/next blocks
	size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
	removeBlock(NEXT_BLKP(bp));
	PUT(HDRP(bp), PACK(size,0));
	PUT(FTRP(bp), PACK(size,0));
	}

  else if (!prev_alloc && next_alloc){//3. Previous block is annotated next block is not
//Previous block merged with current block, head of prev/ foot of current block are updated with size of two blocks
	size += GET_SIZE(HDRP(PREV_BLKP(bp)));
	removeBlock(bp);
	PUT(FTRP(bp), PACK(size,0));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
	bp = PREV_BLKP(bp);
	}

  else{//4. Previous and next block are free
//All 3 blocks are merged to form a single free block, with the header of prev bllock and footer of next block updated with combined size
  size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
	PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
	PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
	bp = PREV_BLKP(bp);
  }
  insertAtFront(bp);
  return bp;
}

//
// mm_malloc - Allocate a block with at least size bytes of payload 
//
void *mm_malloc(size_t size) 
{
  size_t asize;
  size_t extendsize;
  char *bp;

  if(size<=0)
  	return NULL;
  //Itganesan Solution

  asize = MAX(ALIGN(size) + DSIZE, (DSIZE + OVERHEAD)); //change size to account for overhead/alignment

  if((bp = find_fit(asize))){
  	place(bp, asize);
  	return bp;
  }
 //Bok Solution 
// //adjusts size to make up for any overflow or specific alignment guidelines
//   if (size <= DSIZE)
// 	   asize = 2*DSIZE;
//   else
// 	   asize = DSIZE * (( size + (DSIZE) + (DSIZE-1))/ DSIZE);
// //search available lists to find space
//   if ((bp = find_fit(asize)) != NULL){
// 	   place(bp,asize);
// 	   return bp;
//   }
//no fit = get more memory then place in there
  extendsize = MAX(asize,CHUNKSIZE);
  if((bp = extend_heap(extendsize/WSIZE)) == NULL)
	   return NULL;
  place(bp, asize);
  return bp;
} 


//
//
// Practice problem 9.9
//
// place - Place block of asize bytes at start of free block bp 
//         and split if remainder would be at least minimum block size
//
static void place(void *bp, size_t asize)
{
  size_t csize = GET_SIZE(HDRP(bp)); 
  if((csize-asize) >= (DSIZE + OVERHEAD)){
//If block remainder after splitting would be greater than or equal to min blcok size, then split block
//place blocks 
	PUT(HDRP(bp), PACK(asize,1));
	PUT(FTRP(bp), PACK(asize,1));
	removeBlock(bp);
//Move to next block
	bp = NEXT_BLKP(bp);
	PUT(HDRP(bp), PACK(csize-asize,0));
	PUT(FTRP(bp), PACK(csize-asize,0));
	coalesce(bp);
  }
  else{
//if not place blocks
	PUT(HDRP(bp), PACK(csize, 1));
	PUT(FTRP(bp), PACK(csize, 1));
	removeBlock(bp);
  }
}


//
// mm_realloc -- implemented for you
//
void *mm_realloc(void *ptr, size_t size)
{
	size_t prevSize;
	void *newptr;
	size_t asize = MAX(ALIGN(size) + DSIZE, (DSIZE + OVERHEAD));
	if(size <= 0){
		//If true, pointer is free and return NULL
		free(ptr);
		return NULL;
	}
	if(ptr == NULL){
		//If true, just allocate the memory
		return mm_malloc(size);
	}
	prevSize = GET_SIZE(HDRP(ptr)); //gets size of og block

	if(asize == prevSize){
		//if size requires no change, return og ptr
		return ptr;
	}

	if(asize <= prevSize){
		//If true, size needs to be decreased by shrinking block & returning same ptr
		size = asize;
		if(prevSize - size <= (DSIZE + OVERHEAD)){
			//new block cant fit in remaining space, return ptr
			return ptr;
		}
		PUT(HDRP(ptr), PACK(size, 1));
		PUT(FTRP(ptr), PACK(size, 1));
		PUT(HDRP(NEXT_BLKP(ptr)), PACK(prevSize - size, 1));
		free(NEXT_BLKP(ptr));
		return ptr;
	}
	newptr = mm_malloc(size);

	if(!newptr){
		//if realloc fails => og block is untouched
		return 0;
	}

	if(size< prevSize)
		//copy previous data
		prevSize = size;
		memcpy(newptr, ptr, prevSize);
		//free block
		free(ptr);
		return newptr;
   // void *newp;
   // size_t copySize;

   // newp = mm_malloc(size);
   // if (newp == NULL) {
   //   printf("ERROR: mm_malloc failed in mm_realloc\n");
   //   exit(1);
   // }
   // copySize = GET_SIZE(HDRP(ptr));
   // if (size < copySize) {
   //   copySize = size;
   // }
   // memcpy(newp, ptr, copySize);
   // mm_free(ptr);
   // return newp;
 }

//
// mm_checkheap - Check the heap for consistency 
//
void mm_checkheap(int verbose) 
{
  //
  // This provided implementation assumes you're using the structure
  // of the sample solution in the text. If not, omit this code
  // and provide your own mm_checkheap
  //
  void *bp = heap_listp;
  
  if (verbose) {
    printf("Heap (%p):\n", heap_listp);
  }

  if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp))) {
	printf("Bad prologue header\n");
  }
  checkblock(heap_listp);

  for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    if (verbose)  {
      printblock(bp);
    }
    checkblock(bp);
  }
     
  if (verbose) {
    printblock(bp);
  }

  if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
    printf("Bad epilogue header\n");
  }
}

static void printblock(void *bp) 
{
  size_t hsize, halloc, fsize, falloc;

  hsize = GET_SIZE(HDRP(bp));
  halloc = GET_ALLOC(HDRP(bp));  
  fsize = GET_SIZE(FTRP(bp));
  falloc = GET_ALLOC(FTRP(bp));  
    
  if (hsize == 0) {
    printf("%p: EOL\n", bp);
    return;
  }

  printf("%p: header: [%d:%c] footer: [%d:%c]\n",
	 bp, 
	 (int) hsize, (halloc ? 'a' : 'f'), 
	 (int) fsize, (falloc ? 'a' : 'f')); 
}
//
static void checkblock(void *bp) 
{
  if ((size_t)bp % 8) {
    printf("Error: %p is not doubleword aligned\n", bp);
  }
  if (GET(HDRP(bp)) != GET(FTRP(bp))) {
    printf("Error: header does not match footer\n");
  }
  //Check next/prev pointers to be within bounds
  if(NEXT_FREEP(bp) < mem_heap_lo() || NEXT_FREEP(bp) > mem_heap_hi()){
  	printf("ERROR: next pointer %p is not within heap bounds \n", NEXT_FREEP(bp));
  }
  if(PREV_FREEP(bp) < mem_heap_lo() || PREV_FREEP(bp) > mem_heap_hi()){
  	printf("ERROR: prev pointer %p is not within heap bounds \n", PREV_FREEP(bp));
  }
}
//insertAtFront:
//	Inserts a block at the front of the free list.
//  This function takes a block pointer of the block to add to the free list as a parameter.
static void insertAtFront(void *bp){
	NEXT_FREEP(bp) = free_listp; //set next_ptr to start of free list
	PREV_FREEP(free_listp) = bp; //set current previous pointer to new block
	PREV_FREEP(bp) = NULL; //set prev_ptr to NULL
	free_listp = bp; //set free list as new block
}
//removeBlock:
	// removeBlock - Removes a block from the free list
	// This function takes a block pointer of the block to remove as a parameter.
static void removeBlock(void *bp){
	//check for previous block, if one exists set nxt ptr to nxt block
	//if none exists set block prev_ptr to prev block
	if(PREV_FREEP(bp)){
		NEXT_FREEP(PREV_FREEP(bp)) = NEXT_FREEP(bp);
	}
	else {
		free_listp = NEXT_FREEP(bp);
	}
	PREV_FREEP(NEXT_FREEP(bp)) = PREV_FREEP(bp);
}