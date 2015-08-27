/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:     H5Cimage.c
 *              July 20, 2015
 *              John Mainzer
 *
 * Purpose:     Functions in this file are specific to the implementation
 *		of the metadata cache image feature.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#define H5C_PACKAGE		/*suppress error about including H5Cpkg   */
#define H5F_PACKAGE		/*suppress error about including H5Fpkg	  */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#ifdef H5_HAVE_PARALLEL
#include "H5ACprivate.h"        /* Metadata cache                       */
#endif /* H5_HAVE_PARALLEL */
#include "H5Cpkg.h"		/* Cache				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fpkg.h"		/* Files				*/
#include "H5FDprivate.h"	/* File drivers				*/
#include "H5FLprivate.h"	/* Free Lists                           */
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5MFprivate.h"	/* File memory management		*/
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5Pprivate.h"         /* Property lists                       */
#include "H5SLprivate.h"	/* Skip lists				*/


/****************/
/* Local Macros */
/****************/
#if H5C_DO_MEMORY_SANITY_CHECKS
#define H5C_IMAGE_EXTRA_SPACE 8
#define H5C_IMAGE_SANITY_VALUE "DeadBeef"
#else /* H5C_DO_MEMORY_SANITY_CHECKS */
#define H5C_IMAGE_EXTRA_SPACE 0
#endif /* H5C_DO_MEMORY_SANITY_CHECKS */

#define H5C__MDCI_BLOCK_SIGNATURE	"MDCI"
#define H5C__MDCI_BLOCK_SIGNATURE_LEN	4
#define H5C__MDCI_BLOCK_VERSION_0	0
#define H5C__MDCI_ENTRY_SIGNATURE	"MDEI"
#define H5C__MDCI_ENTRY_SIGNATURE_LEN	4

/* metadata cache image entry flags -- max 8 bits */
#define H5C__MDCI_ENTRY_DIRTY_FLAG		0x01
#define H5C__MDCI_ENTRY_IN_LRU_FLAG		0x02
#define H5C__MDCI_ENTRY_IS_FD_PARENT_FLAG	0x04
#define H5C__MDCI_ENTRY_IS_FD_CHILD_FLAG	0x08

#define H5C__MDCI_MAX_FD_CHILDREN		USHRT_MAX

/******************/
/* Local Typedefs */
/******************/


/********************/
/* Local Prototypes */
/********************/

static herr_t H5C__prefetched_entry_get_load_size(const void *udata_ptr,
    size_t *image_len_ptr);

static void * H5C__prefetched_entry_deserialize(const void * image_ptr,
    size_t len, void * udata, hbool_t * dirty_ptr);

static herr_t H5C__prefetched_entry_image_len(const void * thing,
    size_t *image_len_ptr, hbool_t *compressed_ptr,
    size_t *compressed_len_ptr);

static herr_t H5C__prefetched_entry_pre_serialize(const H5F_t *f,
    hid_t dxpl_id, void * thing, haddr_t addr, size_t len,
    size_t compressed_len, haddr_t * new_addr_ptr, size_t * new_len_ptr,
    size_t * new_compressed_len_ptr, unsigned * flags_ptr);

static herr_t H5C__prefetched_entry_serialize(const H5F_t *f, void * image_ptr,
    size_t len, void * thing);

static herr_t H5C__prefetched_entry_notify(H5C_notify_action_t action, 
    void *thing);

static herr_t H5C__prefetched_entry_free_icr(void * thing);

static herr_t H5C__prefetched_entry_fsf_size(const void H5_ATTR_UNUSED * thing,
    size_t H5_ATTR_UNUSED * fsf_size_ptr);

static size_t H5C_cache_image_block_entry_header_size(H5F_t * f);

static size_t H5C_cache_image_block_header_size(void);

static herr_t H5C_decode_cache_image_buffer(H5F_t * f, H5C_t * cache_ptr);

static const uint8_t * H5C_decode_cache_image_header(H5C_t *cache_ptr, 
    const uint8_t *buf);

static const uint8_t *
H5C_decode_cache_image_entry(H5F_t * f, H5C_t * cache_ptr, const uint8_t * buf,
    int entry_num, size_t expected_entry_header_len);

static herr_t H5C_destroy_pf_entry_child_flush_deps(H5C_t *cache_ptr, 
    H5C_cache_entry_t * pf_entry_ptr, H5C_cache_entry_t ** fd_children);

static uint8_t * H5C_encode_cache_image_header(H5C_t *cache_ptr, uint8_t *buf);

static uint8_t * H5C_encode_cache_image_entry(H5F_t * f, H5C_t * cache_ptr, 
    uint8_t * buf, int entry_num, size_t expected_entry_header_len);

static void H5C_prep_for_file_close__partition_image_entries_array(
    H5C_t * cache_ptr, int bottom, int * middle_ptr, int top);

static void H5C_prep_for_file_close__qsort_image_entries_array(
    H5C_t * cache_ptr, int bottom, int top);

static herr_t H5C_prep_for_file_close__setup_image_entries_array(
    H5C_t * cache_ptr);

static herr_t H5C_prep_for_file_close__scan_entries(H5F_t *f, hid_t dxpl_id,
    H5C_t * cache_ptr);

static herr_t H5C_reconstruct_cache_contents(H5F_t * f, hid_t dxpl_id, 
    H5C_t * cache_ptr);

static H5C_cache_entry_t * H5C_reconstruct_cache_entry(H5C_t * cache_ptr, 
    int i);

static herr_t H5C_serialize_cache(const H5F_t * f, hid_t dxpl_id);

static herr_t H5C_serialize_single_entry(const H5F_t *f, hid_t dxpl_id, 
    H5C_t *cache_ptr, H5C_cache_entry_t *entry_ptr, hbool_t *restart_scan_ptr,
    hbool_t *restart_bucket_ptr);

static herr_t H5C__write_cache_image_superblock_msg(H5F_t *f, hid_t dxpl_id, 
    hbool_t create);

/*********************/
/* Package Variables */
/*********************/

extern const H5FD_mem_t class_mem_types[H5C__MAX_NUM_TYPE_IDS + 1];

const H5C_class_t prefetched_entry_class =
{
    /* id               = */ H5AC_PREFETCHED_ENTRY_ID,
    /* name             = */ "prefetched entry",
    /* mem_type         = */ H5FD_MEM_DEFAULT, /* value doesn't matter */
    /* flags            = */ H5AC__CLASS_NO_FLAGS_SET,
    /* get_load_size    = */ H5C__prefetched_entry_get_load_size,
    /* deserialize      = */ H5C__prefetched_entry_deserialize,
    /* image_len        = */ H5C__prefetched_entry_image_len,
    /* pre_serialize    = */ H5C__prefetched_entry_pre_serialize,
    /* serialize        = */ H5C__prefetched_entry_serialize,
    /* notify           = */ H5C__prefetched_entry_notify,
    /* free_icr         = */ H5C__prefetched_entry_free_icr,
    /* clear            = */ NULL,
    /* fsf_size         = */ H5C__prefetched_entry_fsf_size,
};


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/


/*-------------------------------------------------------------------------
 * Function:    H5C_construct_cache_image_buffer()
 *
 *		Allocate a buffer of size cache_ptr->image_len, and 
 *		load it with an image of the metadata cache image block.
 *
 *		Note that by the time this function is called, the cache
 *		should have removed all entries from its data structures.
 *
 * Return:      SUCCEED on success, and FAIL on failure.
 *
 * Programmer:  John Mainzer
 *              8/5/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_construct_cache_image_buffer(H5F_t * f, H5C_t * cache_ptr)
{
    int		i;
    uint32_t    chksum;
    size_t 	entry_header_size;
    uint8_t *	p;
    herr_t 	ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert(f);
    HDassert(f->shared);
    HDassert(cache_ptr == f->shared->cache);
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->image_ctl.generate_image);
    HDassert(cache_ptr->num_entries_in_image > 0);
    HDassert(cache_ptr->index_len == 0);

    /* allocate the buffer in which to construct the cache image block */

    cache_ptr->image_buffer = H5MM_malloc(cache_ptr->image_len + 1);

    if ( NULL == cache_ptr->image_buffer )

	HGOTO_ERROR(H5E_CACHE, H5E_CANTALLOC, FAIL, \
		    "memory allocation failed for cache image buffer");

    p = (uint8_t *)cache_ptr->image_buffer;


    /* construct the cache image block header image */

    if ( NULL == (p = H5C_encode_cache_image_header(cache_ptr, p)) )

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
		    "header image construction failed.")

    HDassert((size_t)(p - (uint8_t *)cache_ptr->image_buffer) < 
             cache_ptr->image_len);


    /* construct the cache entry images */

    entry_header_size = H5C_cache_image_block_entry_header_size(f);

    for ( i = 0; i < cache_ptr->num_entries_in_image; i++ ) {

	p = H5C_encode_cache_image_entry(f, cache_ptr, p, i, entry_header_size);

	if ( NULL ==  p )

            HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
		        "entry image construction failed.")
    }

    HDassert((size_t)(p - (uint8_t *)cache_ptr->image_buffer) < 
             cache_ptr->image_len);


    /* construct the adaptive resize status image -- not yet */


    /* compute the checksum and encode */
    chksum = H5_checksum_metadata(cache_ptr->image_buffer, 
                         (size_t)(cache_ptr->image_len - H5F_SIZEOF_CHKSUM), 0);

    UINT32ENCODE(p, chksum);

    HDassert((size_t)(p - (uint8_t *)cache_ptr->image_buffer) == 
             cache_ptr->image_len);

#ifndef NDEBUG
    /* validate the metadata cache image we just constructed by decoding it
     * and comparing the result with the original data.
     */
    {
        uint32_t        old_chksum;
        const uint8_t *	q;
        H5C_t *	        fake_cache_ptr = NULL;

	fake_cache_ptr = (H5C_t *)H5MM_malloc(sizeof(H5C_t));

        HDassert(fake_cache_ptr);

        fake_cache_ptr->magic = H5C__H5C_T_MAGIC;

        q = (const uint8_t *)cache_ptr->image_buffer;

        q = H5C_decode_cache_image_header(fake_cache_ptr, q);

        HDassert(NULL != p);

        HDassert(fake_cache_ptr->num_entries_in_image == \
                 cache_ptr->num_entries_in_image);

	fake_cache_ptr->image_entries = (H5C_image_entry_t *)
        	H5MM_malloc(sizeof(H5C_image_entry_t) *
                            (size_t)(fake_cache_ptr->num_entries_in_image + 1));

	HDassert(fake_cache_ptr->image_entries);

        for ( i = 0; i < fake_cache_ptr->num_entries_in_image; i++ ) {

	    (fake_cache_ptr->image_entries)[i].magic = 
					H5C__H5C_IMAGE_ENTRY_T_MAGIC;
            (fake_cache_ptr->image_entries)[i].image_index = i;
            (fake_cache_ptr->image_entries)[i].image_ptr = NULL;

	    /* touch up f->shared->cache to satisfy sanity checks... */
            f->shared->cache = fake_cache_ptr;

	    q = H5C_decode_cache_image_entry(f, fake_cache_ptr, q, i, 
                                             entry_header_size);

	    /* ...and then return f->shared->cache to its correct value */
            f->shared->cache = cache_ptr;

	    HDassert(q);

	    /* verify expected contents */
	    HDassert((cache_ptr->image_entries)[i].addr == 
                     (fake_cache_ptr->image_entries)[i].addr);
	    HDassert((cache_ptr->image_entries)[i].size == 
                     (fake_cache_ptr->image_entries)[i].size);
	    HDassert((cache_ptr->image_entries)[i].type_id == 
                     (fake_cache_ptr->image_entries)[i].type_id);
	    HDassert((cache_ptr->image_entries)[i].lru_rank == 
                     (fake_cache_ptr->image_entries)[i].lru_rank);
	    HDassert((cache_ptr->image_entries)[i].is_dirty == 
                     (fake_cache_ptr->image_entries)[i].is_dirty);
	    /* don't check flush_dep_height as it is not stored in 
             * the metadata cache image block.
             */
	    HDassert((cache_ptr->image_entries)[i].fd_parent_addr == 
                     (fake_cache_ptr->image_entries)[i].fd_parent_addr);
	    HDassert((cache_ptr->image_entries)[i].fd_child_count == 
                     (fake_cache_ptr->image_entries)[i].fd_child_count);
	    HDassert((cache_ptr->image_entries)[i].image_ptr);
            HDassert((fake_cache_ptr->image_entries)[i].image_ptr);
            HDassert(!HDmemcmp((cache_ptr->image_entries)[i].image_ptr,
                               (fake_cache_ptr->image_entries)[i].image_ptr,
                               (cache_ptr->image_entries)[i].size));

	    (fake_cache_ptr->image_entries)[i].image_ptr = 
		H5MM_xfree((fake_cache_ptr->image_entries)[i].image_ptr);
	}

        HDassert((size_t)(q - (const uint8_t *)cache_ptr->image_buffer) == 
                 cache_ptr->image_len - H5F_SIZEOF_CHKSUM);

        /* compute the checksum  */
        old_chksum = chksum;
        chksum = H5_checksum_metadata(cache_ptr->image_buffer, 
                         (size_t)(cache_ptr->image_len - H5F_SIZEOF_CHKSUM), 0);

	HDassert(chksum == old_chksum);

	fake_cache_ptr = (H5C_t *)H5MM_xfree((void *)fake_cache_ptr);
    }
#endif /* NDEBUG */

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_construct_cache_image_buffer() */


