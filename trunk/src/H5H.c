/*-------------------------------------------------------------------------
 * Copyright (C) 1997	National Center for Supercomputing Applications.
 *                      All rights reserved.
 *
 *-------------------------------------------------------------------------
 *
 * Created:		H5H.c
 * 			Jul 16 1997
 * 			Robb Matzke <robb@maya.nuance.com>
 *
 * Purpose:		Heap functions.
 *
 * Modifications:	
 *
 *-------------------------------------------------------------------------
 */
#include <assert.h>
#include "hdf5.h"

#include "H5ACprivate.h"		/*cache				*/
#include "H5Hprivate.h"			/*self				*/
#include "H5MFprivate.h"		/*file memory management	*/
#include "H5MMprivate.h"		/*core memory management	*/

#define H5H_MAGIC	"HEAP"		/*heap magic number		*/
#define H5H_FREE_NULL	1		/*end of free list on disk	*/
#define H5H_HDR_SIZE(F)	(4+H5F_SIZEOF_SIZE(F)+2*H5F_SIZEOF_OFFSET(F))
#define H5H_SIZEOF_FREE(F) (H5F_SIZEOF_SIZE(F)+H5F_SIZEOF_OFFSET(F))

typedef struct H5H_free_t {
   off_t	offset;			/*offset of free block		*/
   size_t	size;			/*size of free block		*/
   struct H5H_free_t *prev;		/*previous entry in free list	*/
   struct H5H_free_t *next;		/*next entry in free list	*/
} H5H_free_t;

typedef struct H5H_t {
   intn		dirty;
   haddr_t	addr;			/*address of data		*/
   size_t	disk_alloc;		/*data bytes allocated on disk	*/
   size_t	mem_alloc;		/*data bytes allocated in mem	*/
   uint8	*chunk;			/*the chunk, including header	*/
   H5H_free_t	*freelist;		/*the free list			*/
} H5H_t;

static H5H_t *H5H_load (hdf5_file_t *f, haddr_t addr, const void *udata);
static herr_t H5H_flush (hdf5_file_t *f, hbool_t dest, haddr_t addr,
			 H5H_t *heap);

/*
 * H5H inherits cache-like properties from H5AC
 */
static const H5AC_class_t H5AC_HEAP[1] = {{
   (void*(*)(hdf5_file_t*,haddr_t,const void*))H5H_load,
   (herr_t(*)(hdf5_file_t*,hbool_t,haddr_t,void*))H5H_flush,
}};


/*-------------------------------------------------------------------------
 * Function:	H5H_new
 *
 * Purpose:	Creates a new heap data structure on disk and caches it
 *		in memory.  SIZE_HINT is a hint for the initial size of the
 *		data area of the heap.  If size hint is invalid then a
 *		reasonable (but probably not optimal) size will be chosen.
 *		If the heap ever has to grow, then REALLOC_HINT is the
 *		minimum amount by which the heap will grow.
 *
 * Return:	Success:	File address of new heap.
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 16 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
haddr_t
H5H_new (hdf5_file_t *f, size_t size_hint)
{
   H5H_t	*heap = NULL;
   size_t	total_size;		/*total heap size on disk	*/
   haddr_t	addr;			/*heap file address		*/
   
   assert (f);
   size_hint = MAX (0, size_hint);
   if (size_hint && size_hint<H5H_SIZEOF_FREE(f)) {
      size_hint = H5H_SIZEOF_FREE(f);
   }

   /* allocate file version */
   total_size = H5H_HDR_SIZE(f) + size_hint;
   if ((addr = H5MF_alloc (f, total_size))<0) return -1;

   /* allocate memory version */
   heap = H5MM_xcalloc (1, sizeof(H5H_t));
   heap->addr = addr + H5H_HDR_SIZE(f);
   heap->disk_alloc = size_hint;
   heap->mem_alloc = size_hint;
   heap->chunk = H5MM_xmalloc (H5H_HDR_SIZE(f) + size_hint);

   /* free list */
   if (size_hint) {
      heap->freelist = H5MM_xmalloc (sizeof(H5H_free_t));
      heap->freelist->offset = 0;
      heap->freelist->size = size_hint;
      heap->freelist->prev = heap->freelist->next = NULL;
   } else {
      heap->freelist = NULL;
   }
   
   /* add to cache */
   heap->dirty = 1;
   H5AC_set (f, H5AC_HEAP, addr, heap);
   
   return addr;
}


