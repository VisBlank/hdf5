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

/******************/
/* Local Typedefs */
/******************/


/********************/
/* Local Prototypes */
/********************/

static size_t H5C_cache_image_block_entry_header_size(H5F_t * f);

static size_t H5C_cache_image_block_header_size(void);

static herr_t H5C_prep_for_file_close__scan_entries(H5F_t *f, 
    H5C_t * cache_ptr);

static herr_t H5C_serialize_cache(const H5F_t * f, hid_t dxpl_id);

static herr_t H5C_serialize_single_entry(const H5F_t *f, hid_t dxpl_id, 
    H5C_t *cache_ptr, H5C_cache_entry_t *entry_ptr, hbool_t *restart_scan_ptr,
    hbool_t *restart_bucket_ptr);

static herr_t H5C__write_cache_image_superblock_msg(H5F_t *f, hid_t dxpl_id, 
    hbool_t create);

/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/


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
 * Return:      Non-negative on success/Negative on failure
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
    herr_t		ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert( f );
    HDassert( f->shared );

    cache_ptr = f->shared->cache;

    HDassert( cache_ptr );
    HDassert( cache_ptr->magic == H5C__H5C_T_MAGIC );

    cache_ptr->image_addr   = addr,
    cache_ptr->image_len    = len;
    cache_ptr->load_image   = TRUE;
    cache_ptr->delete_image = rw;

done:

    FUNC_LEAVE_NOAPI(ret_value)

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
	    if ( H5C_prep_for_file_close__scan_entries(f, cache_ptr) < 0 )

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
#if 1 /* this code triggers the bug */
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
#endif
		/* update the metadata cache image superblock extension 
                 * message with the new cache image block base address and 
                 * length.
                 *
                 * to simplify testing, do this only if the 
                 * H5C_CI__GEN_MDC_IMAGE_BLK bit is set in 
                 * cache_ptr->image_ctl.flags.
                 */
                if ( cache_ptr->image_ctl.flags & H5C_CI__GEN_MDC_IMAGE_BLK ) {

                    HDassert(cache_ptr->image_ctl.flags & \
                             H5C_CI__GEN_MDCI_SBE_MESG);

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
	        if ( H5C_prep_for_file_close__scan_entries(f, cache_ptr) < 0 )

		    HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                        "H5C_prep_for_file_close__scan_entries failed (2).")

	    } while ( ( i < 3 ) && ( old_image_len != cache_ptr->image_len ) );

	    if ( old_image_len != cache_ptr->image_len )

		HGOTO_ERROR(H5E_CACHE, H5E_SYSTEM, FAIL, \
                            "image len failed to converge.")

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
 * Function:    H5C_load_cache_image
 *
 * Purpose:     Read the cache image superblock extension message and
 *		delete it.
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

	/* this shouldn't be necessary, but try marking the superblock dirty */
	if ( H5F_super_dirty(f) < 0 )

	    HGOTO_ERROR(H5E_CACHE, H5E_CANTMARKDIRTY, FAIL, \
                        "can't mark superblock dirty")
    }

    /* load metadata cache image */

    /* decode metadata cache image */

    if ( cache_ptr->delete_image ) { /* free metadata cache image */

    }

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_load_cache_image() */


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
 *		In passing, compute the size of the metadata cache image
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
H5C_prep_for_file_close__scan_entries(H5F_t *f, H5C_t * cache_ptr)
{
    int			i;
    int                 j;
    int			entries_visited = 0;
    int			lru_rank = 1;
    size_t		image_len;
    size_t		entry_header_len;
    H5C_cache_entry_t * entry_ptr;
    herr_t		ret_value = SUCCEED;      /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* sanity checks */
    HDassert(cache_ptr);
    HDassert(cache_ptr->magic == H5C__H5C_T_MAGIC);
    HDassert(cache_ptr->close_warning_received);
    HDassert(cache_ptr->pl_len == 0);

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

	    entry_ptr->lru_rank = -1;
            entry_ptr->image_dirty = entry_ptr->is_dirty;

            if ( entry_ptr->flush_dep_parent ) {

                HDassert(entry_ptr->flush_dep_parent->magic == \
                         H5C__H5C_CACHE_ENTRY_T_MAGIC);
                HDassert(entry_ptr->flush_dep_parent->is_pinned);

		entry_ptr->fd_parent_addr = entry_ptr->flush_dep_parent->addr;
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

            entries_visited++;

            entry_ptr = entry_ptr->ht_next;
        }
    }

    HDassert(entries_visited == cache_ptr->index_len);

    entries_visited = 0;

    /* now scan the LRU list to set the lru_rank fields of all entries
     * on the LRU.
     *
     * Note that we start with rank 1, and increment by 1 with each 
     * entry on the LRU.  
     *
     * Note that manually pinned entryies will have lru_rank -1,
     * and no flush dependency.  Putting these entries at the head of 
     * the reconstructed LRU should appropriate.
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
         */
	if ( entry_ptr->type->id == H5C__EPOCH_MARKER_TYPE ) {

	    lru_rank++;

	} else {

	    entry_ptr->lru_rank = lru_rank;
            lru_rank++;
        }

        entries_visited++;
        
        entry_ptr = entry_ptr->next;
     }

    HDassert(entries_visited == cache_ptr->LRU_list_len);

    cache_ptr->image_len = image_len;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5C_prep_for_file_close__scan_entries() */


/*-------------------------------------------------------------------------
 * Function:    H5C_serialize_cache
 *
 * Purpose:	Serialize (i.e. construct an on disk image) for all entries 
 *		in the metadata cache including clean entries.  
 *
 *		Note that flush dependencies must be observed in the 
 *		serialization process.  
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
    int                 j;
    unsigned		fd_height;
    unsigned		max_observed_fd_height;
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
    }

done:

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