/*-------------------------------------------------------------------------
 *
 * Function:    H5C_deserialize_prefetched_entry()
 *
 * Purpose:     Deserialize the supplied prefetched entry entry, and return
 *		a pointer to the deserialized entry in *entry_ptr_ptr. 
 *		If successful, remove the prefetched entry from the cache,
 *		and free it.  Insert the deserialized entry into the cache.
 *
 *		Note that the on disk image of the entry is not freed -- 
 *		a pointer to it is stored in the deserialized entries'
 *		image_ptr field, and its image_up_to_date field is set to
 *		TRUE unless the entry is dirtied by the deserialize call.
 *
 *		If the prefetched entry is a flush dependency child,
 *		destroy that flush dependency prior to calling the 
 *		deserialize callback.  If appropriate, the flush dependency
 *		relationship will be recreated by the cache client.
 *
 *		If the prefetched entry is a flush dependency parent,
 *		destroy the flush dependency relationship with all its 
 *		children.  As all these children must be prefetched entries,
 *		recreate these flush dependency relationships with 
 *		deserialized entry after it is inserted in the cache.
 *
 *		Since deserializing a prefetched entry is semantically 
 *		equivalent to a load, issue an entry loaded nofification 
 *		if the notify callback is defined.
 *
 * Return:      SUCCEED on success, and FAIL on failure.
 *
 *		Note that *entry_ptr_ptr is undefined on failure.
 *
 * Programmer:  John Mainzer, 8/10/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_deserialize_prefetched_entry(H5F_t *             f,
                                 hid_t               dxpl_id,
                                 H5C_t *	     cache_ptr,
				 H5C_cache_entry_t** entry_ptr_ptr,
                                 const H5C_class_t * type,
                                 haddr_t             addr,
                                 void *              udata)
{
    int			i;
    hbool_t		dirty = FALSE;  /* Flag indicating whether thing was 
                                         * dirtied during deserialize 
                                         */
    hbool_t		compressed = FALSE; /* flag indicating whether thing */
 					/* will be run through filters on    */
                                        /* on read and write.  Usually FALSE */
					/* set to true if appropriate.       */
    size_t		compressed_size = 0; /* entry compressed size if     */
                                        /* known -- otherwise uncompressed.  */
				        /* Zero indicates compression not    */
                                        /* enabled.                          */
    size_t              len;            /* Size of image in file */
    void *		thing = NULL;   /* Pointer to thing loaded */
    H5C_cache_entry_t * pf_entry_ptr;   /* pointer to the prefetched entry   */
                                        /* supplied in *entry_ptr_ptr.       */
    H5C_cache_entry_t *	ds_entry_ptr;   /* Alias for thing loaded, as cache 
                                         * entry 
                                         */
    H5C_cache_entry_t** fd_children = NULL; /* Pointer to a dynamically      */
                                        /* allocated array of pointers to    */
                                        /* the flush dependency children of  */
                                        /* the prefetched entry, or NULL if  */
                                        /* that array does not exist.        */
    unsigned            flush_flags = (H5C__FLUSH_INVALIDATE_FLAG | 
				       H5C__FLUSH_CLEAR_ONLY_FLAG);
    unsigned            u;              /* Local index variable */
    herr_t      	ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* sanity checks */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->cache);
    HDassert(f->shared->cache == cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(entry_ptr_ptr);
    HDassert(*entry_ptr_ptr);
    pf_entry_ptr = *entry_ptr_ptr;
    HDassert(pf_entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
    HDassert(pf_entry_ptr->type);
    HDassert(pf_entry_ptr->type->id == H5AC_PREFETCHED_ENTRY_ID);
    HDassert(pf_entry_ptr->prefetched);
    HDassert(pf_entry_ptr->image_up_to_date);
    HDassert(pf_entry_ptr->image_ptr);
    HDassert(pf_entry_ptr->size > 0);
    HDassert(pf_entry_ptr->addr == addr);
    HDassert(type);
    HDassert(type->id == pf_entry_ptr->prefetch_type_id);
    HDassert(type->mem_type == class_mem_types[type->id]);

    /* verify absence of prohibited or unsupported type flag combinations */
    HDassert(!(type->flags & H5C__CLASS_NO_IO_FLAG));
 
    /* for now, we do not combine the speculative load and compressed flags */
    HDassert(!((type->flags & H5C__CLASS_SPECULATIVE_LOAD_FLAG) &&
               (type->flags & H5C__CLASS_COMPRESSED_FLAG)));

    /* Can't see how skip reads could be usefully combined with 
     * either the speculative read or compressed flags.  Hence disallow.
     */
    HDassert(!((type->flags & H5C__CLASS_SKIP_READS) &&
               (type->flags & H5C__CLASS_SPECULATIVE_LOAD_FLAG)));
    HDassert(!((type->flags & H5C__CLASS_SKIP_READS) &&
               (type->flags & H5C__CLASS_COMPRESSED_FLAG)));

    HDassert(H5F_addr_defined(addr));
    HDassert(type->get_load_size);
    HDassert(type->deserialize);


    /* if *pf_entry_ptr is a flush dependency child, destroy that 
     * relationship now.  The client will restore the relationship with
     * the deserialized entry if appropriate.
     */
    if ( pf_entry_ptr->flush_dep_parent ) {

	HDassert(pf_entry_ptr->flush_dep_parent->addr == \
                 pf_entry_ptr->fd_parent_addr);

	if ( pf_entry_ptr->flush_dep_parent->prefetched ) {

	    /* decrement the fd_child_count on the parent */
	    HDassert(pf_entry_ptr->flush_dep_parent->fd_child_count > 0);
            (pf_entry_ptr->flush_dep_parent->fd_child_count)--;
        }

        if ( H5C_destroy_flush_dependency(pf_entry_ptr->flush_dep_parent,
                                          pf_entry_ptr) < 0 )

	    HGOTO_ERROR(H5E_CACHE, H5E_CANTUNDEPEND, FAIL,
                        "can't destroy pf entry parent flush dependency.");

        pf_entry_ptr->fd_parent_addr = HADDR_UNDEF;
    }

    HDassert(NULL == pf_entry_ptr->flush_dep_parent);
    HDassert(HADDR_UNDEF == pf_entry_ptr->fd_parent_addr);


    /* If *pf_entry_ptr is a flush dependency parent, destroy its flush 
     * dependency relationships with all its children (which must be 
     * prefetched entries as well).
     *
     * These flush dependency relationships will have to be restored 
     * after the deserialized entry is inserted into the cache in order
     * to transfer these relationships to the new entry.  Hence save the
     * pointers to the flush dependency children of *pf_enty_ptr for later
     * use.
     */
    if ( pf_entry_ptr->fd_child_count > 0 ) {

	fd_children = (H5C_cache_entry_t **)
		H5MM_calloc(sizeof(H5C_cache_entry_t **) * 
                             (size_t)(pf_entry_ptr->fd_child_count + 1));

        if ( NULL == fd_children )

                HGOTO_ERROR(H5E_CACHE, H5E_CANTALLOC, FAIL, \
                            "memory allocation failed for fd child ptr array");

        if ( H5C_destroy_pf_entry_child_flush_deps(cache_ptr, pf_entry_ptr,
                                                   fd_children) < 0)

	    HGOTO_ERROR(H5E_CACHE, H5E_CANTUNDEPEND, FAIL,
                        "can't destroy pf entry child flush dependency(s).");
    }


    /* Since the size of the on disk image is known exactly, there is 
     * no need for either a call to the get_load_size() callback, or 
     * retries if the H5C__CLASS_SPECULATIVE_LOAD_FLAG flag is set.
     * Similarly, there is no need to clamp possible reads beyond
     * EOF.
     */
    len = pf_entry_ptr->size;


    /* Deserialize the prefetched on-disk image of the entry into the 
     * native memory form 
     */
    if ( NULL == (thing = type->deserialize(pf_entry_ptr->image_ptr, 
                                            len, udata, &dirty)) )

        HGOTO_ERROR(H5E_CACHE, H5E_CANTLOAD, FAIL, "Can't deserialize image")

    /* If the client's cache has an image_len callback, check it.  As 
     * the on disk size of the entry is well known already, this call can
     * only give us new information if the H5C__CLASS_COMPRESSED_FLAG
     * is set.  However, we also use it for sanity checking.
     */
    if ( type->image_len ) {

        size_t	new_len;        /* New size of on-disk image */

	/* set magic and type field in *ds_entry_ptr.  While the image_len 
         * callback shouldn't touch the cache specific fields, it may check 
         * these fields to ensure that it it has received the expected 
         * value.
         */
        ds_entry_ptr        = (H5C_cache_entry_t *)thing;
        ds_entry_ptr->magic = H5C__H5C_CACHE_ENTRY_T_MAGIC;
        ds_entry_ptr->type  = type;

	/* verify that compressed and compressed_len are initialized */
        HDassert(compressed == FALSE);
        HDassert(compressed_size == 0);

        /* Get the actual image size for the thing */
        if ( type->image_len(thing, &new_len, &compressed, 
                             &compressed_size) < 0)

	    HGOTO_ERROR(H5E_CACHE, H5E_CANTGET, FAIL, \
                        "can't retrieve image length")

	if(new_len == 0)

	    HGOTO_ERROR(H5E_CACHE, H5E_BADVALUE, FAIL, "image length is 0")

        HDassert(((type->flags & H5C__CLASS_COMPRESSED_FLAG) != 0) ||
                 ((compressed == FALSE) && (compressed_size == 0)));

        HDassert((compressed == TRUE) || (compressed_size == 0));

	if(new_len != len) {

            if ( type->flags & H5C__CLASS_COMPRESSED_FLAG ) {

                /* if new_len != len, then compression must be 
                 * enabled on the entry.  In this case, the image_len
                 * callback should have set compressed to TRUE, set 
                 * new_len equal to the uncompressed size of the 
                 * entry, and compressed_len equal to the compressed
                 * size -- which must equal len.
                 *
                 * We can't verify the uncompressed size, but we can 
		 * verify the rest with the following assertions.
                 */
		HDassert(compressed);
                HDassert(compressed_size == len);

		/* new_len should contain the uncompressed size.  Set len
                 * equal to new_len, so that the cache will use the 
                 * uncompressed size for purposes of space allocation, etc.
                 */
                len = new_len;

            } else if (type->flags & H5C__CLASS_SPECULATIVE_LOAD_FLAG) {

                HGOTO_ERROR(H5E_CACHE, H5E_UNSUPPORTED, FAIL, \
                     "size of prefetched speculative object changed")

            } else { /* throw an error */

                HGOTO_ERROR(H5E_CACHE, H5E_UNSUPPORTED, FAIL, \
                     "size of non-speculative, non-compressed object changed")
            }
	} /* end if (new_len != len) */
    } /* end if */

    ds_entry_ptr = (H5C_cache_entry_t *)thing;

    /* In general, an entry should be clean just after it is loaded.
     *
     * However, when this code is used in the metadata cache, it is
     * possible that object headers will be dirty at this point, as
     * the deserialize function will alter object headers if necessary to
     * fix an old bug.
     *
     * In the following assert:
     *
     * 	HDassert( ( dirty == FALSE ) || ( type->id == 5 || type->id == 6 ) );
     *
     * note that type ids 5 & 6 are associated with object headers in the 
     * metadata cache.
     *
     * When we get to using H5C for other purposes, we may wish to
     * tighten up the assert so that the loophole only applies to the
     * metadata cache.
     *
     * Note that at present, dirty can't be set to true with prefetched 
     * entries.  However this may change, so include this functionality
     * against that posibility.
     *
     * Also, note that it is possible for a prefetched entry to be dirty --
     * hence the value assigned to ds_entry_ptr->is_dirty below.
     */

    HDassert( ( dirty == FALSE ) || ( type->id == 5 || type->id == 6) );

    ds_entry_ptr->magic                = H5C__H5C_CACHE_ENTRY_T_MAGIC;
    ds_entry_ptr->cache_ptr            = f->shared->cache;
    ds_entry_ptr->addr                 = addr;
    ds_entry_ptr->size                 = len;
    HDassert(ds_entry_ptr->size < H5C_MAX_ENTRY_SIZE);
    ds_entry_ptr->compressed	       = compressed;
    ds_entry_ptr->compressed_size      = compressed_size;
    ds_entry_ptr->image_ptr            = pf_entry_ptr->image_ptr;
    ds_entry_ptr->image_up_to_date     = !dirty;
    ds_entry_ptr->type                 = type;
    ds_entry_ptr->is_dirty	       = dirty | pf_entry_ptr->is_dirty;
    ds_entry_ptr->dirtied              = FALSE;
    ds_entry_ptr->is_protected         = FALSE;
    ds_entry_ptr->is_read_only         = FALSE;
    ds_entry_ptr->ro_ref_count         = 0;
    ds_entry_ptr->is_pinned            = FALSE;
    ds_entry_ptr->in_slist             = FALSE;
    ds_entry_ptr->flush_marker         = FALSE;
#ifdef H5_HAVE_PARALLEL
    ds_entry_ptr->clear_on_unprotect   = FALSE;
    ds_entry_ptr->flush_immediately    = FALSE;
#endif /* H5_HAVE_PARALLEL */
    ds_entry_ptr->flush_in_progress    = FALSE;
    ds_entry_ptr->destroy_in_progress  = FALSE;

    /* Initialize flush dependency height fields */
    ds_entry_ptr->flush_dep_parent     = NULL;

    for ( u = 0; u < H5C__NUM_FLUSH_DEP_HEIGHTS; u++ )

        ds_entry_ptr->child_flush_dep_height_rc[u] = 0;

    ds_entry_ptr->flush_dep_height     = 0;
    ds_entry_ptr->ht_next              = NULL;
    ds_entry_ptr->ht_prev              = NULL;

    ds_entry_ptr->next                 = NULL;
    ds_entry_ptr->prev                 = NULL;

    ds_entry_ptr->aux_next             = NULL;
    ds_entry_ptr->aux_prev             = NULL;

    /* initialize cache image related fields */
    ds_entry_ptr->include_in_image     = FALSE;
    ds_entry_ptr->lru_rank	       = 0;
    ds_entry_ptr->image_index	       = -1;
    ds_entry_ptr->image_dirty	       = FALSE;
    ds_entry_ptr->fd_parent_addr       = HADDR_UNDEF;
    ds_entry_ptr->fd_child_count       = pf_entry_ptr->fd_child_count;
    ds_entry_ptr->prefetched	       = FALSE;
    ds_entry_ptr->prefetch_type_id     = 0;

    H5C__RESET_CACHE_ENTRY_STATS(ds_entry_ptr);


    /* apply to to the newly deserialized entry */
    if ( H5C_tag_entry(cache_ptr, ds_entry_ptr, dxpl_id) < 0 )

        HGOTO_ERROR(H5E_CACHE, H5E_CANTTAG, FAIL, "Cannot tag metadata entry")


    /* We have successfully deserialized the prefetched entry.
     *
     * Before we return a pointer to the deserialized entry, we must remove
     * the prefetched entry from the cache, discard it, and replace it with 
     * the deserialized entry.  Note that we do not free the prefetched 
     * entries image, as that has been transferred to the deserialized
     * entry.
     *
     * Also note that we have not yet restored any flush dependencies.  This
     * must wait until the deserialized entry is inserted in the cache.
     *
     * To delete the prefetched entry from the cache:
     *
     *  1) Set pf_entry_ptr->image_ptr to NULL.  Since we have already
     *     transferred the buffer containing the image to *ds_entry_ptr,
     *	   this is not a memory leak.
     * 
     *  2) Call H5C__flush_single_entry() with the H5C__FLUSH_INVALIDATE_FLAG
     *     and H5C__FLUSH_CLEAR_ONLY_FLAG flags set.
     */

    pf_entry_ptr->image_ptr = NULL; 

    if ( pf_entry_ptr->is_dirty ) {

	HDassert(pf_entry_ptr->in_slist);
        flush_flags |= H5C__DEL_FROM_SLIST_ON_DESTROY_FLAG;
    }

    if ( H5C__flush_single_entry(f, dxpl_id, pf_entry_ptr, 
                                 flush_flags, NULL) < 0 )

        HGOTO_ERROR(H5E_CACHE, H5E_CANTEXPUNGE, FAIL, \
                    "can't expunge prefetched entry")

#ifndef NDEGUG /* verify deletion */
    H5C__SEARCH_INDEX(cache_ptr, addr, pf_entry_ptr, FAIL);

    HDassert(NULL == pf_entry_ptr);
#endif /* NDEBUG */

    /* make space in the cache if required -- in general it is not necessary
     * to even check, as typically the prefetched entry will be the same size
     * as the deserialized entry, and we just removed the prefetched entry
     * from the cache.
     *
     * However, it is possible that the entry was compressed, and thus
     * that the deserialized entry is larger than its prefetched cognate.
     * Check for space only in this case.
     */
    if ( ( ds_entry_ptr->compressed ) &&
         ( ds_entry_ptr->size > ds_entry_ptr->compressed_size ) ) {

	/* Given the relatively small sizes of compressed entries in 
         * the cache (fractal heap direct blocks, which top out at 64 K
         * at present), I don't think it is possible for the size increase
         * to be large enough to trigger a flash cache size increase.
         *
         * That said, the following code should deal with the situation
         * should it arise.
         */
        hbool_t write_permitted = FALSE;
        size_t size_increase;
        size_t empty_space;
        size_t space_needed;

	HDassert(ds_entry_ptr->size <= H5C_MAX_ENTRY_SIZE);

	size_increase = ds_entry_ptr->size - ds_entry_ptr->compressed_size;

	HDassert(size_increase > 0);

        if ( ( cache_ptr->flash_size_increase_possible ) &&
             ( size_increase > cache_ptr->flash_size_increase_threshold ) ) {

            if ( H5C__flash_increase_cache_size(cache_ptr, 0, 
                                                size_increase) < 0 ) {

                HGOTO_ERROR(H5E_CACHE, H5E_CANTPROTECT, FAIL, \
                            "H5C__flash_increase_cache_size failed.")
             }
        }

        if ( cache_ptr->index_size >= cache_ptr->max_cache_size )
           empty_space = 0;
        else
           empty_space = cache_ptr->max_cache_size - cache_ptr->index_size;

        if ( empty_space >= ds_entry_ptr->size ) {
	   space_needed = 0;
        } else {
           cache_ptr->cache_full = TRUE;
           space_needed = ds_entry_ptr->size - empty_space;
        }

	if ( space_needed > 0 ) {

	    if ( cache_ptr->check_write_permitted != NULL ) {

		if((cache_ptr->check_write_permitted)(f, &write_permitted) < 0){

                    HGOTO_ERROR(H5E_CACHE, H5E_CANTPROTECT, FAIL, \
                               "Can't get write_permitted")

                }
            } else {

                write_permitted = cache_ptr->write_permitted;
            }

	    if ( H5C_make_space_in_cache(f, dxpl_id, space_needed, 
                                         write_permitted) < 0 )

		HGOTO_ERROR(H5E_CACHE, H5E_CANTPROTECT, FAIL, \
                            "H5C_make_space_in_cache failed.")
        }
    }


    /* Insert the deserialized entry into the cache.  */

    H5C__INSERT_IN_INDEX(cache_ptr, ds_entry_ptr, FAIL)

    HDassert(!ds_entry_ptr->in_slist);

    if ( ds_entry_ptr->is_dirty ) {

	H5C__INSERT_ENTRY_IN_SLIST(cache_ptr, ds_entry_ptr, FAIL)
    }

    H5C__UPDATE_RP_FOR_INSERTION(cache_ptr, ds_entry_ptr, FAIL)


    /* Deserializing a prefetched entry is the conceptual equivalent of 
     * loading it from file.  If the deserialized entry has a notify callback,
     * send an "after load" notice now that the deserialized entry is fully
     * integrated into the cache.
     */
    if ( ( ds_entry_ptr->type->notify ) &&
         ( (ds_entry_ptr->type->notify)(H5C_NOTIFY_ACTION_AFTER_LOAD, 
                                        ds_entry_ptr) < 0 ) )

        HGOTO_ERROR(H5E_CACHE, H5E_CANTNOTIFY, FAIL, 
	    "can't notify client about entry loaded into cache")

    /* restore flush dependencies with the flush dependency children of 
     * of the prefetched entry.  Note that we must protect *ds_entry_ptr 
     * before the call to avoid triggering sanity check failures, and 
     * then unprotect it afterwards.
     */
    i = 0;
    if ( fd_children != NULL ) {

	H5C__UPDATE_RP_FOR_PROTECT(cache_ptr, ds_entry_ptr, FAIL)

        ds_entry_ptr->is_protected = TRUE;

        while ( fd_children[i] != NULL ) {

	    HDassert((fd_children[i])->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
	    HDassert((fd_children[i])->prefetched);
	    HDassert((fd_children[i])->fd_parent_addr == ds_entry_ptr->addr);

	    if (H5C_create_flush_dependency(ds_entry_ptr, fd_children[i]) < 0) {

	        HGOTO_ERROR(H5E_CACHE, H5E_CANTDEPEND, FAIL, \
                            "Can't restore child flush dependency.");
	    }

	    i++;
        } /* end while */

        H5C__UPDATE_RP_FOR_UNPROTECT(cache_ptr, ds_entry_ptr, FAIL);

	ds_entry_ptr->is_protected = FALSE;

    } /* end if ( fd_children != NULL ) */

    HDassert((unsigned)i == ds_entry_ptr->fd_child_count);

    ds_entry_ptr->fd_child_count = 0;


    H5C__UPDATE_STATS_FOR_PREFETCH_HIT(cache_ptr)


    /* finally, pass ds_entry_ptr back to the caller */

    *entry_ptr_ptr = ds_entry_ptr;

done:

    if ( fd_children )

	fd_children = (H5C_cache_entry_t **)H5MM_xfree((void *)fd_children);

    /* Cleanup on error */
    if ( FAIL == ret_value ) {

        /* Release resources */
        if ( thing && type->free_icr(thing) < 0 )

            HDONE_ERROR(H5E_CACHE, H5E_CANTFLUSH, FAIL, \
                        "free_icr callback failed")

    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_deserialize_prefetched_entry() */


/*-------------------------------------------------------------------------
 * Function:    H5C_free_image_entries_array
 *
 *		If the image entries array exists, free the image 
 *		associated with each entry, and then free the image 
 *		entries array proper.
 *
 *		Note that by the time this function is called, the cache
 *		should have removed all entries from its data structures.
 *
 * Return:      SUCCEED on success, and FAIL on failure.
 *
 * Programmer:  John Mainzer
 *              8/4/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_free_image_entries_array(H5C_t * cache_ptr)
{
    int 		i;
    H5C_image_entry_t * ie_ptr;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->image_ctl.generate_image);
    HDassert(cache_ptr->index_len == 0);

    if ( cache_ptr->image_entries != NULL ) {

        for ( i = 0; i < cache_ptr->num_entries_in_image; i++) {

 	    ie_ptr = &((cache_ptr->image_entries)[i]);

	    HDassert(ie_ptr);
            HDassert(ie_ptr->magic == H5C__H5C_IMAGE_ENTRY_T_MAGIC);
            HDassert(ie_ptr->image_index == i);
	    HDassert(ie_ptr->image_ptr != NULL);

            /* free the image */
            ie_ptr->image_ptr = H5MM_xfree(ie_ptr->image_ptr);

	    /* set magic field to bad magic so we can detect freed entries */
	    ie_ptr->magic = H5C__H5C_IMAGE_ENTRY_T_BAD_MAGIC;
	}

	/* free the image entries array */
	cache_ptr->image_entries = 
		(H5C_image_entry_t *)H5MM_xfree(cache_ptr->image_entries);
    }

    FUNC_LEAVE_NOAPI(SUCCEED)

} /* H5C_free_image_entries_array() */


/*-------------------------------------------------------------------------
 * Function:    H5C_get_cache_image_config
 *
 * Purpose:     Copy the current configuration for cache image generation
 *              on file close into the instance of H5C_cache_image_ctl_t
 *              pointed to by config_ptr.
 *
 * Return:      SUCCEED on success, and FAIL on failure.
 *
 * Programmer:  John Mainzer
 *              7/3/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_get_cache_image_config(const H5C_t * cache_ptr,
                           H5C_cache_image_ctl_t *config_ptr)
{
    herr_t ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    if ( ( cache_ptr == NULL ) || ( cache_ptr->magic != H5C__H5C_T_MAGIC ) ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, "Bad cache_ptr on entry.")
    }

    if ( config_ptr == NULL ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, "Bad config_ptr on entry.")
    }

    *config_ptr = cache_ptr->image_ctl;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_get_cache_image_config() */


/*-------------------------------------------------------------------------
 * Function:    H5C_get_serialization_in_progress
 *
 * Purpose:     Return the current value of 
 *              cache_ptr->serialization_in_progress.
 *
 * Return:      Current value of cache_ptr->serialization_in_progress.
 *
 * Programmer:  John Mainzer
 *		8/24/15
 *
 *-------------------------------------------------------------------------
 */
hbool_t
H5C_get_serialization_in_progress(H5F_t * f)
{
    H5C_t * cache_ptr;
    hbool_t ret_value;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(f);
    HDassert(f->shared);

    cache_ptr = f->shared->cache;

    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);


    /* Set return value */
    ret_value = cache_ptr->serialization_in_progress;

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_get_serialization_in_progress() */


/*-------------------------------------------------------------------------
 * Function:    H5C_load_cache_image
 *
 * Purpose:     Read the cache image superblock extension message and
 *		delete it if so directed.
 *
 *		Then load the cache image block at the specified location,
 *		decode it, and insert its contents into the metadata
 *		cache.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *		7/6/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_load_cache_image(H5F_t *    f,
                     hid_t	dxpl_id)
{
    H5C_t *             cache_ptr;
    herr_t		ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert( f );
    HDassert( f->shared );

    cache_ptr = f->shared->cache;

    HDassert( cache_ptr );
    HDassert( cache_ptr->magic == H5C__H5C_T_MAGIC );

    if ( cache_ptr->delete_image ) {

        if ( H5F_super_ext_remove_msg(f, dxpl_id, H5O_MDCI_MSG_ID) < 0 )

	    HGOTO_ERROR(H5E_CACHE, H5E_CANTREMOVE, FAIL, \
         "can't remove metadata cache image message from superblock extension")

	/* this shouldn't be necessary, but must mark the superblock dirty */
	if ( H5F_super_dirty(f) < 0 )

	    HGOTO_ERROR(H5E_CACHE, H5E_CANTMARKDIRTY, FAIL, \
                        "can't mark superblock dirty")
    }

    /* If the image address is defined, load the image, decode it, 
     * and insert its contents into the metadata cache. 
     *
     * Note that under normal operating conditions, it is an error if the 
     * image address is HADDR_UNDEF.  However, to facilitate testing,
     * we allow this special value of the image address which means that
     * no image exists, and that the load operation should be skipped 
     * silently.  
     */
    if ( HADDR_UNDEF != cache_ptr->image_addr ) {

	HDassert(cache_ptr->image_len > 0);
        HDassert(cache_ptr->image_buffer == NULL);

	/* allocate space for the image */
	cache_ptr->image_buffer = H5MM_malloc(cache_ptr->image_len + 1);

	    if ( NULL == cache_ptr->image_buffer )

                HGOTO_ERROR(H5E_CACHE, H5E_CANTALLOC, FAIL, \
                            "memory allocation failed for cache image buffer");

	/* load the image from file */
	if ( H5AC_read_cache_image(f, dxpl_id, cache_ptr->image_addr,
				   cache_ptr->image_len, 
                                   cache_ptr->image_buffer) < 0 )

            HGOTO_ERROR(H5E_CACHE, H5E_READERROR, FAIL, 
		        "Can't read metadata cache image block")

	/* decode metadata cache image */
	if ( H5C_decode_cache_image_buffer(f, cache_ptr) < 0 )

            HGOTO_ERROR(H5E_CACHE, H5E_CANTOPENFILE, FAIL, 
		        "Can't decode metadata cache image block")

	/* insert image contents into cache */
	if ( H5C_reconstruct_cache_contents(f, dxpl_id, cache_ptr) < 0 )

            HGOTO_ERROR(H5E_CACHE, H5E_CANTOPENFILE, FAIL, 
		        "Can't reconstruct cache contents from image block")

	/* free the image buffer */
        cache_ptr->image_buffer = H5MM_xfree(cache_ptr->image_buffer);

	/* if directed, free the on disk metadata cache image */
        if ( cache_ptr->delete_image ) { 

	    HDassert(HADDR_UNDEF != cache_ptr->image_addr);

            if ( H5MF_xfree(f, H5FD_MEM_SUPER, dxpl_id, cache_ptr->image_addr,
                            (hsize_t)(cache_ptr->image_len)) < 0)

                HGOTO_ERROR(H5E_CACHE, H5E_CANTFREE, FAIL, \
                            "unable to free file space for cache image block.")

            /* clean up */
            cache_ptr->image_len = 0;
            cache_ptr->image_addr = HADDR_UNDEF;
        }

        /* free the image entries array.  Note that all on disk image 
         * image buffers have been transferred to their respective
         * prefetched entries so we can just free the array of 
         * H5C_image_entry_t.
         */
#ifndef NDEBUG
        {
            int i;
            H5C_image_entry_t * ie_ptr;

	    for ( i = 0; i < cache_ptr->num_entries_in_image; i++ ) {

		ie_ptr = &((cache_ptr->image_entries)[i]);

		HDassert(ie_ptr);
                HDassert(ie_ptr->magic == H5C__H5C_IMAGE_ENTRY_T_MAGIC);
                HDassert(ie_ptr->image_index == i);
                HDassert(ie_ptr->image_ptr == NULL);
            }
        }
#endif /* NDEBUG */
	cache_ptr->image_entries =
                (H5C_image_entry_t *)H5MM_xfree(cache_ptr->image_entries);
	cache_ptr->num_entries_in_image = 0;


        H5C__UPDATE_STATS_FOR_CACHE_IMAGE_LOAD(cache_ptr)
    }

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_load_cache_image() */