/*-------------------------------------------------------------------------
 * Function:	H5H_load
 *
 * Purpose:	Loads a heap from disk.
 *
 * Return:	Success:	Ptr to heap memory data structure.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 17 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5H_t *
H5H_load (hdf5_file_t *f, haddr_t addr, const void *udata)
{
   uint8	hdr[20], *p;
   H5H_t	*heap = H5MM_xcalloc (1, sizeof(H5H_t));
   H5H_free_t	*fl=NULL, *tail=NULL;
   haddr_t	free_block;

   assert (addr>0);
   assert (H5H_HDR_SIZE(f) <= sizeof hdr);
   assert (!udata);

   if (H5F_block_read (f, addr, H5H_HDR_SIZE(f), hdr)<0) goto error;
   p = hdr;

   /* magic number */
   if (HDmemcmp (hdr, H5H_MAGIC, 4)) goto error;
   p += 4;

   /* heap data size */
   H5F_decode_length (f, p, heap->disk_alloc);
   heap->mem_alloc = heap->disk_alloc;

   /* free list head */
   H5F_decode_offset (f, p, free_block);
   assert (-1==free_block || (free_block>=0 && free_block<heap->disk_alloc));

   /* data */
   H5F_decode_offset (f, p, heap->addr);
   heap->chunk = H5MM_xmalloc (H5H_HDR_SIZE(f) + heap->mem_alloc);
   if (heap->disk_alloc &&
       H5F_block_read (f, heap->addr, heap->disk_alloc,
		       heap->chunk + H5H_HDR_SIZE(f))<0) {
      goto error;
   }

   /* free list */
   while (H5H_FREE_NULL!=free_block) {
      assert (free_block>=0 && free_block<heap->disk_alloc);
      fl = H5MM_xmalloc (sizeof (H5H_free_t));
      fl->offset = free_block;
      fl->prev = tail;
      fl->next = NULL;
      if (tail) tail->next = fl;
      tail = fl;
      if (!heap->freelist) heap->freelist = fl;
		   
      p = heap->chunk + H5H_HDR_SIZE(f) + free_block;
      H5F_decode_offset (f, p, free_block);
      H5F_decode_length (f, p, fl->size);

      assert (fl->offset + fl->size <= heap->disk_alloc);
   }
   return heap;

error:
   if (heap) {
      heap->chunk = H5MM_xfree (heap->chunk);
      H5MM_xfree (heap);
   }
   return NULL;
}


/*-------------------------------------------------------------------------
 * Function:	H5H_flush
 *
 * Purpose:	Flushes a heap from memory to disk if it's dirty.  Optionally
 *		deletes the heap from memory.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 17 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5H_flush (hdf5_file_t *f, hbool_t destroy, haddr_t addr, H5H_t *heap)
{
   uint8	*p = heap->chunk;
   H5H_free_t	*fl = heap->freelist;
   
   if (heap->dirty) {

      /*
       * If the heap grew larger than disk storage then move the
       * data segment of the heap to a larger contiguous block of
       * disk storage.
       */
      if (heap->mem_alloc > heap->disk_alloc) {
	 H5MF_free (f, heap->addr, heap->disk_alloc);
	 heap->addr = H5MF_alloc (f, heap->mem_alloc);
	 heap->disk_alloc = heap->mem_alloc;
      }

      /*
       * Write the header.
       */
      HDmemcpy (p, H5H_MAGIC, 4); p += 4;
      H5F_encode_length (f, p, heap->mem_alloc);
      H5F_encode_offset (f, p, fl?fl->offset:-1);
      H5F_encode_offset (f, p, heap->addr);

      /*
       * Write the free list.
       */
      while (fl) {
	 p = heap->chunk + H5H_HDR_SIZE(f) + fl->offset;
	 if (fl->next) {
	    H5F_encode_offset (f, p, fl->next->offset);
	 } else {
	    H5F_encode_offset (f, p, H5H_FREE_NULL);
	 }
	 H5F_encode_length (f, p, fl->size);
	 fl = fl->next;
      }

      /*
       * Copy buffer to disk.
       */
      if (heap->addr == addr + H5H_HDR_SIZE(f)) {
	 if (H5F_block_write (f, addr, H5H_HDR_SIZE(f)+heap->disk_alloc,
			      heap->chunk)<0) {
	    return -1;
	 }
      } else {
	 if (H5F_block_write (f, addr, H5H_HDR_SIZE(f), heap->chunk)<0) {
	    return -1;
	 }
	 if (H5F_block_write (f, heap->addr, heap->disk_alloc,
			      heap->chunk + H5H_HDR_SIZE(f))<0) {
	    return -1;
	 }
      }

      heap->dirty = 0;
   }

   /*
    * Should we destroy the memory version?
    */
   if (destroy) {
      heap->chunk = H5MM_xfree (heap->chunk);
      while (heap->freelist) {
	 fl = heap->freelist;
	 heap->freelist = fl->next;
	 H5MM_xfree (fl);
      }
      H5MM_xfree (heap);
   }

   return 0;
}
      

/*-------------------------------------------------------------------------
 * Function:	H5H_read
 *
 * Purpose:	Reads some object (or part of an object) from the heap
 *		whose address is ADDR in file F.  OFFSET is the byte offset
 *		from the beginning of the heap at which to begin reading
 *		and SIZE is the number of bytes to read.
 *
 * 		If BUF is the null pointer then a buffer is allocated by
 *		this function.
 *
 * 	       	Attempting to read past the end of an object may cause this
 *		function to fail.
 *
 * Return:	Success:	BUF (or the allocated buffer)
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 16 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void *
H5H_read (hdf5_file_t *f, haddr_t addr, off_t offset, size_t size, void *buf)
{
   H5H_t	*heap = H5AC_find (f, H5AC_HEAP, addr, NULL);

   assert (offset>=0 && offset<heap->mem_alloc);
   assert (offset+size<=heap->mem_alloc);

   if (!buf) buf = H5MM_xmalloc (size);
   HDmemcpy (buf, heap->chunk+H5H_HDR_SIZE(f)+offset, size);
   return buf;
}


/*-------------------------------------------------------------------------
 * Function:	H5H_peek
 *
 * Purpose:	This function is a more efficient version of H5H_read.
 *		Instead of copying a heap object into a caller-supplied
 *		buffer, this function returns a pointer directly into the
 *		cache where the heap is being held.  Thus, the return pointer
 *		is valid only until the next call to the cache.
 *
 * 		The address of the heap is ADDR in file F.  OFFSET is the
 *		byte offset of the object from the beginning of the heap and
 *		may include an offset into the interior of the object.
 *
 * Return:	Success:	Ptr to the object.  The pointer points to
 *				a chunk of memory large enough to hold the
 *				object from the specified offset (usually
 *				the beginning of the object) to the end
 *				of the object.  Do not attempt to read past
 *				the end of the object.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 16 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
const void *
H5H_peek (hdf5_file_t *f, haddr_t addr, off_t offset)
{
   H5H_t	*heap = H5AC_find (f, H5AC_HEAP, addr, NULL);

   assert (offset>=0 && offset<heap->mem_alloc);
   return heap->chunk+H5H_HDR_SIZE(f)+offset;
}


/*-------------------------------------------------------------------------
 * Function:	H5H_remove_free
 *
 * Purpose:	Removes free list element FL from the specified heap.
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 17 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
H5H_remove_free (H5H_t *heap, H5H_free_t *fl)
{
   if (fl->prev) fl->prev->next = fl->next;
   if (fl->next) fl->next->prev = fl->prev;

   if (!fl->prev) heap->freelist = fl->next;
}


/*-------------------------------------------------------------------------
 * Function:	H5H_insert
 *
 * Purpose:	Inserts a new item into the heap.
 *
 * Return:	Success:	Offset of new item within heap.
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 17 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
off_t
H5H_insert (hdf5_file_t *f, haddr_t addr, size_t size, const void *buf)
{
   H5H_t	*heap = H5AC_find (f, H5AC_HEAP, addr, NULL);
   H5H_free_t	*fl=NULL, *max_fl=NULL;
   off_t	offset = -1;
   size_t	need_more;

   heap->dirty += 1;

   /*
    * Look for a free slot large enough for this object and which would
    * leave zero or at least H5G_SIZEOF_FREE bytes left over.
    */
   for (fl=heap->freelist; fl; fl=fl->next) {
      if (fl->size>size && fl->size-size>=H5H_SIZEOF_FREE(f)) {
	 offset = fl->offset;
	 fl->offset += size;
	 fl->size -= size;
	 break;
      } else if (fl->size==size) {
	 offset = fl->offset;
	 H5H_remove_free (heap, fl);
	 break;
      } else if (!max_fl || max_fl->offset < fl->offset) {
	 max_fl = fl;
      }
   }


   /*
    * If no free chunk was large enough, then allocate more space and
    * add it to the free list.  If the heap ends with a free chunk, we
    * can extend that free chunk.  Otherwise we'll have to make another
    * free chunk.  If the heap must expand, we double its size.
    */
   if (offset<0) {

      need_more = MAX (2*heap->mem_alloc, H5H_SIZEOF_FREE(f));
      need_more = MAX (need_more, size);

      if (max_fl && max_fl->offset+max_fl->size==heap->mem_alloc) {
	 /*
	  * Increase the size of the maximum free block.
	  */
	 offset = max_fl->offset;
	 max_fl->offset += size;
	 max_fl->size += need_more - size;

	 if (max_fl->size < H5H_SIZEOF_FREE(f)) {
	    if (max_fl->size) {
	       fprintf (stderr, "H5H_insert: lost %d bytes at line %d\n",
			max_fl->size, __LINE__);
	    }
	    H5H_remove_free (heap, max_fl);
	 }
	 
      } else {
	 /*
	  * Create a new free list element large enough that we can
	  * take some space out of it right away.
	  */
	 offset = heap->mem_alloc;
	 if (need_more-size >= H5H_SIZEOF_FREE(f)) {
	    fl = H5MM_xmalloc (sizeof(H5H_free_t));
	    fl->offset = heap->mem_alloc + size;
	    fl->size = need_more - size;
	    fl->prev = NULL;
	    fl->next = heap->freelist;
	    if (heap->freelist) heap->freelist->prev = fl;
	    heap->freelist = fl;
	 } else if (need_more>size) {
	    fprintf (stderr, "H5H_insert: lost %d bytes at line %d\n",
		     need_more-size, __LINE__);
	 }
      }

      fprintf (stderr, "H5H_insert: resize mem buf from %lu to %lu bytes\n",
	       (unsigned long)(heap->mem_alloc),
	       (unsigned long)(heap->mem_alloc + need_more));
      heap->mem_alloc += need_more;
      heap->chunk = H5MM_xrealloc (heap->chunk,
				   H5H_HDR_SIZE(f)+heap->mem_alloc);
   }

   /*
    * Copy the data into the heap
    */
   HDmemcpy (heap->chunk + H5H_HDR_SIZE(f) + offset, buf, size);
   return offset;
}
   