/*-------------------------------------------------------------------------
 * Function:    H5C_load_cache_image_on_next_protect()
 *
 * Purpose:     Note the fact that a metadata cache image superblock 
 *		extension message exists, along with the base address
 *		and length of the metadata cache image block.
 *
 *		Once this notification is received the metadata cache 
 *		image block must be read, decoded, and loaded into the 
 *		cache on the next call to H5C_protect().
 *
 *		Further, if the file is opened R/W, the metadata cache 
 *		image superblock extension message must be deleted from 
 *		the superblock extension and the image block freed
 *
 *		Contrawise, if the file is openened R/O, the metadata
 *		cache image superblock extension message and image block
 *		must be left as is.  Further, any dirty entries in the
 *		cache image block must be marked as clean to avoid 
 *		attempts to write them on file close.
 *
 * Return:      SUCCEED 
 *
 * Programmer:  John Mainzer
 *		7/6/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_load_cache_image_on_next_protect(H5F_t * f,
				     haddr_t addr,
				     size_t len,
				     hbool_t rw)
{
    H5C_t *             cache_ptr;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert( f );
    HDassert( f->shared );

    cache_ptr = f->shared->cache;

    HDassert( cache_ptr );
    HDassert( cache_ptr->magic == H5C__H5C_T_MAGIC );

    cache_ptr->image_addr   = addr,
    cache_ptr->image_len    = len;
    cache_ptr->load_image   = TRUE;
    cache_ptr->delete_image = rw;

    FUNC_LEAVE_NOAPI(SUCCEED)

} /* H5C_load_cache_image_on_next_protect() */


/*-------------------------------------------------------------------------
 *
 * Function:    H5C_prep_for_file_close
 *
 * Purpose:     This function should be called just prior to the cache 
 *		flushes at file close.  There should be no protected 
 *		entries in the cache at this point.
 *		
 *              The objective of the call is to allow the metadata cache 
 *		to do any preparatory work prior to generation of a 
 *		cache image.
 *
 *		In particular, the cache must 
 *
 *		1) serialize all its entries,
 *
 *		2) compute the size of the metadata cache image, 
 *
 *		3) allocate space for the metadata cache image, and
 *
 *		4) setup the metadata cache image superblock extension
 *		   message with the address and size of the metadata 
 *		   cache image.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              7/3/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_prep_for_file_close(H5F_t *f, hid_t dxpl_id)
{
    H5C_t *     cache_ptr = NULL;
    int         i;
    size_t	old_image_len;
    herr_t	ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* sanity checks */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->cache);

    cache_ptr = f->shared->cache;

    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(!cache_ptr->close_warning_received);
    HDassert(cache_ptr->pl_len == 0);

    /* If the file is opened and closed without any access to 
     * any group or data set, it is possible that the cache image (if 
     * it exists) has not been read yet.  Do this now if required.
     */
    if ( cache_ptr->load_image ) {

        cache_ptr->load_image = FALSE;

        if ( H5C_load_cache_image(f, dxpl_id) < 0 )

	    HGOTO_ERROR(H5E_CACHE, H5E_CANTLOAD, FAIL, "Can't load cache image")
    }

    cache_ptr->close_warning_received = TRUE;

    if ( cache_ptr->close_warning_received ) {

        if ( cache_ptr->image_ctl.generate_image ) { /* we have work to do */

	    /* Create the cache image super block extension message.
             * 
             * Note that the base address and length of the metadata cache
 	     * image are undefined at this point, and thus will have to be
	     * updated later.
             *
             * Create the super block extension message now so that space 
	     * is allocated for it (if necessary) before we allocate space
 	     * for the cache image block.
             *
             * To simplify testing, do this only if the 
             * H5C_CI__GEN_MDCI_SBE_MESG bit is set in 
             * cache_ptr->image_ctl.flags.
             */
            if ( cache_ptr->image_ctl.flags & H5C_CI__GEN_MDCI_SBE_MESG ) {

		if ( H5C__write_cache_image_superblock_msg(f, dxpl_id, 
                                                           TRUE) < 0 )

		    HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                                "creation of cache image SB mesg failed.")
            }

	    /* serialize the cache */

	    if ( H5C_serialize_cache(f, dxpl_id) < 0 )

 	        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                            "serialization of the cache failed (1).")

	    /* Scan the cache and record data needed to construct the 
	     * cache image.  In particular, for each entry we must record:
             *
             * 1) rank in LRU (if entry is in LRU)
             *
             * 2) Whether the entry is dirty prior to flush of 
             *    cache just prior to close.
             *
             * 3) Address of flush dependency parent (if any).
             *
             * 4) Number of flush dependency children.  
             *
             * In passing, also compute the size of the metadata cache 
             * image.  Note that this is probably only a first aproximation,
             * as allocation of the superblock extension message may change
	     * the size of the file space allocation related metadata, which
             * will in turn change the size of the metadata cache image block.
             */
	    if ( H5C_prep_for_file_close__scan_entries(f, dxpl_id, 
                                                       cache_ptr) < 0 )

		HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                    "H5C_prep_for_file_close__scan_entries failed (1).")

            HDassert(HADDR_UNDEF == cache_ptr->image_addr);

	    i = 0; 
	    do {
		i++;

	        /* save the current estimate of the length of the metadata 
                 * cache image block.
                 */
                old_image_len = cache_ptr->image_len;

		/* deallocate the current cache image block if it exists */
		if ( HADDR_UNDEF != cache_ptr->image_addr ) {

		    if ( H5MF_xfree(f, H5FD_MEM_SUPER, dxpl_id, 
				    cache_ptr->image_addr, 
                                    (hsize_t)(cache_ptr->image_len)) < 0)

                        HGOTO_ERROR(H5E_CACHE, H5E_CANTFREE, FAIL, \
			    "unable to free file space for cache image block.")

		}

                /* allocate the cache image block */
		if ( HADDR_UNDEF == (cache_ptr->image_addr = 
		                     H5MF_alloc(f, H5FD_MEM_SUPER, dxpl_id, 
				             (hsize_t)(cache_ptr->image_len))) )

		    HGOTO_ERROR(H5E_CACHE, H5E_NOSPACE, FAIL, \
			"can't allocate file space for metadata cache image")

		/* update the metadata cache image superblock extension 
                 * message with the new cache image block base address and 
                 * length.
                 *
                 * to simplify testing, do this only if the 
                 * H5C_CI__GEN_MDC_IMAGE_BLK bit is set in 
                 * cache_ptr->image_ctl.flags.
                 */
                if ( cache_ptr->image_ctl.flags & H5C_CI__GEN_MDC_IMAGE_BLK ) {

		    if ( H5C__write_cache_image_superblock_msg(f, dxpl_id, 
                                                               FALSE) < 0 )

		        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                                    "update of cache image SB mesg failed.")
                }

		/* re-serialize the cache */
	        if ( H5C_serialize_cache(f, dxpl_id) < 0 )

 	            HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                                "serialization of the cache failed (2).")

		/* re-scan the cache */
	        if ( H5C_prep_for_file_close__scan_entries(f, dxpl_id, 
                                                           cache_ptr) < 0 )

		    HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                        "H5C_prep_for_file_close__scan_entries failed (2).")

	    } while ( ( i < 3 ) && ( old_image_len != cache_ptr->image_len ) );

	    if ( old_image_len != cache_ptr->image_len )

		HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                            "image len failed to converge.")

	    /* At this point:
             *
             *   1) space in the file for the metadata cache image
             *      is allocated, 
             *
             *   2) the metadata cache image superblock extension 
             *      message exists and (if so configured) contains 
             *      the correct data,
             *
             *   3) All entries in the cache are serialized with 
             *      up to date images.
             *
             *   4) All entries in the cache that will be include in
             *      the cache are marked as such, and we have a count
             *      of same.
             *
             * If there are any entries to be included in the metadata cache
             * image, allocate, populate, and sort the image_entries array.  
             * Sort the array now, as we still have the flush_dep_height 
             * for each entry.  If we wait until after the flushes, we would 
	     * have to store these values.
             *
             * If the metadata cache image will be empty, delete the 
             * metadata cache image superblock extension message, set 
             * cache_ptr->image_ctl.generate_image to FALSE.  This will
             * allow the file close to continue normally without the 
             * unecessary generation of the metadata cache image.
             */

	    if ( cache_ptr->num_entries_in_image > 0 ) {

		if ( H5C_prep_for_file_close__setup_image_entries_array(
							cache_ptr) < 0 )

		    HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \

                        "can't setup image entries array.")

		H5C_prep_for_file_close__qsort_image_entries_array( cache_ptr, 
					0, cache_ptr->num_entries_in_image - 1);

	    } else { /* cancel creation of metadata cache iamge */

	        HDassert(cache_ptr->image_entries == NULL);

		/* To avoid breaking the control flow tests, only delete 
                 * the mdci superblock extension message if the 
                 * H5C_CI__GEN_MDC_IMAGE_BLK flag is set in 
                 * cache_ptr->image_ctl.flags.
                 */
                if ( cache_ptr->image_ctl.flags & H5C_CI__GEN_MDC_IMAGE_BLK ) {

                    if ( H5F_super_ext_remove_msg(f, dxpl_id, H5O_MDCI_MSG_ID) 
			< 0 )

		        HGOTO_ERROR(H5E_CACHE, H5E_CANTREMOVE, FAIL, \
         		    "can't remove MDC image msg from superblock ext.")

		}

		cache_ptr->image_ctl.generate_image = FALSE;
	    }
        } /* end if ( cache_ptr->image_ctl.generate_image ) */

    } /* end if ( cache_ptr->close_warning_received ) */

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_prep_for_file_close() */


/*-------------------------------------------------------------------------
 * Function:    H5C_set_cache_image_config
 *
 * Purpose:	If *config_ptr contains valid data, copy it into the 
 *		image_ctl field of *cache_ptr.  Make adjustments for 
 *		changes in configuration as required.
 *
 *		Fail if the new configuration is invalid.
 *
 * Return:      SUCCEED on success, and FAIL on failure.
 *
 * Programmer:  John Mainzer
 *		7/3/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_set_cache_image_config(H5C_t *cache_ptr,
                           H5C_cache_image_ctl_t *config_ptr)
{
    herr_t	ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    if ( ( cache_ptr == NULL ) || ( cache_ptr->magic != H5C__H5C_T_MAGIC ) ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, "Bad cache_ptr on entry.")
    }

    if ( config_ptr == NULL ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, "NULL config_ptr on entry.")
    }

    if ( config_ptr->version != H5C__CURR_CACHE_IMAGE_CTL_VER ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, "Unknown config version.")
    }

    /* check general configuration section of the config: */
    if ( SUCCEED != H5C_validate_cache_image_config(config_ptr) ) {

        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, \
                    "invalid cache image configuration.")
    }

    cache_ptr->image_ctl = *config_ptr;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_set_cache_image_config() */


/*-------------------------------------------------------------------------
 * Function:    H5C_validate_cache_image_config()
 *
 * Purpose:	Run a sanity check on the provided instance of struct 
 *		H5AC_cache_image_config_t.
 *
 *		Do nothing and return SUCCEED if no errors are detected,
 *		and flag an error and return FAIL otherwise.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              6/15/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_validate_cache_image_config(H5C_cache_image_ctl_t * ctl_ptr)
{
    herr_t              ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    if ( ctl_ptr == NULL ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, "NULL ctl_ptr on entry.")
    }

    if ( ctl_ptr->version != H5C__CURR_CACHE_IMAGE_CTL_VER ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                    "Unknown cache image control version.")
    }

    if ( ( ctl_ptr->generate_image != FALSE ) &&
         ( ctl_ptr->generate_image != TRUE ) ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                    "generate_image field corrupted?")
    }

    /* at present, max image size is always limited only by cache size,
     * and hence the max_image_size field must always be zero.
     */
    if ( ctl_ptr->max_image_size != 0 ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                    "unexpected max_image_size field.")
    }

    if ( (ctl_ptr->flags & ~H5C_CI__ALL_FLAGS) != 0 ) {

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, "unknown flag set.")
    }

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_validate_cache_image_config() */


/***************************************************************************/
/*************** Class functions for H5C__PREFETCHED_ENTRY_TYPE ************/
/***************************************************************************/

/***************************************************************************
 * With two exceptions, these functions should never be called, and thus 
 * there is little point in documenting them separately as they all simply
 * throw an error.
 *
 * See header comments for the two exceptions (free_icr and notify).
 * 
 *                                                     JRM - 8/13/15
 *
 ***************************************************************************/

static herr_t
H5C__prefetched_entry_get_load_size(const void H5_ATTR_UNUSED *udata_ptr,
    size_t H5_ATTR_UNUSED *image_len_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__prefetched_entry_get_load_size() */


static void *
H5C__prefetched_entry_deserialize(const void H5_ATTR_UNUSED * image_ptr, 
    size_t H5_ATTR_UNUSED len, void H5_ATTR_UNUSED * udata, 
    hbool_t H5_ATTR_UNUSED * dirty_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(NULL)
} /* end H5C__prefetched_entry_deserialize() */


static herr_t
H5C__prefetched_entry_image_len(const void H5_ATTR_UNUSED *thing,
    size_t H5_ATTR_UNUSED *image_len_ptr, 
    hbool_t H5_ATTR_UNUSED *compressed_ptr, 
    size_t H5_ATTR_UNUSED *compressed_len_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__prefetched_entry_image_len() */


static herr_t
H5C__prefetched_entry_pre_serialize(const H5F_t H5_ATTR_UNUSED *f, 
    hid_t H5_ATTR_UNUSED dxpl_id, void H5_ATTR_UNUSED *thing, 
    haddr_t H5_ATTR_UNUSED addr, size_t H5_ATTR_UNUSED len,
    size_t H5_ATTR_UNUSED compressed_len, haddr_t H5_ATTR_UNUSED *new_addr_ptr, 
    size_t H5_ATTR_UNUSED *new_len_ptr, 
    size_t H5_ATTR_UNUSED *new_compressed_len_ptr,
    unsigned H5_ATTR_UNUSED *flags_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__prefetched_entry_pre_serialize() */


static herr_t
H5C__prefetched_entry_serialize(const H5F_t H5_ATTR_UNUSED *f, 
    void H5_ATTR_UNUSED *image_ptr,
    size_t H5_ATTR_UNUSED len, void H5_ATTR_UNUSED *thing)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__prefetched_entry_serialize() */


/*-------------------------------------------------------------------------
 * Function:    H5C__prefetched_entry_notify
 *
 * Purpose:     On H5AC_NOTIFY_ACTION_BEFORE_EVICT, check to see if the 
 *		target entry is a child in a flush dependency relationship.
 *		If it is, destroy that flush dependency relationship.
 *
 *		Ignore on all other notifications.
 *
 * Return:      Success:        SUCCEED
 *              Failure:        FAIL
 *
 * Programmer:  John Mainzer
 *              8/13/15
 *
 *-------------------------------------------------------------------------
 */

static herr_t
H5C__prefetched_entry_notify(H5C_notify_action_t action, void * thing)
{
    H5C_cache_entry_t *  entry_ptr; 
    herr_t               ret_value = SUCCEED;

    FUNC_ENTER_STATIC

    HDassert(thing);

    entry_ptr = (H5C_cache_entry_t *)thing;

    HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
    HDassert(entry_ptr->prefetched);

    switch ( action ) {

        case H5AC_NOTIFY_ACTION_AFTER_INSERT:
        case H5AC_NOTIFY_ACTION_AFTER_LOAD:
        case H5AC_NOTIFY_ACTION_AFTER_FLUSH:
            /* do nothing */
            break;

        case H5AC_NOTIFY_ACTION_BEFORE_EVICT:
            if ( entry_ptr->flush_dep_parent ) {

		HDassert(entry_ptr->flush_dep_parent->magic ==
                         H5C__H5C_CACHE_ENTRY_T_MAGIC);
                HDassert(entry_ptr->flush_dep_parent->addr ==
                         entry_ptr->fd_parent_addr);

                /* destroy flush dependency with flush dependency parent */
                if ( H5C_destroy_flush_dependency(entry_ptr->flush_dep_parent, 
                                                  entry_ptr) < 0) {

                    HGOTO_ERROR(H5E_CACHE, H5E_CANTUNDEPEND, FAIL, \
			"unable to destroy prefetched entry flush dependency")
                }
            } /* end if */
            break;

        default:
            HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, \
                        "unknown action from metadata cache")
            break;
    } /* end switch */

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* end H5C__prefetched_entry_notify() */


/*-------------------------------------------------------------------------
 * Function:    H5C__prefetched_entry_free_icr
 *
 * Purpose:     Free the in core representation of the prefetched entry.
 *		Verify that the image buffer associated with the entry
 *		has been either transferred or freed.
 *
 * Return:      Success:        SUCCEED
 *              Failure:        FAIL
 *
 * Programmer:  John Mainzer
 *              8/13/15
 *
 *-------------------------------------------------------------------------
 */

static herr_t
H5C__prefetched_entry_free_icr(void * thing)
{
    H5C_cache_entry_t *  entry_ptr; 
    herr_t               ret_value = SUCCEED;

    FUNC_ENTER_STATIC

    HDassert(thing);

    entry_ptr = (H5C_cache_entry_t *)thing;

    HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_BAD_MAGIC);
    HDassert(entry_ptr->prefetched);

    if ( entry_ptr->image_ptr != NULL )

	HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                    "prefetched entry image buffer still attatched?")

    entry_ptr = (H5C_cache_entry_t *)H5MM_xfree((void *)entry_ptr);

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* end H5C__prefetched_entry_free_icr() */


static herr_t 
H5C__prefetched_entry_fsf_size(const void H5_ATTR_UNUSED * thing, 
    size_t H5_ATTR_UNUSED *fsf_size_ptr)
{
    FUNC_ENTER_STATIC_NOERR /* Yes, even though this pushes an error on the stack */

    HERROR(H5E_CACHE, H5E_SYSTEM, "called unreachable fcn.");

    FUNC_LEAVE_NOAPI(FAIL)
} /* end H5C__prefetched_entry_fsf_size() */



/*************************************************************************/
/**************************** Private Functions: *************************/
/*************************************************************************/

/*-------------------------------------------------------------------------
 * Function:    H5C_cache_image_block_entry_header_size
 *
 * Purpose:     Compute the size of the header of the metadata cache
 *		image block, and return the value.
 *
 * Return:      Size of the header section of the metadata cache image 
 *		block in bytes.
 *
 * Programmer:  John Mainzer
 *		7/27/15
 *
 *-------------------------------------------------------------------------
 */
size_t
H5C_cache_image_block_entry_header_size(H5F_t * f)
{
    size_t ret_value;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* Set return value */
    ret_value = (size_t)( 4 +                   /* signature                */
			  1 +			/* type                     */
			  1 +			/* flags                    */
			  2 +			/* dependency child count   */
			  4 +			/* index in LRU             */
                          H5F_SIZEOF_ADDR(f) +  /* dependency parent offset */
			  H5F_SIZEOF_ADDR(f) +  /* entry offset             */
			  H5F_SIZEOF_SIZE(f) ); /* entry length             */

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_cache_image_block_entry_header_size() */


/*-------------------------------------------------------------------------
 * Function:    H5C_cache_image_block_header_size
 *
 * Purpose:     Compute the size of the header of the metadata cache
 *		image block, and return the value.
 *
 * Return:      Size of the header section of the metadata cache image 
 *		block in bytes.
 *
 * Programmer:  John Mainzer
 *		7/27/15
 *
 *-------------------------------------------------------------------------
 */
size_t
H5C_cache_image_block_header_size(void)
{
    size_t ret_value;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* Set return value */
    ret_value = (size_t)( 4 +                   /* signature           */
			  1 +			/* version             */
			  4 );			/* num_entries         */

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_cache_image_block_header_size() */


/*-------------------------------------------------------------------------
 * Function:    H5C_decode_cache_image_buffer()
 *
 *		Allocate a suitably size array of instances of 
 *		H5C_image_entry_t and and set cache_ptr->image_entries
 *		to point to this array.  Set cache_ptr->num_entries_in_image
 *		equal to the number of entries in this array.
 *
 *		Decode the contents of cache_ptr->image_buffer into the 
 *		array.
 *
 * Return:      SUCCEED on success, and FAIL on failure.
 *
 * Programmer:  John Mainzer
 *              8/9/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_decode_cache_image_buffer(H5F_t * f, H5C_t * cache_ptr)
{
    int			i;
    uint32_t    	read_chksum;
    uint32_t    	computed_chksum;
    size_t 		entry_header_size;
    const uint8_t *		p;
    herr_t 		ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert(f);
    HDassert(f->shared);
    HDassert(cache_ptr == f->shared->cache);
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->image_buffer);
    HDassert(cache_ptr->image_len > 0);
    HDassert(cache_ptr->image_entries == NULL);
    HDassert(cache_ptr->num_entries_in_image == 0);


    p = (uint8_t *)cache_ptr->image_buffer;


    /* decode metadata cache image header */

    if ( NULL == (p = H5C_decode_cache_image_header(cache_ptr, p)) )

        HGOTO_ERROR(H5E_CACHE, H5E_CANTOPENFILE, FAIL, \
		    "cache image header decode failed.")

    HDassert((size_t)(p - (uint8_t *)cache_ptr->image_buffer) < 
             cache_ptr->image_len);


    /* we should now have cache_ptr->num_entries_in_image -- allocate the
     * image entries array.
     */
    HDassert(cache_ptr->num_entries_in_image > 0);

    cache_ptr->image_entries = (H5C_image_entry_t *)
                H5MM_malloc(sizeof(H5C_image_entry_t) *
                            (size_t)(cache_ptr->num_entries_in_image + 1));

    if ( NULL == cache_ptr->image_entries )

	HGOTO_ERROR(H5E_CACHE, H5E_CANTALLOC, FAIL, \
                    "memory allocation failed image entries array")


    /* load the image entries */

    entry_header_size = H5C_cache_image_block_entry_header_size(f);

    for ( i = 0; i < cache_ptr->num_entries_in_image; i++ ) {

	(cache_ptr->image_entries)[i].magic = H5C__H5C_IMAGE_ENTRY_T_MAGIC;
        (cache_ptr->image_entries)[i].flush_dep_height = 0;
        (cache_ptr->image_entries)[i].image_index = i;
        (cache_ptr->image_entries)[i].image_ptr = NULL;

	p = H5C_decode_cache_image_entry(f, cache_ptr, p, i, entry_header_size);

	if ( NULL ==  p )

            HGOTO_ERROR(H5E_CACHE, H5E_CANTOPENFILE, FAIL, \
                        "entry image decode failed.")
    }

    HDassert((size_t)(p - (uint8_t *)cache_ptr->image_buffer) < 
             cache_ptr->image_len);


    /* load the adaptive cache resize status -- not yet */


    /* verify the checksum */

    UINT32DECODE(p, read_chksum);

    HDassert((size_t)(p - (uint8_t *)cache_ptr->image_buffer) == 
             cache_ptr->image_len);

    computed_chksum = H5_checksum_metadata(cache_ptr->image_buffer, 
                         (size_t)(cache_ptr->image_len - H5F_SIZEOF_CHKSUM), 0);

    if ( read_chksum != computed_chksum )

        HGOTO_ERROR(H5E_CACHE, H5E_CANTOPENFILE, FAIL, 
	    "bad checksum on metadata cache image block")

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_decode_cache_image_buffer() */


/*-------------------------------------------------------------------------
 * Function:    H5C_decode_cache_image_header()
 *
 *		Decode the metadata cache image buffer header from the 
 *		supplied buffer and load the data into the supplied instance
 *		of H5C_t.  Return a pointer to the first byte in the buffer
 *		after the header image, or NULL on failure.
 *
 * Return:      Pointer to first byte after the header image on success.
 *		NULL on failure.
 *
 * Programmer:  John Mainzer
 *              8/6/15
 *
 *-------------------------------------------------------------------------
 */
const uint8_t *
H5C_decode_cache_image_header(H5C_t * cache_ptr, const uint8_t * buf)
{
    uint8_t		version;
    int32_t		num_entries_in_image;
    size_t 		actual_header_len;
    size_t		expected_header_len;
    const uint8_t *	p;
    const uint8_t * 	ret_value = NULL;      /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(buf);

    p = buf;

    /* check signature */
    if ( HDmemcmp(buf, H5C__MDCI_BLOCK_SIGNATURE, 
                  (size_t)H5C__MDCI_BLOCK_SIGNATURE_LEN) )

	HGOTO_ERROR(H5E_CACHE, H5E_BADVALUE, NULL, \
                    "Bad metadata cache image header signature")

    p += H5C__MDCI_BLOCK_SIGNATURE_LEN;


    /* check version */
    version = *p++;

    if ( version != (uint8_t)H5C__MDCI_BLOCK_VERSION_0 )

	HGOTO_ERROR(H5E_CACHE, H5E_BADVALUE, NULL, \
                    "Bad metadata cache image version")


    /* read num entries */
    INT32DECODE(p, num_entries_in_image);

    if ( num_entries_in_image <= 0 ) 

	HGOTO_ERROR(H5E_CACHE, H5E_BADVALUE, NULL, \
                    "Bad metadata cache entry count")

    cache_ptr->num_entries_in_image = num_entries_in_image;


    /* verify expected length of header */
    actual_header_len = (size_t)(p - buf);
    expected_header_len = H5C_cache_image_block_header_size();

    if ( actual_header_len != expected_header_len )

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, NULL, "Bad header image len.")

    ret_value = p;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_decode_cache_image_header() */


/*-------------------------------------------------------------------------
 * Function:    H5C_decode_cache_image_entry()
 *
 *		Decode the metadata cache image entry from the supplied 
 *		buffer into the supplied instance of H5C_image_entry_t.
 *		This includes allocating a buffer for the entry image,
 *		loading it, and seting ie_ptr->image_ptr to point to 
 *		the buffer.
 *
 *		Return a pointer to the first byte after the header image 
 *		in the buffer, or NULL on failure.
 *
 * Return:      Pointer to first byte after the header image on success.
 *		NULL on failure.
 *
 * Programmer:  John Mainzer
 *              8/6/15
 *
 *-------------------------------------------------------------------------
 */
const uint8_t *
H5C_decode_cache_image_entry(H5F_t * f, 
			     H5C_t * cache_ptr,
                             const uint8_t * buf, 
                             int entry_num,
                             size_t expected_entry_header_len)
{
    hbool_t		is_dirty = FALSE;
    hbool_t		in_lru = FALSE;
    hbool_t		is_fd_parent = FALSE;
    hbool_t		is_fd_child = FALSE;
    haddr_t		fd_parent_addr;
    haddr_t 		addr;
    size_t		size;
    void *		image_ptr;
    uint8_t             flags = 0;
    uint8_t		type_id;
    uint16_t 		fd_child_count;
    int32_t		lru_rank;
    H5C_image_entry_t * ie_ptr = NULL;
    const uint8_t *	p;
    const uint8_t *	ret_value = NULL;      /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(f);
    HDassert(f->shared);
    HDassert(cache_ptr == f->shared->cache);
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(buf);
    HDassert(entry_num >= 0);
    HDassert(entry_num < cache_ptr->num_entries_in_image);

    ie_ptr = &((cache_ptr->image_entries)[entry_num]);
    
    HDassert(ie_ptr);
    HDassert(ie_ptr->magic == H5C__H5C_IMAGE_ENTRY_T_MAGIC);

    p = buf;


    /* check signature */
    if ( HDmemcmp(buf, H5C__MDCI_ENTRY_SIGNATURE, 
                  (size_t)H5C__MDCI_ENTRY_SIGNATURE_LEN) )

	HGOTO_ERROR(H5E_CACHE, H5E_BADVALUE, NULL, \
                    "Bad metadata cache image entry signature")

    p += H5C__MDCI_ENTRY_SIGNATURE_LEN;


    /* decode type id */
    type_id = *p++;


    /* decode flags */
    flags = *p++;
    
    if ( flags & H5C__MDCI_ENTRY_DIRTY_FLAG ) 		is_dirty = TRUE;

    if ( flags & H5C__MDCI_ENTRY_IN_LRU_FLAG ) 		in_lru = TRUE;

    if ( flags & H5C__MDCI_ENTRY_IS_FD_PARENT_FLAG ) 	is_fd_parent = TRUE;

    if ( flags & H5C__MDCI_ENTRY_IS_FD_CHILD_FLAG )	is_fd_child = TRUE;


    /* decode dependency child count */
    UINT16DECODE(p, fd_child_count);

    HDassert( ( is_fd_parent && ( fd_child_count > 0 ) ) ||
              ( !is_fd_parent && ( fd_child_count == 0 ) ) );


    /* decode index in LRU */
    INT32DECODE(p, lru_rank);

    HDassert( ( in_lru && ( lru_rank >= 0 ) ) ||
              ( ! in_lru && ( lru_rank == -1 ) ) );


    /* decode dependency parent offset */
    H5F_addr_decode(f, &p, &fd_parent_addr);

    HDassert( ( is_fd_child && ( HADDR_UNDEF != fd_parent_addr ) ) ||
              ( ! is_fd_child && ( HADDR_UNDEF == fd_parent_addr ) ) );


    /* decode entry offset */
    H5F_addr_decode(f, &p, &addr);
    
    HDassert(HADDR_UNDEF != addr);


    /* decode entry length */
    H5F_DECODE_LENGTH(f, p, size);

    HDassert(size > 0);


    /* allocate buffer for entry image */
    image_ptr = H5MM_malloc(size + H5C_IMAGE_EXTRA_SPACE);

    if ( NULL == image_ptr )

	HGOTO_ERROR(H5E_CACHE, H5E_CANTALLOC, NULL, \
                    "memory allocation failed for on disk image buffer")

#if H5C_DO_MEMORY_SANITY_CHECKS
        HDmemcpy(((uint8_t *)image_ptr) + size, H5C_IMAGE_SANITY_VALUE, 
                 H5C_IMAGE_EXTRA_SPACE);
#endif /* H5C_DO_MEMORY_SANITY_CHECKS */


    /* copy the entry image from the cache image block */

    HDmemcpy(image_ptr, p, size);
    p += size;


    /* verify expected length of entry image */

    if ( (size_t)(p - buf) != expected_entry_header_len + size )

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, NULL, "Bad entry image len.")

    
    /* copy data into target */
    ie_ptr->addr             = addr;
    ie_ptr->size             = size;
    ie_ptr->type_id          = (int32_t)type_id;
    ie_ptr->lru_rank         = lru_rank;
    ie_ptr->is_dirty         = is_dirty;
    ie_ptr->fd_parent_addr   = fd_parent_addr;
    ie_ptr->fd_child_count   = (uint64_t)fd_child_count;
    ie_ptr->image_ptr        = image_ptr;


    ret_value = p;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_decode_cache_image_entry() */


/*-------------------------------------------------------------------------
 * Function:    H5C_destroy_pf_entry_child_flush_deps()
 *
 *		Destroy all flush dependencies in this the supplied 
 *		prefetched entry is the parent.  Note that the children
 *		in these flush dependencies must be prefetched entries as 
 *		well.
 *
 *		As this action is part of the process of transferring all
 *		such flush dependencies to the deserialized version of the
 *		prefetched entry, ensure that the data necessary to complete
 *		the transfer is retained.
 *
 *		Note: The current implementation of this function is 
 *		      quite inefficient -- mostly due to the current 
 *		      implementation of flush dependencies.  This should
 *		      be fixed at some point.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              8/11/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_destroy_pf_entry_child_flush_deps(H5C_t *cache_ptr, 
                                      H5C_cache_entry_t * pf_entry_ptr,
				      H5C_cache_entry_t ** fd_children)
{
    int			i;
    int			entries_visited = 0;
    int			fd_children_found = 0;
    H5C_cache_entry_t * entry_ptr;
    herr_t		ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* sanity checks */
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(pf_entry_ptr);
    HDassert(pf_entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
    HDassert(pf_entry_ptr->type);
    HDassert(pf_entry_ptr->type->id == H5AC_PREFETCHED_ENTRY_ID);
    HDassert(pf_entry_ptr->prefetched);
    HDassert(pf_entry_ptr->fd_child_count > 0);
    HDassert(fd_children);

    /* scan each entry on the hash table */
    for ( i = 0; i < H5C__HASH_TABLE_LEN; i++ )
    {
	entry_ptr = cache_ptr->index[i];

        while ( entry_ptr != NULL )
        {
	    HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);

	    if ( ( entry_ptr->prefetched ) &&
                 ( entry_ptr->flush_dep_parent == pf_entry_ptr ) ) {

		HDassert(entry_ptr->fd_parent_addr == pf_entry_ptr->addr);
                HDassert(entry_ptr->type);
                HDassert(entry_ptr->type->id == H5AC_PREFETCHED_ENTRY_ID);

		HDassert(NULL == fd_children[fd_children_found]);

		fd_children[fd_children_found] = entry_ptr;

		fd_children_found++;

                if ( H5C_destroy_flush_dependency(pf_entry_ptr, entry_ptr) < 0 )

	            HGOTO_ERROR(H5E_CACHE, H5E_CANTUNDEPEND, FAIL,
                              "can't destroy pf entry child flush dependency.");
            }

            entries_visited++;

            entry_ptr = entry_ptr->ht_next;
        }
    }

    HDassert(NULL == fd_children[fd_children_found]);

    HDassert((unsigned)fd_children_found == pf_entry_ptr->fd_child_count);

    HDassert(entries_visited == cache_ptr->index_len);

    HDassert(!pf_entry_ptr->is_pinned);

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_destroy_pf_entry_child_flush_deps() */


/*-------------------------------------------------------------------------
 * Function:    H5C_encode_cache_image_header()
 *
 *		Encode the metadata cache image buffer header in the 
 *		supplied buffer.  Return a pointer to the first byte 
 *		after the header image in the buffer, or NULL on failure.
 *
 * Return:      Pointer to first byte after the header image on success.
 *		NULL on failure.
 *
 * Programmer:  John Mainzer
 *              8/6/15
 *
 *-------------------------------------------------------------------------
 */
uint8_t *
H5C_encode_cache_image_header(H5C_t * cache_ptr, uint8_t * buf)
{
    size_t 	actual_header_len;
    size_t	expected_header_len;
    uint8_t *	p;
    uint8_t * 	ret_value = NULL;      /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->image_ctl.generate_image);
    HDassert(cache_ptr->index_len == 0);
    HDassert(buf);

    p = buf;

    /* write signature */
    HDmemcpy(p, H5C__MDCI_BLOCK_SIGNATURE, 
             (size_t)H5C__MDCI_BLOCK_SIGNATURE_LEN);
    p += H5C__MDCI_BLOCK_SIGNATURE_LEN;

    /* write version */
    *p++ = (uint8_t)H5C__MDCI_BLOCK_VERSION_0;

    /* write num entries */
    INT32ENCODE(p, cache_ptr->num_entries_in_image);

    /* verify expected length of header */
    actual_header_len = (size_t)(p - buf);
    expected_header_len = H5C_cache_image_block_header_size();

    if ( actual_header_len != expected_header_len )

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, NULL, "Bad header image len.")

    ret_value = p;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_encode_cache_image_header() */


/*-------------------------------------------------------------------------
 * Function:    H5C_encode_cache_image_entry()
 *
 *		Encode the metadata cache image buffer header in the 
 *		supplied buffer.  Return a pointer to the first byte 
 *		after the header image in the buffer, or NULL on failure.
 *
 * Return:      Pointer to first byte after the header image on success.
 *		NULL on failure.
 *
 * Programmer:  John Mainzer
 *              8/6/15
 *
 *-------------------------------------------------------------------------
 */
uint8_t *
H5C_encode_cache_image_entry(H5F_t * f, 
                             H5C_t * cache_ptr, 
                             uint8_t * buf, 
                             int entry_num,
                             size_t expected_entry_header_len)
{
    H5C_image_entry_t *	ie_ptr;
    uint8_t             flags = 0;
    uint8_t *		p;
    uint8_t * 		ret_value = NULL;      /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    HDassert(f);
    HDassert(f->shared);
    HDassert(cache_ptr == f->shared->cache);
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->image_ctl.generate_image);
    HDassert(cache_ptr->index_len == 0);
    HDassert(buf);
    HDassert(entry_num >= 0);
    HDassert(entry_num < cache_ptr->num_entries_in_image);

    ie_ptr = &((cache_ptr->image_entries)[entry_num]);

    HDassert(ie_ptr->magic == H5C__H5C_IMAGE_ENTRY_T_MAGIC);

    p = buf;


    /* copy signature */

    HDmemcpy(p, H5C__MDCI_ENTRY_SIGNATURE, 
             (size_t)H5C__MDCI_ENTRY_SIGNATURE_LEN);
    p += H5C__MDCI_ENTRY_SIGNATURE_LEN;


    /* encode type */

    if ( ( ie_ptr->type_id < 0 ) || ( ie_ptr->type_id > 255 ) )
        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, NULL, "type_id out of range.")

    *p++ = (uint8_t)(ie_ptr->type_id);
    

    /* compose and encode flags */

    if ( ie_ptr->is_dirty ) 
	flags |= H5C__MDCI_ENTRY_DIRTY_FLAG;

    if ( ie_ptr->lru_rank > 0) 
	flags |= H5C__MDCI_ENTRY_IN_LRU_FLAG;

    if ( ie_ptr->fd_child_count > 0 )
	flags |= H5C__MDCI_ENTRY_IS_FD_PARENT_FLAG;

    if ( H5F_addr_defined(ie_ptr->fd_parent_addr) ) 
	flags |= H5C__MDCI_ENTRY_IS_FD_CHILD_FLAG;

    *p++ = flags;


    /* validate and encode dependency child count */

    if ( ie_ptr->fd_child_count > H5C__MDCI_MAX_FD_CHILDREN )
        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, NULL, "fd_child_count out of range.")

    UINT16ENCODE(p, (uint16_t)(ie_ptr->fd_child_count));


    /* encode index in LRU */

    INT32ENCODE(p, ie_ptr->lru_rank);


    /* encode dependency parent offset */

    H5F_addr_encode(f, &p, ie_ptr->fd_parent_addr);


    /* encode entry offset */

    H5F_addr_encode(f, &p, ie_ptr->addr);


    /* encode entry length */

    H5F_ENCODE_LENGTH(f, p, ie_ptr->size);


    /* copy entry image */

    HDmemcpy(p, ie_ptr->image_ptr, ie_ptr->size);
    p += ie_ptr->size;


    /* verify expected length of entry image */

    if ( (size_t)(p - buf) != expected_entry_header_len + ie_ptr->size )

        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, NULL, "Bad entry image len.")


    ret_value = p;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_encode_cache_image_entry() */