/*-------------------------------------------------------------------------
 * Function:	H5H_write
 *
 * Purpose:	Writes (overwrites) the object (or part of object) stored
 *		in BUF to the heap at file address ADDR in file F.  The
 *		writing begins at byte offset OFFSET from the beginning of
 *		the heap and continues for SIZE bytes.
 *
 * 		Do not partially write an object to create it;  the first
 *		write for an object must be for the entire object.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 16 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5H_write (hdf5_file_t *f, haddr_t addr, off_t offset, size_t size,
	   const void *buf)
{
   H5H_t	*heap = H5AC_find (f, H5AC_HEAP, addr, NULL);

   assert (offset>=0 && offset<heap->mem_alloc);
   assert (offset+size<=heap->mem_alloc);
   assert (buf);

   heap->dirty += 1;
   HDmemcpy (heap->chunk+H5H_HDR_SIZE(f)+offset, buf, size);
   return 0;
}


/*-------------------------------------------------------------------------
 * Function:	H5H_remove
 *
 * Purpose:	Removes an object or part of an object from the heap at
 *		address ADDR of file F.  The object (or part) to remove
 *		begins at byte OFFSET from the beginning of the heap and
 *		continues for SIZE bytes.
 *
 * 		Once part of an object is removed, one must not attempt
 *		to access that part.  Removing the beginning of an object
 *		results in the object OFFSET increasing by the amount
 *		truncated.  Removing the end of an object results in
 *		object truncation.  Removing the middle of an object results
 *		in two separate objects, one at the original offset and
 *		one at the first offset past the removed portion.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Robb Matzke
 *		robb@maya.nuance.com
 *		Jul 16 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5H_remove (hdf5_file_t *f, haddr_t addr, off_t offset, size_t size)
{
   H5H_t	*heap = H5AC_find (f, H5AC_HEAP, addr, NULL);
   H5H_free_t	*fl = heap->freelist, *fl2 = NULL;

   assert (offset>=0 && offset<heap->mem_alloc);
   assert (offset+size<=heap->mem_alloc);

   heap->dirty += 1;

   /*
    * Check if this chunk can be prepended or appended to an already
    * free chunk.  It might also fall between two chunks in such a way
    * that all three chunks can be combined into one.
    */
   while (fl) {
      if (offset + size == fl->offset) {
	 fl->offset = offset;
	 fl->size += size;
	 fl2 = fl->next;
	 while (fl2) {
	    if (fl2->offset + fl2->size == fl->offset) {
	       fl->offset = fl2->offset;
	       fl->size += fl2->size;
	       H5H_remove_free (heap, fl2);
	       return 0;
	    }
	 }
	 return 0;
	 
      } else if (fl->offset + fl->size == offset) {
	 fl->size += size;
	 fl2 = fl->next;
	 while (fl2) {
	    if (fl->offset + fl->size == fl2->offset) {
	       fl->size += fl2->size;
	       H5H_remove_free (heap, fl2);
	       return 0;
	    }
	 }
	 return 0;
      }

      fl = fl->next;
   }


   /*
    * The amount which is being removed must be large enough to
    * hold the free list data.  If not, the freed chunk is forever
    * lost.
    */
   if (size < H5H_SIZEOF_FREE(f)) {
      fprintf (stderr, "H5H_remove: lost %d bytes\n", size);
      return 0;
   }

   /*
    * Add an entry to the free list.
    */
   fl = H5MM_xmalloc (sizeof(H5H_free_t));
   fl->offset = offset;
   fl->size = size;
   fl->prev = NULL;
   fl->next = heap->freelist;
   if (heap->freelist) heap->freelist->prev = fl;
   heap->freelist = fl;

   return 0;
}