/*-------------------------------------------------------------------------
 *
 * Function:    H5C_prep_for_file_close__partition_image_entries_array
 *
 * Purpose:     Perform a quick sort on cache_ptr->image_entries.
 *		Entries are sorted first by flush dependency height,
 *		and then by LRU rank.
 *		
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              8/4/15
 *
 *-------------------------------------------------------------------------
 */
void
H5C_prep_for_file_close__partition_image_entries_array(H5C_t * cache_ptr, 
                                                       int bottom, 
						       int * middle_ptr,
                                                       int top)
{
    hbool_t done = FALSE;
    hbool_t i_le_pivot;
    hbool_t j_ge_pivot;
    int i;
    int j;
    H5C_image_entry_t   tmp;
    H5C_image_entry_t * pivot_ptr;
    H5C_image_entry_t * ith_ptr;
    H5C_image_entry_t * jth_ptr;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* sanity checks */
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->num_entries_in_image > 0);
    HDassert(cache_ptr->image_entries != NULL);
    HDassert(bottom >= 0);
    HDassert(middle_ptr);
    HDassert(top < cache_ptr->num_entries_in_image);
    HDassert(bottom <= top);

    pivot_ptr = &((cache_ptr->image_entries)[bottom]);

    HDassert((pivot_ptr->flush_dep_height == 0) ||
             (pivot_ptr->lru_rank == -1));

    i = bottom - 1;
    j = top + 1;

    while ( ! done ) {
#if 0 /* old code */ /* JRM */
	do {
	    i_le_pivot = FALSE;
	    i++;
            ith_ptr = &((cache_ptr->image_entries)[i]);

	    if ( ( ith_ptr->flush_dep_height > 0 ) &&
                 ( ith_ptr->flush_dep_height <= pivot_ptr->flush_dep_height ) )
            {

		HDassert(ith_ptr->lru_rank == -1);

		i_le_pivot = TRUE;

	    } else if ( ( ith_ptr->flush_dep_height == 0 ) &&
                        ( ith_ptr->lru_rank >= pivot_ptr->lru_rank ) ) {

		/* HDassert(pivot_ptr->flush_dep_height == 0); */

		i_le_pivot = TRUE;
	    }
        } while ( ! i_le_pivot );
#else /* new code */ /* JRM */
	do {
	    i_le_pivot = FALSE;
	    i++;
            ith_ptr = &((cache_ptr->image_entries)[i]);

            if ( pivot_ptr->flush_dep_height > 0 )
            {
                HDassert(pivot_ptr->lru_rank == -1);

		if ( ith_ptr->flush_dep_height <= pivot_ptr->flush_dep_height )
                {
		    i_le_pivot = TRUE;
                }
            } 
            else 
            {
                HDassert(pivot_ptr->lru_rank >= -1);

		if ( ( ith_ptr->lru_rank >= pivot_ptr->lru_rank ) &&
                     ( ith_ptr->flush_dep_height == 0 ) )
                {
		    i_le_pivot = TRUE;
                }
            } 
        } while ( ! i_le_pivot );

#endif /* new code */ /* JRM */

#if 0 /* old code */ /* JRM */
	do {
	    j_ge_pivot = FALSE;
	    j--;
	    jth_ptr = &((cache_ptr->image_entries)[j]);

	    if ( ( jth_ptr->flush_dep_height > 0 ) &&
                 ( jth_ptr->flush_dep_height >= pivot_ptr->flush_dep_height ) )
            {

		HDassert(jth_ptr->lru_rank == -1);

		j_ge_pivot = TRUE;

	    } else if ( ( jth_ptr->flush_dep_height == 0 ) &&
                        ( jth_ptr->lru_rank <= pivot_ptr->lru_rank ) ) {

		/* HDassert(pivot_ptr->flush_dep_height == 0); */

		j_ge_pivot = TRUE;
	    }
	} while  ( ! j_ge_pivot );

#else /* new code */ /* JRM */
	do {
	    j_ge_pivot = FALSE;
	    j--;
	    jth_ptr = &((cache_ptr->image_entries)[j]);

            if ( pivot_ptr->flush_dep_height > 0 )
            {
                HDassert(pivot_ptr->lru_rank == -1);

		if ( jth_ptr->flush_dep_height >= pivot_ptr->flush_dep_height )
                {
		    HDassert(jth_ptr->lru_rank == -1);

		    j_ge_pivot = TRUE;
                }
            } 
            else 
            {
                HDassert(pivot_ptr->lru_rank >= -1);

		if ( ( jth_ptr->lru_rank <= pivot_ptr->lru_rank ) ||
                     ( jth_ptr->flush_dep_height > 0 ) )
                {
		    j_ge_pivot = TRUE;
                }
            } 
	} while  ( ! j_ge_pivot );

#endif /* new code */ /* JRM */

	if ( i < j ) { /* swap ith and jth entries */

	    tmp = (cache_ptr->image_entries)[i];

            (cache_ptr->image_entries)[i] = *jth_ptr;
            ((cache_ptr->image_entries)[i]).image_index = i;

            (cache_ptr->image_entries)[j] = tmp;
            ((cache_ptr->image_entries)[j]).image_index = j;

	} else {

	   done = TRUE;
	   *middle_ptr = j;
	}
    }

    FUNC_LEAVE_NOAPI_VOID

} /* H5C_prep_for_file_close__partition_image_entries_array() */


/*-------------------------------------------------------------------------
 *
 * Function:    H5C_prep_for_file_close__qsort_image_entries_array
 *
 * Purpose:     Perform a quick sort on cache_ptr->image_entries.
 *		Entries are sorted first by highest flush dependency 
 *		height, and then by least LRU rank.
 *		
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              8/4/15
 *
 *-------------------------------------------------------------------------
 */
void
H5C_prep_for_file_close__qsort_image_entries_array(H5C_t * cache_ptr, 
                                                   int bottom, 
                                                   int top)
{
    int			middle;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* sanity checks */
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->num_entries_in_image > 0);
    HDassert(cache_ptr->image_entries != NULL);
    HDassert(bottom >= 0);
    HDassert(top < cache_ptr->num_entries_in_image);
    HDassert(bottom <= top);

    if ( bottom < top ) {

        H5C_prep_for_file_close__partition_image_entries_array(cache_ptr, 
							       bottom, 
                                                               &middle, 
                                                               top);

	H5C_prep_for_file_close__qsort_image_entries_array(cache_ptr, 
                                                           bottom, 
                                                           middle);

	
	H5C_prep_for_file_close__qsort_image_entries_array(cache_ptr, 
                                                           middle + 1, 
                                                           top);
    }

#ifndef NDEBUG
    /* verify sort if this is the top level qsort call */
    if ( ( bottom == 0 ) && ( top + 1 == cache_ptr->num_entries_in_image ) ) {

        int i;
        H5C_image_entry_t * ie_ptr;
        H5C_image_entry_t * next_ie_ptr;

        for ( i = 0; i < top; i++ ) {

	    ie_ptr = &((cache_ptr->image_entries)[i]);
            next_ie_ptr = &((cache_ptr->image_entries)[i + 1]);

	    HDassert(ie_ptr);
            HDassert(ie_ptr->magic == H5C__H5C_IMAGE_ENTRY_T_MAGIC);
            HDassert(ie_ptr->image_index == i);
            HDassert((ie_ptr->flush_dep_height == 0) ||
                     (ie_ptr->lru_rank == -1));

	    HDassert(next_ie_ptr);
            HDassert(next_ie_ptr->magic == H5C__H5C_IMAGE_ENTRY_T_MAGIC);
            HDassert(next_ie_ptr->image_index == i + 1);
            HDassert((next_ie_ptr->flush_dep_height == 0 ) ||
                     (next_ie_ptr->lru_rank == -1 ) );

            HDassert(ie_ptr->flush_dep_height >= 
                     next_ie_ptr->flush_dep_height);

	    HDassert((ie_ptr->flush_dep_height > 0) ||
                     ((ie_ptr->lru_rank <= 0) &&
                      (ie_ptr->lru_rank <= next_ie_ptr->lru_rank)) ||
                     (ie_ptr->lru_rank < next_ie_ptr->lru_rank));
        }
    }
#endif /* NDEBUG */

    FUNC_LEAVE_NOAPI_VOID

} /* H5C_prep_for_file_close__qsort_image_entries_array() */


/*-------------------------------------------------------------------------
 *
 * Function:    H5C_prep_for_file_close__setup_image_entries_array
 *
 * Purpose:     Allocate space for the image_entries array, and load
 *		each instance of H5C_image_entry_t in the array with 
 *		the data necessary to construct the metadata cache image.
 *		
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              8/4/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_prep_for_file_close__setup_image_entries_array(H5C_t * cache_ptr)
{
    int			i;
    int                 j;
    int32_t		entries_visited = 0;
    int32_t	 	num_entries_in_image;
    H5C_cache_entry_t * entry_ptr;
    H5C_image_entry_t * image_entries = NULL;
    herr_t		ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* sanity checks */
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->pl_len == 0);
    HDassert(cache_ptr->num_entries_in_image > 0);
    HDassert(cache_ptr->image_entries == NULL);

    /* allocate and initialize image_entries array */
    num_entries_in_image = cache_ptr->num_entries_in_image;
    image_entries = (H5C_image_entry_t *)
        H5MM_malloc(sizeof(H5C_image_entry_t) * 
                    (size_t)(num_entries_in_image + 1));

    if ( image_entries == NULL ) {

        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, \
                    "memory allocation failed for image_entries")
    }

    for ( j = 0; j <= num_entries_in_image; j++ ) {

	image_entries[j].magic            = H5C__H5C_IMAGE_ENTRY_T_MAGIC;
	image_entries[j].addr             = HADDR_UNDEF;
	image_entries[j].size             = 0;
	image_entries[j].type_id          = -1;
	image_entries[j].image_index      = -1;
	image_entries[j].lru_rank         = 0;
	image_entries[j].is_dirty         = FALSE; 
	image_entries[j].flush_dep_height = 0;
	image_entries[j].fd_parent_addr   = HADDR_UNDEF;
	image_entries[j].fd_child_count   = 0;
	image_entries[j].image_ptr        = NULL;
    }


    /* scan each entry on the hash table and populate the image_entries array */
    j = 0;
    for ( i = 0; i < H5C__HASH_TABLE_LEN; i++ )
    {
	entry_ptr = cache_ptr->index[i];

        while ( entry_ptr != NULL )
        {
	    HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);

	    /* since we have already serialized the cache, the following
             * should hold.
             */
            HDassert(entry_ptr->image_up_to_date);
            HDassert(entry_ptr->image_ptr);

            if ( entry_ptr->include_in_image ) {

		HDassert(entry_ptr->type);

	        image_entries[j].addr             = entry_ptr->addr;
	        image_entries[j].size             = entry_ptr->size;

		/* when a prefetched entry is included in the image, store
                 * its underlying type id in the image entry, not 
                 * H5AC_PREFETCHED_ENTRY_ID.
                 */
		if ( entry_ptr->type->id == H5AC_PREFETCHED_ENTRY_ID )

		    image_entries[j].type_id	  = entry_ptr->prefetch_type_id;

		else

                    image_entries[j].type_id	  = entry_ptr->type->id;

	        image_entries[j].image_index      = j;
	        image_entries[j].lru_rank         = entry_ptr->lru_rank;
	        image_entries[j].is_dirty         = entry_ptr->is_dirty;
	        image_entries[j].flush_dep_height = entry_ptr->flush_dep_height;
	        image_entries[j].fd_parent_addr   = entry_ptr->fd_parent_addr;
	        image_entries[j].fd_child_count   = entry_ptr->fd_child_count;
	        image_entries[j].image_ptr        = entry_ptr->image_ptr;

                j++;

		HDassert(j <= num_entries_in_image);
            }

            entries_visited++;

            entry_ptr = entry_ptr->ht_next;
        }
    }

    HDassert(entries_visited == cache_ptr->index_len);

    HDassert(j == num_entries_in_image);

    HDassert(image_entries[j].image_ptr == NULL);

    cache_ptr->image_entries = image_entries;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_prep_for_file_close__setup_image_entries_array() */


/*-------------------------------------------------------------------------
 *
 * Function:    H5C_prep_for_file_close__scan_entries
 *
 * Purpose:     Scan all entries in the metadata cache, and store all 
 *		entry specific data required for construction of the 
 *		metadatat cache image block and likely to be discarded
 *		or modified during the cache flush on file close.
 *
 *		In particular, make note of:
 *
 *			entry rank in LRU
 *
 *			whether the entry is dirty
 *
 *			base address of entry flush dependency parent
 *			if it exists.
 *
 *			number of flush dependency children, if any.
 *
 *		Also, determine which entries are to be included in the
 *		metadata cache image.  At present, all entries other than
 *		the superblock, the superblock extension object header and
 *		its associated chunks (if any) are included.
 *
 *		Finally, compute the size of the metadata cache image
 *		block.
 *		
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              7/21/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_prep_for_file_close__scan_entries(H5F_t *f, hid_t dxpl_id, H5C_t * cache_ptr)
{
    hbool_t		include_in_image;
    int			i;
    int                 j;
    int			entries_visited = 0;
    int			lru_rank = 1;
    int32_t		num_entries_in_image = 0;
    unsigned		nchunks = 0;
    size_t		image_len;
    size_t		entry_header_len;
    haddr_t		superblock_addr = 0; /* by definition */
    haddr_t		sb_ext_addr;
    haddr_t           * chunk_addrs = NULL;
    H5C_cache_entry_t * entry_ptr;
    herr_t		ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* sanity checks */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->sblock);
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->pl_len == 0);

    /* get addresses of superblock, superblock extension, and any chunks */
    sb_ext_addr = f->shared->sblock->ext_addr;

    HDassert(H5F_addr_defined(sb_ext_addr));

    if ( H5F__super_ext_get_num_chunks(f, dxpl_id, &nchunks) < 0 )
    
        HGOTO_ERROR(H5E_CACHE, H5E_CANTGET, FAIL, \
		    "Can't get num superblock extension chunks.")

    /* while the superblock extension can have continuation messages, and 
     * thus chunks, I am not aware of any cases in which it does.  The 
     * following assertion will let us know when we run across one -- 
     * at which point we will need test code for chunk management if we
     * don't have it already.
     */
    /* HDassert(nchunks == 0); */

    if ( nchunks > 0 ) { /* get the chunk addresses */

        /* allocate addrs array */
        chunk_addrs = (haddr_t *)
                      H5MM_malloc(sizeof(haddr_t) * (size_t)(nchunks + 1));

	if ( chunk_addrs == NULL ) {

            HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, \
                        "memory allocation failed for chunk addrs array")
        }

        /* for sanity checking, initialize all entries to HADDR_UNDEF */
	for ( i = 0; i <= (int)nchunks; i++ ) chunk_addrs[i] = HADDR_UNDEF;

        /* get chunk addresses */
	if ( H5F__super_ext_get_chunk_addrs(f, dxpl_id, nchunks, chunk_addrs)
             < 0 ) {
    
            HGOTO_ERROR(H5E_CACHE, H5E_CANTGET, FAIL, \
		    "Can't get superblock extension chunk addresses.")
	}

	/* if the first chunk address is the object header address,
         * remove it from the chunk addresses array.
         */
        if ( H5F_addr_eq(chunk_addrs[0], sb_ext_addr) ) {

            for ( i = 0; i < (int)nchunks; i++ ) {

		chunk_addrs[i] = chunk_addrs[i + 1];
            }
        }

	nchunks--;

	/* do sanity checks on chunk_addrs */
        for ( i = 0; i < (int)nchunks; i++ ) {

	    HDassert(HADDR_UNDEF != chunk_addrs[i]);
        }

	HDassert(HADDR_UNDEF == chunk_addrs[nchunks]);
    }

    /* initialize image len to the size of the metadata cache image block
     * header.
     */
    image_len        = H5C_cache_image_block_header_size();
    entry_header_len = H5C_cache_image_block_entry_header_size(f);

    /* scan each entry on the hash table */
    for ( i = 0; i < H5C__HASH_TABLE_LEN; i++ )
    {
	entry_ptr = cache_ptr->index[i];

        while ( entry_ptr != NULL )
        {
	    HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);

	    /* since we have already serialized the cache, the following
             * should hold.
             */
            HDassert(entry_ptr->image_up_to_date);
            HDassert(entry_ptr->image_ptr);

	    if ( H5F_addr_eq(entry_ptr->addr, superblock_addr) ) {

		include_in_image = FALSE;
		HDassert(entry_ptr->type->id == H5AC_SUPERBLOCK_ID);

	    } else if ( H5F_addr_eq(entry_ptr->addr, sb_ext_addr) ) {

		include_in_image = FALSE;
		HDassert(entry_ptr->type->id == H5AC_OHDR_ID);

	    } else if ( nchunks > 0 ) {

		include_in_image = TRUE;

		/* In most if not all case, nchunks will be very small --
                 * typically 0 and seldom if ever greater than 1.  As 
                 * long as this holds, the following code will be the 
                 * most efficient option.  If it ever ceases to hold,
                 * this linear search should be replaced with a binary 
                 * search on a sorted list.
                 */
		for ( j = 0; j < (int)nchunks; i++ ) {

		    if ( H5F_addr_eq(entry_ptr->addr, chunk_addrs[j]) ) {

			include_in_image = FALSE;
			HDassert(entry_ptr->type->id == H5AC_OHDR_CHK_ID);
		    }
 		}
            } else {

		include_in_image = TRUE;

            }

            entry_ptr->include_in_image = include_in_image;

            if ( include_in_image ) {

	        entry_ptr->lru_rank = -1;
                entry_ptr->image_dirty = entry_ptr->is_dirty;

                if ( entry_ptr->flush_dep_parent ) {

                    HDassert(entry_ptr->flush_dep_parent->magic == \
                             H5C__H5C_CACHE_ENTRY_T_MAGIC);
                    HDassert(entry_ptr->flush_dep_parent->is_pinned);

		    entry_ptr->fd_parent_addr = 
			entry_ptr->flush_dep_parent->addr;
	        }

	        if ( entry_ptr->flush_dep_height > 0 ) {

                    /* only interested in the number of direct flush 
                     * flush dependency children.
                     */
		    if ( ! entry_ptr->is_pinned )

		        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                    		    "encountered unpinned fd parent?!?")

                    j = (int)(entry_ptr->flush_dep_height - 1);
                    entry_ptr->fd_child_count = 
		        entry_ptr->child_flush_dep_height_rc[j];

		    HDassert(entry_ptr->fd_child_count > 0);
                }

	        if ( entry_ptr->compressed )

		    image_len += entry_header_len + entry_ptr->compressed_size;

	        else 

		    image_len += entry_header_len + entry_ptr->size;

		num_entries_in_image++;
	    }

            entries_visited++;

            entry_ptr = entry_ptr->ht_next;
        }
    }

    HDassert(entries_visited == cache_ptr->index_len);

    HDassert(entries_visited == (num_entries_in_image + 2 + (int)nchunks));

    cache_ptr->num_entries_in_image = num_entries_in_image;

    entries_visited = 0;

    /* now scan the LRU list to set the lru_rank fields of all entries
     * on the LRU.
     *
     * Note that we start with rank 1, and increment by 1 with each 
     * entry on the LRU.  
     *
     * Note that manually pinned entryies will have lru_rank -1,
     * and no flush dependency.  Putting these entries at the head of 
     * the reconstructed LRU should be appropriate.
     */

     entry_ptr = cache_ptr->LRU_head_ptr;

     while ( entry_ptr != NULL )
     {
	HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
        HDassert(entry_ptr->type != NULL);

        /* to avoid confusion, don't set lru_rank on epoch markers.
         * Note that we still increment the lru_rank, so that the holes
         * in the sequence of entries on the LRU will indicate the 
         * locations of epoch markers (if any) when we reconstruct 
         * the LRU.
         *
         * Do not set lru_rank or increment lru_rank for entries 
         * that will not be included in the cache image.
         */
	if ( entry_ptr->type->id == H5C__EPOCH_MARKER_TYPE ) {

	    lru_rank++;

	} else if ( entry_ptr->include_in_image ) {

	    entry_ptr->lru_rank = lru_rank;
            lru_rank++;
        }

        entries_visited++;
        
        entry_ptr = entry_ptr->next;
     }

    HDassert(entries_visited == cache_ptr->LRU_list_len);

    image_len += H5F_SIZEOF_CHKSUM;

    cache_ptr->image_len = image_len;

done:

    /* free chunk_addrs if allocated */
    if ( chunk_addrs != NULL )

	chunk_addrs = (haddr_t *)H5MM_xfree((void *)chunk_addrs);

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_prep_for_file_close__scan_entries() */


/*-------------------------------------------------------------------------
 * Function:    H5C_reconstruct_cache_contents()
 *
 *		Scan the image_entries array, and create a prefetched
 *		cache entry for every entry in the array.  Insert the 
 *		prefetched entries in the index and the LRU, and 
 *		reconstruct any flush dependencies.  Order the entries 
 *		in the LRU as indicated by the stored lru_ranks.
 *
 * Return:      SUCCEED on success, and FAIL on failure.
 *
 * Programmer:  John Mainzer
 *              8/14/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_reconstruct_cache_contents(H5F_t * f, hid_t dxpl_id, H5C_t * cache_ptr)
{
    int			i;
#ifndef NDEBUG
    int			j;
    H5C_image_entry_t * ie_ptr = NULL;
    H5C_cache_entry_t *	entry_ptr;
#endif /* NDEBUG */
    H5C_cache_entry_t *	pf_entry_ptr;
    herr_t 		ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert(f);
    HDassert(f->shared);
    HDassert(cache_ptr == f->shared->cache);
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->image_buffer);
    HDassert(cache_ptr->image_len > 0);
    HDassert(cache_ptr->image_entries != NULL);
    HDassert(cache_ptr->num_entries_in_image > 0);

    for ( i = 0; i < cache_ptr->num_entries_in_image; i++ ) {

	/* create the prefetched entry described by the ith
         * entry in cache_ptr->image_entrise.
         */
        pf_entry_ptr = H5C_reconstruct_cache_entry(cache_ptr, i);

	if ( NULL == pf_entry_ptr )

            HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
		        "reconstruction of cache entry failed.")


	/* Note that we make no checks on available cache space before 
         * inserting the reconstructed entry into the metadata cache.
         *
         * This is OK since the cache must be almost empty at the beginning
         * of the process, and since we check cache size at the end of the 
         * reconstruction process.
         */


	/* insert the prefetched entry in the index */
	H5C__INSERT_IN_INDEX(cache_ptr, pf_entry_ptr, FAIL)


	/* if dirty, insert the entry into the slist. */
	if ( pf_entry_ptr->is_dirty ) {

	    H5C__INSERT_ENTRY_IN_SLIST(cache_ptr, pf_entry_ptr, FAIL)
        }


        /* append the entry to the LRU */
	H5C__UPDATE_RP_FOR_INSERT_APPEND(cache_ptr, pf_entry_ptr, FAIL)


	H5C__UPDATE_STATS_FOR_PREFETCH(cache_ptr, pf_entry_ptr->is_dirty)


	/* if the prefetched entry is the child in a flush dependency
         * relationship, recreate that flush dependency.
         */
        if ( H5F_addr_defined(pf_entry_ptr->fd_parent_addr) ) {

            H5C_cache_entry_t *	parent_ptr = NULL;

	    H5C__SEARCH_INDEX(cache_ptr, pf_entry_ptr->fd_parent_addr, \
                              parent_ptr, FAIL)

	    if ( parent_ptr == NULL )

                HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
		            "fd parent not in cache?!?")

	    HDassert(parent_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
            HDassert(parent_ptr->addr == pf_entry_ptr->fd_parent_addr);
            HDassert(parent_ptr->lru_rank == -1);

	    /* Must protect parent entry to set up a flush dependency.
             * Do this now, and then uprotect when done.
             */
            H5C__UPDATE_RP_FOR_PROTECT(cache_ptr, parent_ptr, FAIL)
            parent_ptr->is_protected = TRUE;

	    
	    /* setup the flush dependency */
	    if ( H5C_create_flush_dependency(parent_ptr, pf_entry_ptr) < 0 )

		HGOTO_ERROR(H5E_CACHE, H5E_CANTDEPEND, FAIL, \
			    "Can't restore flush dependency.")


	    /* and now unprotect */
	    H5C__UPDATE_RP_FOR_UNPROTECT(cache_ptr, parent_ptr, FAIL)
	    parent_ptr->is_protected = FALSE;
        }
    } 

#ifndef NDEBUG
    /* scan the image_entries array, and verify that each entry has
     * the expected flush dependency status.
     */
    for ( i = 0; i < cache_ptr->num_entries_in_image; i++ ) {

	ie_ptr = &(cache_ptr->image_entries[i]);

        HDassert(ie_ptr->magic == H5C__H5C_IMAGE_ENTRY_T_MAGIC);
        HDassert(ie_ptr->image_index == i);

	pf_entry_ptr = NULL;

	H5C__SEARCH_INDEX(cache_ptr, ie_ptr->addr, pf_entry_ptr, FAIL);

	HDassert(pf_entry_ptr);
	HDassert(pf_entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
	HDassert(pf_entry_ptr->prefetched);
	HDassert(ie_ptr->fd_parent_addr == pf_entry_ptr->fd_parent_addr);
	HDassert(ie_ptr->lru_rank == pf_entry_ptr->lru_rank);

	if ( H5F_addr_defined(ie_ptr->fd_parent_addr) ) {

	    HDassert(pf_entry_ptr->flush_dep_parent);
            HDassert(pf_entry_ptr->flush_dep_parent->addr ==
                     pf_entry_ptr->fd_parent_addr);
        }

	HDassert(ie_ptr->fd_child_count == pf_entry_ptr->fd_child_count);

	if ( pf_entry_ptr->fd_child_count > 0 ) {

	    j = (int)(pf_entry_ptr->flush_dep_height - 1);

	    HDassert(pf_entry_ptr->fd_child_count == 
		     pf_entry_ptr->child_flush_dep_height_rc[j]);

	} else {

	    HDassert(pf_entry_ptr->flush_dep_height == 0);
	}
    }

    /* scan the LRU, and verify the expected ordering of the 
     * prefetched entries.
     */
    j = -1;
    entry_ptr = cache_ptr->LRU_head_ptr;

    while ( entry_ptr != NULL ) {

	HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
        HDassert(entry_ptr->type != NULL);

	if ( entry_ptr->prefetched ) {

	    HDassert(j <= entry_ptr->lru_rank);
	    HDassert((entry_ptr->lru_rank <= 2) || 
                     (entry_ptr->lru_rank == j + 1));

	    j = entry_ptr->lru_rank;
	}

	entry_ptr = entry_ptr->next;
    }
#endif /* NDEBUG */

    /* check to see if the cache is oversize, and evict entries as 
     * necessary to remain within limits.
     */
    if ( cache_ptr->index_size >= cache_ptr->max_cache_size ) {

	/* cache is oversized -- call H5C_make_space_in_cache() with zero
         * space needed to repair the situation if possible.
         */
        hbool_t write_permitted = FALSE;

	if ( cache_ptr->check_write_permitted != NULL ) {

	    if ( (cache_ptr->check_write_permitted)(f, &write_permitted) < 0 ) {

                HGOTO_ERROR(H5E_CACHE, H5E_CANTPROTECT, FAIL, \
                           "Can't get write_permitted")

            }
        } else {

            write_permitted = cache_ptr->write_permitted;
        }

	if ( H5C_make_space_in_cache(f, dxpl_id, 0, write_permitted) < 0 )

	    HGOTO_ERROR(H5E_CACHE, H5E_CANTPROTECT, FAIL, \
                        "H5C_make_space_in_cache failed.")
    }

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_reconstruct_cache_contents() */


/*-------------------------------------------------------------------------
 * Function:    H5C_reconstruct_cache_entry()
 *
 *		Allocate a prefetched metadata cache entry and initialize
 *		it from the indicated entry in the image_entries array.
 *
 *		Return a pointer to the newly allocated cache entry,
 *		or NULL on failure.
 *
 * Return:      Pointer to the new instance of H5C_cache_entry on success, 
 *		or NULL on failure.
 *
 * Programmer:  John Mainzer
 *              8/14/15
 *
 *-------------------------------------------------------------------------
 */

H5C_cache_entry_t *
H5C_reconstruct_cache_entry(H5C_t * cache_ptr, int i)
{
    hbool_t		file_is_rw;
    int			u;
    H5C_image_entry_t * ie_ptr = NULL;
    H5C_cache_entry_t *	pf_entry_ptr = NULL;
    H5C_cache_entry_t *	ret_value = NULL;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->image_entries != NULL);
    HDassert(cache_ptr->num_entries_in_image > 0);
    HDassert(i < cache_ptr->num_entries_in_image);

    ie_ptr = &((cache_ptr->image_entries)[i]);

    HDassert(ie_ptr->magic == H5C__H5C_IMAGE_ENTRY_T_MAGIC);
    HDassert(ie_ptr->image_index == i);
    HDassert(H5F_addr_defined(ie_ptr->addr));
    HDassert(ie_ptr->size > 0);
    HDassert(ie_ptr->image_ptr);

    file_is_rw = cache_ptr->delete_image;

    /* allocate space for the prefetched cache entry */
    pf_entry_ptr = (H5C_cache_entry_t *)H5MM_malloc(sizeof(H5C_cache_entry_t));

    if ( NULL == pf_entry_ptr )

	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, \
                        "memory allocation failed for prefetched cache entry")


    /* initialize the prefetched entry from the entry image */
    pf_entry_ptr->magic			= H5C__H5C_CACHE_ENTRY_T_MAGIC;
    pf_entry_ptr->cache_ptr 		= cache_ptr;
    pf_entry_ptr->addr			= ie_ptr->addr;
    pf_entry_ptr->size			= ie_ptr->size;
    pf_entry_ptr->compressed		= FALSE;
    pf_entry_ptr->compressed_size	= 0;
    pf_entry_ptr->image_ptr		= ie_ptr->image_ptr;
    pf_entry_ptr->image_up_to_date	= TRUE;
    pf_entry_ptr->type			= &prefetched_entry_class;
    pf_entry_ptr->tag			= H5AC__IGNORE_TAG;

    /* force dirty entries to clean if the file read only -- must do 
     * this as otherwise the cache will attempt to write them on file
     * close.  Since the file is R/O, the metadata cache image superblock
     * extension message and the cache image block will not be removed.
     * Hence no danger in this.
     */
    pf_entry_ptr->is_dirty		= ie_ptr->is_dirty && file_is_rw;
    pf_entry_ptr->dirtied		= FALSE;
    pf_entry_ptr->is_protected		= FALSE;
    pf_entry_ptr->is_read_only		= FALSE;
    pf_entry_ptr->ro_ref_count		= 0;
    pf_entry_ptr->is_pinned		= FALSE;
    pf_entry_ptr->in_slist		= FALSE;
    pf_entry_ptr->flush_marker		= FALSE;
    pf_entry_ptr->flush_me_last		= FALSE;
#ifdef H5_HAVE_PARALLEL
    pf_entry_ptr->flush_me_collectively	= FALSE;
    pf_entry_ptr->clear_on_unprotect	= FALSE;
    pf_entry_ptr->flush_immediately	= FALSE;
#endif
    pf_entry_ptr->flush_in_progress	= FALSE;
    pf_entry_ptr->destroy_in_progress	= FALSE;

    /* Initialize flush dependency height fields */
    pf_entry_ptr->flush_dep_parent	= NULL;
    for(u = 0; u < H5C__NUM_FLUSH_DEP_HEIGHTS; u++)
        pf_entry_ptr->child_flush_dep_height_rc[u] = 0;
    pf_entry_ptr->flush_dep_height	= 0;
    pf_entry_ptr->pinned_from_client	= FALSE;
    pf_entry_ptr->pinned_from_cache	= FALSE;

    /* Initialize fields supporting the hash table: */
    pf_entry_ptr->ht_next		= NULL;
    pf_entry_ptr->ht_prev		= NULL;

    /* Initialize fields supporting replacement policies: */
    pf_entry_ptr->next			= NULL;
    pf_entry_ptr->prev			= NULL;
    pf_entry_ptr->aux_next		= NULL;
    pf_entry_ptr->aux_prev		= NULL;

    /* Initialize cache image related fields */
    pf_entry_ptr->include_in_image	= FALSE;
    pf_entry_ptr->lru_rank		= ie_ptr->lru_rank;
    pf_entry_ptr->image_index		= -1;
    pf_entry_ptr->image_dirty		= FALSE;
    pf_entry_ptr->fd_parent_addr	= ie_ptr->fd_parent_addr;
    pf_entry_ptr->fd_child_count	= ie_ptr->fd_child_count;
    pf_entry_ptr->prefetched		= TRUE;
    pf_entry_ptr->prefetch_type_id	= ie_ptr->type_id;


    /* On disk image of entry is now transferred to the prefetched entry.
     * Thus set ie_ptr->image_ptr to NULL.
     */
    HDassert(pf_entry_ptr->image_ptr == ie_ptr->image_ptr);
    ie_ptr->image_ptr = NULL;


    H5C__RESET_CACHE_ENTRY_STATS(pf_entry_ptr)


    /* sanity checks */
    HDassert(pf_entry_ptr->size > 0 &&  \
             pf_entry_ptr->size < H5C_MAX_ENTRY_SIZE);
    HDassert((pf_entry_ptr->type->flags & H5C__CLASS_COMPRESSED_FLAG) == 0);


    ret_value = pf_entry_ptr;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_reconstruct_cache_entry() */


/*-------------------------------------------------------------------------
 * Function:    H5C_serialize_cache
 *
 * Purpose:	Serialize (i.e. construct an on disk image) for all entries 
 *		in the metadata cache including clean entries.  
 *
 *		Note that flush dependencies and "flush me last" flags
 *		must be observed in the serialization process.  
 *
 *		Note also that entries may be loaded, flushed, evicted,
 *		expunged, relocated, resized, or removed from the cache
 *		during this process, just as these actions may occur during
 *		a regular flush.
 *
 *		However, we are given that the cache will contain no protected
 *		entries on entry to this routine (although entries may be 
 *		briefly protected and then unprotected during the serialize 
 *		process).  
 *
 *		The objective of this routine is serialize all entries and 
 *		to force all entries into their actual locations on disk.  
 *
 *		The initial need for this routine is to settle all entries 
 *		in the cache prior to construction of the metadata cache 
 *		image so that the size of the cache image can be calculated.
 *		However, I gather that other uses for the routine are 
 *		under consideration.
 *
 * Return:      Non-negative on success/Negative on failure or if there was
 *		a request to flush all items and something was protected.
 *
 * Programmer:  John Mainzer
 *		7/22/15
 *
 * Changes:	None.
 *
 *-------------------------------------------------------------------------
 */

herr_t
H5C_serialize_cache(const H5F_t * f,
                    hid_t    dxpl_id)
{
    hbool_t		restart_scan = TRUE;
    hbool_t		restart_bucket;
    int                 i;
    unsigned		fd_height;
    H5C_t             * cache_ptr;
    H5C_cache_entry_t * entry_ptr;
    herr_t              ret_value = SUCCEED;

    FUNC_ENTER_NOAPI(FAIL)

    /* sanity checks */
    HDassert(f);
    HDassert(f->shared);

    cache_ptr = f->shared->cache;

    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);

    /* set cache_ptr->serialization_in_progress to TRUE, and back 
     * to FALSE at the end of the function.  Must maintain this flag
     * to support H5C_get_serialization_in_progress(), which is in 
     * turn required to support sanity checking in some cache 
     * clients.
     */
    HDassert(!cache_ptr->serialization_in_progress);
    cache_ptr->serialization_in_progress = TRUE;

    /* The objective here is to serialize all entries in the cache 
     * in increasing flush dependency height order.
     *
     * The basic algorithm is to scan the cache index once for each 
     * flush dependency level, serializing all entries at the current
     * level on each scan, and then incrementing the target dependency
     * level by one and repeating the process until all flush dependency
     * levels have been scaned.
     *
     * However, this algorithm is greatly complicated by the ability
     * of client serialization callbacks to perform operations on 
     * on the cache which can result in the insertion, deletion, 
     * relocation, resizing, flushing, eviction, and removal (via the
     * take ownership flag) of entries.  Changes in the flush dependency
     * structure are also possible.
     *
     * In particular, if either:
     *
     *    1) An entry other than the target entry is inserted or loaded 
     *       into the cache, or
     *
     *    2) An entry other than the target entry is relocated, or 
     *
     *    3) The flush dependency tree is altered (more specifically,
     *	     the flush dependency height of some node is altered), 
     *
     * we must restart the scan from the beginning lest we either miss 
     * serializing an entry, or we fail to serialize entries in increasing
     * flush dependency height order.
     *
     * Similarly, if the target entry is relocated, we must restart the 
     * scan of the current hash bucket, as the ht_next field of the target
     * entry may no longer point to an entry in the current bucket.
     *
     * H5C_serialize_single_entry() should recognize these situations, and
     * set restart_scan or restart_bucket to TRUE when they appear.
     *
     * Observe that either eviction or removal of entries as a result of 
     * a serialization is not a problem as long as the flush depencency 
     * tree does not change beyond the removal of a leaf.
     */

    while ( restart_scan ) 
    {
        restart_scan = FALSE;
	restart_bucket = FALSE;
        fd_height = 0;

	while ( ( fd_height <= H5C__NUM_FLUSH_DEP_HEIGHTS ) && 
                ( !restart_scan ) )
        {
            i = 0;
            while ( ( i < H5C__HASH_TABLE_LEN ) && ( !restart_scan ) ) 
            {
		entry_ptr = cache_ptr->index[i];

		while ( entry_ptr != NULL )
                {
		    HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);

		    /* skip flush me last entries for now */
		    if ( ! entry_ptr->flush_me_last ) {

		        if ( fd_height > entry_ptr->flush_dep_height ) {

			    HDassert(entry_ptr->image_up_to_date);

		        } else if ( fd_height == entry_ptr->flush_dep_height ) {

		            if ( ! entry_ptr->image_up_to_date ) {

			        /* serialize the entry */
			        if ( H5C_serialize_single_entry(f, dxpl_id,  
				          cache_ptr, entry_ptr,
                                          &restart_scan, &restart_bucket) < 0 ) 

            			    HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
					        "entry serialization failed.");
                            }
#if H5C_COLLECT_CACHE_STATS
			    if ( restart_scan )
			        cache_ptr->index_scan_restarts++;
			    else if ( restart_bucket )
			        cache_ptr->hash_bucket_scan_restarts++;
#endif /* H5C_COLLECT_CACHE_STATS */
		        } 
                    } /* if ( ! entry_ptr->flush_me_last ) */

		    if ( restart_bucket ) {

			restart_bucket = FALSE;
			entry_ptr = cache_ptr->index[i];

		    } else {

			entry_ptr = entry_ptr->ht_next;
		    }
                }

                i++;
            }
            fd_height++;
	}
    } /* while ( restart_scan ) */


    /* At this point, all entries not marked "flush me last" should
     * be serialized and have up to date images.  Scan the index 
     * again to serialize the "flush me last" entries and to verify
     * that all other entries have up to date images.
     */

    i = 0;
    while ( i < H5C__HASH_TABLE_LEN )
    {
	entry_ptr = cache_ptr->index[i];

	while ( entry_ptr != NULL )
        {
	    HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);

	    if ( entry_ptr->flush_me_last ) {

		if ( ! entry_ptr->image_up_to_date ) {

	            /* serialize the entry */
	            if ( H5C_serialize_single_entry(f, dxpl_id,  cache_ptr, 
					            entry_ptr, &restart_scan, 
						    &restart_bucket) < 0 ) {

		        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
			            "entry serialization failed.");

		    } else if ( restart_scan || restart_bucket ) {

		        HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
			"flush_me_last entry serialization triggered restart.");
		    }
                }
            } else {

	        HDassert(entry_ptr->image_up_to_date);
            }

	    entry_ptr = entry_ptr->ht_next;
        }

        i++;
    }

done:
    HDassert(cache_ptr->serialization_in_progress);
    cache_ptr->serialization_in_progress = FALSE;

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_serialize_cache() */


/*-------------------------------------------------------------------------
 *
 * Function:    H5C_serialize_single_entry
 *
 * Purpose:     Serialize the cache entry pointed to by the entry_ptr 
 *		parameter.
 *
 * Return:      Non-negative on success/Negative on failure or if there was
 *		an attempt to serialize a protected item.
 *
 * Programmer:  John Mainzer, 7/24/15
 *
 * Changes:	None
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C_serialize_single_entry(const H5F_t *f, 
                           hid_t dxpl_id, 
		           H5C_t *cache_ptr,
                           H5C_cache_entry_t *entry_ptr,
                           hbool_t *restart_scan_ptr,
                           hbool_t *restart_bucket_ptr)
{
    hbool_t		was_dirty;
    hbool_t		target_entry_relocated = FALSE;
    unsigned 		serialize_flags = H5C__SERIALIZE_NO_FLAGS_SET;
    haddr_t		new_addr = HADDR_UNDEF;
    haddr_t		old_addr = HADDR_UNDEF;
    size_t		new_len = 0;
    size_t		new_compressed_len = 0;
    herr_t		ret_value = SUCCEED;      /* Return value */

#if 0
    FUNC_ENTER_PACKAGE
#else
    FUNC_ENTER_NOAPI(FAIL)
#endif

    /* sanity checks */
    HDassert(f);
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(entry_ptr);
    HDassert(entry_ptr->magic == H5C__H5C_CACHE_ENTRY_T_MAGIC);
    HDassert(!entry_ptr->prefetched);
    HDassert(!entry_ptr->image_up_to_date);
    HDassert(!entry_ptr->is_protected);
    HDassert(!entry_ptr->flush_in_progress);
    HDassert(entry_ptr->type);
    HDassert(restart_scan_ptr);
    HDassert(*restart_scan_ptr == FALSE);
    HDassert(restart_bucket_ptr);
    HDassert(*restart_bucket_ptr == FALSE);

    /* set entry_ptr->flush_in_progress to TRUE so the the target entry
     * will not be evicted out from under us.  Must set it back to FALSE
     * when we are done.
     */
    entry_ptr->flush_in_progress = TRUE;


    /* allocate buffer for the entry image if required. */
    if ( NULL == entry_ptr->image_ptr ) {

        size_t image_size;

        if(entry_ptr->compressed)

            image_size = entry_ptr->compressed_size;

        else

            image_size = entry_ptr->size;

        HDassert(image_size > 0);

        if ( NULL == (entry_ptr->image_ptr = 
			H5MM_malloc(image_size + H5C_IMAGE_EXTRA_SPACE)) )

            HGOTO_ERROR(H5E_CACHE, H5E_CANTALLOC, FAIL, \
                        "memory allocation failed for on disk image buffer")

#if H5C_DO_MEMORY_SANITY_CHECKS
        HDmemcpy(((uint8_t *)entry_ptr->image_ptr) + image_size, \
                 H5C_IMAGE_SANITY_VALUE, H5C_IMAGE_EXTRA_SPACE);
#endif /* H5C_DO_MEMORY_SANITY_CHECKS */
    } /* end if */


    /* Serialize the entry.  Note that the entry need not be dirty. */

    /* reset cache_ptr->slist_changed so we can detect slist
     * modifications in the pre_serialize call.
     */
    cache_ptr->slist_changed = FALSE;

    /* make note of the entry's current address */
    old_addr = entry_ptr->addr;

    /* make note of whether the entry was dirty at the beginning of 
     * the serialization process 
     */
    was_dirty = entry_ptr->is_dirty;

    /* reset the counters so that we can detect insertions, loads,
     * moves, and flush dependency height changes caused by the pre_serialize
     * and serialize calls.
     */
    cache_ptr->entries_loaded_counter         = 0;
    cache_ptr->entries_inserted_counter	      = 0;
    cache_ptr->entries_relocated_counter      = 0;
    cache_ptr->entry_fd_height_change_counter = 0;

    /* Call client's pre-serialize callback, if there's one */
    if ( ( entry_ptr->type->pre_serialize != NULL ) && 
         ( (entry_ptr->type->pre_serialize)(f, dxpl_id, 
                                            (void *)entry_ptr,
                                            entry_ptr->addr, 
                                            entry_ptr->size,
					    entry_ptr->compressed_size,
                                            &new_addr, &new_len, 
                                            &new_compressed_len,
                                            &serialize_flags) < 0 ) )

        HGOTO_ERROR(H5E_CACHE, H5E_CANTFLUSH, FAIL, \
                    "unable to pre-serialize entry")

     /* set cache_ptr->slist_change_in_pre_serialize if the slist 
      * was modified.
      */
     if(cache_ptr->slist_changed)

         cache_ptr->slist_change_in_pre_serialize = TRUE;

     /* Check for any flags set in the pre-serialize callback */
     if ( serialize_flags != H5C__SERIALIZE_NO_FLAGS_SET ) {

        /* Check for unexpected flags from serialize callback */
        if(serialize_flags & ~(H5C__SERIALIZE_RESIZED_FLAG | 
                               H5C__SERIALIZE_MOVED_FLAG |
                               H5C__SERIALIZE_COMPRESSED_FLAG))

            HGOTO_ERROR(H5E_CACHE, H5E_CANTFLUSH, FAIL, \
                        "unknown serialize flag(s)")

#ifdef H5_HAVE_PARALLEL
        /* In the parallel case, resizes and moves in the serialize 
         * operation can cause problems.  If they occur, scream and die.
         *
         * At present, in the parallel case, the aux_ptr will only be 
         * set if there is more than one process.  Thus we can use this 
         * to detect the parallel case.
         *
         * This works for now, but if we start using the aux_ptr for 
         * other purposes, we will have to change this test accordingly.
         *
         * NB: While this test detects entryies that attempt
         *     to resize or move themselves during a flush
         *     in the parallel case, it will not detect an
         *     entry that dirties, resizes, and/or moves
         *     other entries during its flush.
         *
         *     From what Quincey tells me, this test is
         *     sufficient for now, as any flush routine that
         *     does the latter will also do the former.
         *
         *     If that ceases to be the case, further
         *     tests will be necessary.
         */
        if(cache_ptr->aux_ptr != NULL)

            HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                        "resize/move in serialize occured in parallel case.")
#endif /* H5_HAVE_PARALLEL */

        /* Resize the buffer if required */
        if ( ( ( ! entry_ptr->compressed ) &&
               ( serialize_flags & H5C__SERIALIZE_RESIZED_FLAG ) ) ||
             ( ( entry_ptr->compressed ) &&
               ( serialize_flags & H5C__SERIALIZE_COMPRESSED_FLAG ) ) )
        {
            size_t new_image_size;

      	    if(entry_ptr->compressed)

                new_image_size = new_compressed_len;

            else

                new_image_size = new_len;

            HDassert(new_image_size > 0);

            /* Release the current image */
            if(entry_ptr->image_ptr)

                entry_ptr->image_ptr = H5MM_xfree(entry_ptr->image_ptr);

            /* Allocate a new image buffer */
            if ( NULL == (entry_ptr->image_ptr = 
			H5MM_malloc(new_image_size + H5C_IMAGE_EXTRA_SPACE)) )

                HGOTO_ERROR(H5E_CACHE, H5E_CANTALLOC, FAIL, \
                            "memory allocation failed for on disk image buffer")

#if H5C_DO_MEMORY_SANITY_CHECKS
            HDmemcpy(((uint8_t *)entry_ptr->image_ptr) + new_image_size, 
                     H5C_IMAGE_SANITY_VALUE, H5C_IMAGE_EXTRA_SPACE);
#endif /* H5C_DO_MEMORY_SANITY_CHECKS */
        } /* end if */

	/* If required, update the entry and the cache data structures
         * for a resize.
	 */
        if ( serialize_flags & H5C__SERIALIZE_RESIZED_FLAG) {

            H5C__UPDATE_STATS_FOR_ENTRY_SIZE_CHANGE(cache_ptr, \
                                                    entry_ptr, new_len)

            /* update the hash table for the size change */
            H5C__UPDATE_INDEX_FOR_SIZE_CHANGE(cache_ptr, \
                                              entry_ptr->size, \
                                              new_len, entry_ptr, \
                                              !(entry_ptr->is_dirty));

            /* The entry can't be protected since we are
             * in the process of serializing the cache.  Thus we must
             * update the replacement policy data structures for the 
	     * size change.  The macro deals with the pinned case.
             */
            H5C__UPDATE_RP_FOR_SIZE_CHANGE(cache_ptr, entry_ptr, new_len);

            /* If the entry is dirty, it should be in the skip list.  If so
             * we must update the skip list for the size change.
             */
            if ( entry_ptr->is_dirty ) {

	        HDassert(entry_ptr->in_slist);
                H5C__UPDATE_SLIST_FOR_SIZE_CHANGE(cache_ptr, entry_ptr->size, \
                                                  new_len)
            } else {

		HDassert(!entry_ptr->in_slist);
	    }

            /* finally, update the entry for its new size */
            entry_ptr->size = new_len;
        } /* end if */


        /* If required, update the entry and the cache data structures 
         * for a move 
         */
        if ( serialize_flags & H5C__SERIALIZE_MOVED_FLAG ) {

#if H5C_DO_SANITY_CHECKS
            int64_t saved_slist_len_increase;
            int64_t saved_slist_size_increase;
#endif /* H5C_DO_SANITY_CHECKS */

	    target_entry_relocated = TRUE;

	    /* since the entry has moved, it is probably no longer in 
             * the same hash bucket.  Thus at a minimum, we must set 
             * *restart_bucket_ptr to TRUE.
             */

	    *restart_bucket_ptr = TRUE;

            if(entry_ptr->addr == old_addr) {

		/* update stats and the entries relocated counter */
                H5C__UPDATE_STATS_FOR_MOVE(cache_ptr, entry_ptr)
		cache_ptr->entries_relocated_counter++;

                /* we must update cache data structures for the 
                 * change in address.
                 */

                /* delete the entry from the hash table and the 
                 * slist (if appropriate).
                 */
                H5C__DELETE_FROM_INDEX(cache_ptr, entry_ptr)

		if ( was_dirty ) {

	            HDassert(entry_ptr->in_slist);
                    H5C__REMOVE_ENTRY_FROM_SLIST(cache_ptr, entry_ptr)

                } else {

		    HDassert(!entry_ptr->in_slist);
		}

	        /* update the entry for its new address */
                entry_ptr->addr = new_addr;

	        /* and then reinsert in the index and slist (if appropriate) */
                H5C__INSERT_IN_INDEX(cache_ptr, entry_ptr, FAIL)

		if ( entry_ptr->is_dirty ) {
#if H5C_DO_SANITY_CHECKS
	            /* save cache_ptr->slist_len_increase and 
                     * cache_ptr->slist_size_increase before the 
                     * reinsertion into the slist, and restore 
                     * them afterwards to avoid skewing our sanity
                     * checking.
                     */
                    saved_slist_len_increase = cache_ptr->slist_len_increase;
                    saved_slist_size_increase = cache_ptr->slist_size_increase;
#endif /* H5C_DO_SANITY_CHECKS */

                    H5C__INSERT_ENTRY_IN_SLIST(cache_ptr, entry_ptr, FAIL)

#if H5C_DO_SANITY_CHECKS
                    cache_ptr->slist_len_increase = saved_slist_len_increase;
                    cache_ptr->slist_size_increase = saved_slist_size_increase;
#endif /* H5C_DO_SANITY_CHECKS */
		}
            }
            else /* move is alread done for us -- just do sanity checks */

                HDassert(entry_ptr->addr == new_addr);

        } /* end if */

        if ( serialize_flags & H5C__SERIALIZE_COMPRESSED_FLAG ) {

            /* just save the new compressed entry size in 
             * entry_ptr->compressed_size.  We don't need to 
 	     * do more, as compressed size is only used for I/O.
             */
            HDassert(entry_ptr->compressed);
            entry_ptr->compressed_size = new_compressed_len;
        }
    } /* end if ( serialize_flags != H5C__SERIALIZE_NO_FLAGS_SET ) */

    /* Serialize object into buffer */
    {
        size_t image_len;

        if(entry_ptr->compressed)

            image_len = entry_ptr->compressed_size;

        else

            image_len = entry_ptr->size;

        /* reset cache_ptr->slist_changed so we can detect slist
         * modifications in the serialize call.
         */
        cache_ptr->slist_changed = FALSE;

            
        if( entry_ptr->type->serialize(f, entry_ptr->image_ptr, 
                                       image_len, (void *)entry_ptr) < 0 )

            HGOTO_ERROR(H5E_CACHE, H5E_CANTFLUSH, FAIL, \
                        "unable to serialize entry")

        /* set cache_ptr->slist_change_in_serialize if the 
         * slist was modified.
         */
        if(cache_ptr->slist_changed)

            cache_ptr->slist_change_in_serialize = TRUE;

#if H5C_DO_MEMORY_SANITY_CHECKS
        HDassert(0 == HDmemcmp(((uint8_t *)entry_ptr->image_ptr) + image_len, 
                                       H5C_IMAGE_SANITY_VALUE, H5C_IMAGE_EXTRA_SPACE));
#endif /* H5C_DO_MEMORY_SANITY_CHECKS */

        entry_ptr->image_up_to_date = TRUE;
    }

    /* reset the flush_in progress flag */
    entry_ptr->flush_in_progress = FALSE;

    /* set *restart_scan_ptr to TRUE if appropriate */
    if ( ( cache_ptr->entries_loaded_counter > 0 ) ||
         ( cache_ptr->entries_inserted_counter > 0 ) ||
         ( cache_ptr->entries_relocated_counter > 1 ) ||
         ( ( cache_ptr->entries_relocated_counter > 0 ) &&
           ( ! target_entry_relocated ) ) ||
         ( cache_ptr->entry_fd_height_change_counter > 0 ) ) {

	*restart_scan_ptr = TRUE;
    }

done:

    HDassert( ( ret_value != SUCCEED ) || 
              ( ! entry_ptr->flush_in_progress ) );

    HDassert( ( ret_value != SUCCEED ) || 
              ( entry_ptr->image_up_to_date ) );

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_serialize_single_entry() */



/*-------------------------------------------------------------------------
 *
 * Function:    H5C__write_cache_image_superblock_msg
 *
 * Purpose:     Write the cache image superblock extension message, 
 *		creating if specified.
 *
 *		In general, the size and location of the cache image block
 *		will be unknow at the time that the cache image superblock
 *		message is created.  A subsequent call to this routine will
 *		be used to write the correct data.
 *
 * Return:      Non-negative on success/Negative on failure.
 *
 * Programmer:  John Mainzer, 7/4/15
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5C__write_cache_image_superblock_msg(H5F_t *f, hid_t dxpl_id, hbool_t create)
{
    H5C_t *		cache_ptr = NULL;
    H5O_mdci_msg_t 	mdci_msg;	/* metadata cache image message */
					/* to insert in the superblock  */
					/* extension.			*/
    unsigned	   	mesg_flags = 
			(H5O_MSG_FLAG_FAIL_IF_UNKNOWN_AND_OPEN_FOR_WRITE | 
		 	H5O_MSG_FLAG_FAIL_IF_UNKNOWN_ALWAYS);
    herr_t              ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* sanity checks */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->cache);

    cache_ptr = f->shared->cache;

    HDassert( cache_ptr );
    HDassert( cache_ptr->magic == H5C__H5C_T_MAGIC );
    HDassert( cache_ptr->close_warning_received );

    /* Write data into the metadata cache image superblock extension message.
     * Note that this data will be bogus when we first create the message.
     * We will overwrite this data later in a second call to this function.
     */
    mdci_msg.addr = cache_ptr->image_addr;
    mdci_msg.size = cache_ptr->image_len;

    /* Write metadata cache image message to superblock extension */
    if ( H5F_super_ext_write_msg(f, dxpl_id, &mdci_msg, 
                                 H5O_MDCI_MSG_ID, create, mesg_flags) < 0 )

        HGOTO_ERROR(H5E_CACHE, H5E_WRITEERROR, FAIL, \
	    "can't write metadata cache image message to superblock extension")

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C__write_cache_image_superblock_msg() */

