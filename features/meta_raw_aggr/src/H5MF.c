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
 * Created:             H5MF.c
 *                      Jul 11 1997
 *                      Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:             File memory management functions.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#define H5F_PACKAGE		/*suppress error about including H5Fpkg	  */
#define H5MF_PACKAGE		/*suppress error about including H5MFpkg  */


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fpkg.h"             /* File access				*/
#include "H5MFpkg.h"		/* File memory management		*/
#include "H5Vprivate.h"		/* Vectors and arrays 			*/


/****************/
/* Local Macros */
/****************/

#define H5MF_FSPACE_SHRINK      80              /* Percent of "normal" size to shrink serialized free space size */
#define H5MF_FSPACE_EXPAND      120             /* Percent of "normal" size to expand serialized free space size */

/******************/
/* Local Typedefs */
/******************/

/* Enum for kind of free space section+aggregator merging allowed for a file */
typedef enum {
    H5MF_AGGR_MERGE_SEPARATE,           /* Everything in separate free list */
    H5MF_AGGR_MERGE_DICHOTOMY,          /* Metadata in one free list and raw data in another */
    H5MF_AGGR_MERGE_TOGETHER            /* Metadata & raw data in one free list */
} H5MF_aggr_merge_t;

/* User data for section info iterator callback for iterating over free space sections */
typedef struct {
    H5F_sect_info_t *sects;     /* section info to be retrieved */
    size_t sect_count;          /* # of sections requested */
    size_t sect_idx;            /* the current count of sections */
} H5MF_sect_iter_ud_t;


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/

/* Allocator routines */
static haddr_t H5MF_alloc_pagefs(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, hsize_t size);

/* "File closing" routines */
static herr_t H5MF_close_aggrfs(H5F_t *f, hid_t dxpl_id);
static herr_t H5MF_close_pagefs(H5F_t *f, hid_t dxpl_id);
static herr_t H5MF_close_shrink_eoa(H5F_t *f, hid_t dxpl_id);

/* General routines */
static herr_t H5MF_set_last_small(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, haddr_t addr, hsize_t size, hbool_t xfree);
static herr_t H5MF_get_free_sects(H5F_t *f, hid_t dxpl_id, H5FS_t *fspace, H5MF_sect_iter_ud_t *sect_udata, size_t *nums);

/* Free-space type manager routines */
static herr_t H5MF_create_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type);
static herr_t H5MF_free_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type);
static herr_t H5MF_realloc_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type, haddr_t *fsaddr, hbool_t *update);
static herr_t H5MF_close_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type);
static herr_t H5MF_delete_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type);


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
 * Function:    H5MF_init_merge_flags
 *
 * Purpose:     Initialize the free space section+aggregator merge flags
 *              for the file.
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:  Quincey Koziol
 *              Friday, February  1, 2008
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5MF_init_merge_flags(H5F_t *f)
{
    H5MF_aggr_merge_t mapping_type;     /* Type of free list mapping */
    H5FD_mem_t type;                    /* Memory type for iteration */
    hbool_t all_same;                   /* Whether all the types map to the same value */
    herr_t ret_value = SUCCEED;        	/* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* check args */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->lf);

    /* Iterate over all the free space types to determine if sections of that type
     *  can merge with the metadata or small 'raw' data aggregator
     */
    all_same = TRUE;
    for(type = H5FD_MEM_DEFAULT; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type))
        /* Check for any different type mappings */
        if(f->shared->fs.type_map[type] != f->shared->fs.type_map[H5FD_MEM_DEFAULT]) {
            all_same = FALSE;
            break;
        } /* end if */

    /* Check for all allocation types mapping to the same free list type */
    if(all_same) {
        if(f->shared->fs.type_map[H5FD_MEM_DEFAULT] == H5FD_MEM_DEFAULT)
            mapping_type = H5MF_AGGR_MERGE_SEPARATE;
        else
            mapping_type = H5MF_AGGR_MERGE_TOGETHER;
    } /* end if */
    else {
        /* Check for raw data mapping into same list as metadata */
        if(f->shared->fs.type_map[H5FD_MEM_DRAW] == f->shared->fs.type_map[H5FD_MEM_SUPER])
            mapping_type = H5MF_AGGR_MERGE_SEPARATE;
        else {
            hbool_t all_metadata_same;              /* Whether all metadata go in same free list */

            /* One or more allocation type don't map to the same free list type */
            /* Check if all the metadata allocation types map to the same type */
            all_metadata_same = TRUE;
            for(type = H5FD_MEM_SUPER; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type))
                /* Skip checking raw data free list mapping */
                /* (global heap is treated as raw data) */
                if(type != H5FD_MEM_DRAW && type != H5FD_MEM_GHEAP) {
                    /* Check for any different type mappings */
                    if(f->shared->fs.type_map[type] != f->shared->fs.type_map[H5FD_MEM_SUPER]) {
                        all_metadata_same = FALSE;
                        break;
                    } /* end if */
                } /* end if */

            /* Check for all metadata on same free list */
            if(all_metadata_same)
                mapping_type = H5MF_AGGR_MERGE_DICHOTOMY;
            else
                mapping_type = H5MF_AGGR_MERGE_SEPARATE;
        } /* end else */
    } /* end else */

    /* Based on mapping type, initialize merging flags for each free list type */
    switch(mapping_type) {
        case H5MF_AGGR_MERGE_SEPARATE:
            /* Don't merge any metadata together */
            HDmemset(f->shared->fs.aggr_merge, 0, sizeof(f->shared->fs.aggr_merge));

            /* Check if merging raw data should be allowed */
            /* (treat global heaps as raw data) */
            if(H5FD_MEM_DRAW == f->shared->fs.type_map[H5FD_MEM_DRAW] ||
                    H5FD_MEM_DEFAULT == f->shared->fs.type_map[H5FD_MEM_DRAW]) {
                f->shared->fs.aggr_merge[H5FD_MEM_DRAW] = H5F_FS_MERGE_RAWDATA;
                f->shared->fs.aggr_merge[H5FD_MEM_GHEAP] = H5F_FS_MERGE_RAWDATA;
	    } /* end if */
            break;

        case H5MF_AGGR_MERGE_DICHOTOMY:
            /* Merge all metadata together (but not raw data) */
            HDmemset(f->shared->fs.aggr_merge, H5F_FS_MERGE_METADATA, sizeof(f->shared->fs.aggr_merge));

            /* Allow merging raw data allocations together */
            /* (treat global heaps as raw data) */
            f->shared->fs.aggr_merge[H5FD_MEM_DRAW] = H5F_FS_MERGE_RAWDATA;
            f->shared->fs.aggr_merge[H5FD_MEM_GHEAP] = H5F_FS_MERGE_RAWDATA;
            break;

        case H5MF_AGGR_MERGE_TOGETHER:
            /* Merge all allocation types together */
            HDmemset(f->shared->fs.aggr_merge, (H5F_FS_MERGE_METADATA | H5F_FS_MERGE_RAWDATA), sizeof(f->shared->fs.aggr_merge));
            break;

        default:
            HGOTO_ERROR(H5E_RESOURCE, H5E_BADVALUE, FAIL, "invalid mapping type")
    } /* end switch */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_init_merge_flags() */


/*-------------------------------------------------------------------------
 * Function:	H5MF_open_fstype
 *
 * Purpose:	Open an existing free space manager of TYPE for file by
 *		creating a free-space structure.
 *		Note that TYPE can be H5F_mem_page_t or H5FD_mem_t enum types.
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Jan  8 2008
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5MF_open_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type)
{
    const H5FS_section_class_t *classes[] = { /* Free space section classes implemented for file */
        H5MF_FSPACE_SECT_CLS_SIMPLE,
        H5MF_FSPACE_SECT_CLS_SMALL,
	H5MF_FSPACE_SECT_CLS_LARGE };
    hsize_t alignment;			/* Alignment to use */
    hsize_t threshold;			/* Threshold to use */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(type != H5FD_MEM_NOLIST);
    if(H5F_PAGED_AGGR(f))
	HDassert((H5F_mem_page_t)type < H5F_MEM_PAGE_NTYPES);
    HDassert(f->shared);
    HDassert(H5F_addr_defined(f->shared->fs.man_addr[type]));
    HDassert(f->shared->fs.man_state[type] == H5F_FS_STATE_CLOSED);

    /* Set up the aligment and threshold to use depending on the manager type */
    if(H5F_PAGED_AGGR(f)) {
	alignment = ((H5F_mem_page_t)type == H5F_MEM_PAGE_GENERIC) ? f->shared->fs.page_size : (hsize_t)H5F_ALIGN_DEF;
	threshold = H5F_ALIGN_THRHD_DEF;
    } /* end if */
    else {
	alignment = f->shared->alignment;
	threshold = f->shared->threshold;
    } /* end else */

    /* Open an existing free space structure for the file */
    if(NULL == (f->shared->fs.man[type] = H5FS_open(f, dxpl_id, f->shared->fs.man_addr[type],
	    NELMTS(classes), classes, f, alignment, threshold)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, FAIL, "can't initialize free space info")

    /* Set the state for the free space manager to "open", if it is now */
    if(f->shared->fs.man[type])
        f->shared->fs.man_state[type] = H5F_FS_STATE_OPEN;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_open_fstype() */


/*-------------------------------------------------------------------------
 * Function:	H5MF_create_fstype
 *
 * Purpose:	Create free space manager of TYPE for the file by creating
 *		a free-space structure
 *		Note that TYPE can be H5F_mem_page_t or H5FD_mem_t enum types.
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Jan  8 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_create_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type)
{
    const H5FS_section_class_t *classes[] = { /* Free space section classes implemented for file */
        H5MF_FSPACE_SECT_CLS_SIMPLE,
        H5MF_FSPACE_SECT_CLS_SMALL,
	H5MF_FSPACE_SECT_CLS_LARGE };
    H5FS_create_t fs_create; 		/* Free space creation parameters */
    hsize_t alignment;			/* Alignment to use */
    hsize_t threshold;			/* Threshold to use */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(type != H5FD_MEM_NOLIST);
    if(H5F_PAGED_AGGR(f))
	HDassert((H5F_mem_page_t)type < H5F_MEM_PAGE_NTYPES);
    HDassert(f->shared);
    HDassert(!H5F_addr_defined(f->shared->fs.man_addr[type]));
    HDassert(f->shared->fs.man_state[type] == H5F_FS_STATE_CLOSED);

    /* Set the free space creation parameters */
    fs_create.client = H5FS_CLIENT_FILE_ID;
    fs_create.shrink_percent = H5MF_FSPACE_SHRINK;
    fs_create.expand_percent = H5MF_FSPACE_EXPAND;
    fs_create.max_sect_addr = 1 + H5V_log2_gen((uint64_t)f->shared->maxaddr);
    fs_create.max_sect_size = f->shared->maxaddr;

    /* Set up alignment and threshold to use depending on TYPE */
    if(H5F_PAGED_AGGR(f)) {
	alignment = ((H5F_mem_page_t)type == H5F_MEM_PAGE_GENERIC) ? f->shared->fs.page_size : (hsize_t)H5F_ALIGN_DEF;
	threshold = H5F_ALIGN_THRHD_DEF;
    } /* end if */
    else {
	alignment = f->shared->alignment;
	threshold = f->shared->threshold;
    } /* end else */

    if(NULL == (f->shared->fs.man[type] = H5FS_create(f, dxpl_id, NULL,
	    &fs_create, NELMTS(classes), classes, f, alignment, threshold)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, FAIL, "can't initialize free space info")


    /* Set the state for the free space manager to "open", if it is now */
    if(f->shared->fs.man[type])
        f->shared->fs.man_state[type] = H5F_FS_STATE_OPEN;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_create_fstype() */


/*-------------------------------------------------------------------------
 * Function:	H5MF_start_fstype
 *
 * Purpose:	Open or create a free space manager of a given TYPE.
 *		Note that TYPE can be H5F_mem_page_t or H5FD_mem_t enum types.
 *
 * Return:	Success:	non-negative
 *		Failure:	negative
 *
 * Programmer:	Quincey Koziol
 *		koziol@hdfgroup.org
 *		Jan  8 2008
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5MF_start_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(f->shared);
    HDassert(type != H5FD_MEM_NOLIST);
    if(H5F_PAGED_AGGR(f))
	HDassert((H5F_mem_page_t)type < H5F_MEM_PAGE_NTYPES);

    /* Check if the free space manager exists already */
    if(H5F_addr_defined(f->shared->fs.man_addr[type])) {
        /* Open existing free space manager */
        if(H5MF_open_fstype(f, dxpl_id, type) < 0)
            HGOTO_ERROR(H5E_RESOURCE, H5E_CANTOPENOBJ, FAIL, "can't initialize file free space")
    } /* end if */
    else {
        /* Create new free space manager */
        if(H5MF_create_fstype(f, dxpl_id, type) < 0)
            HGOTO_ERROR(H5E_RESOURCE, H5E_CANTCREATE, FAIL, "can't initialize file free space")
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_start_fstype() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_realloc_fstype
 *
 * Purpose:     Re-allocate data structures for the free-space manager
 *		as specified by TYPE.
 *		Note that TYPE can be H5F_mem_page_t or H5FD_mem_t enum types.
 *
 * Return:      Success:        non-negative
 *              Failure:        negative
 *
 * Programmer: 	Vailin Choi; April 2013
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_realloc_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type, haddr_t *fsaddr, hbool_t *update)
{
    herr_t ret_value = SUCCEED; 	/* Return value */

    FUNC_ENTER_NOAPI_NOINIT

#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */

    /* check args */
    HDassert(f);
    HDassert(fsaddr);
    HDassert(update);
    if(H5F_PAGED_AGGR(f))
	HDassert((H5F_mem_page_t)type < H5F_MEM_PAGE_NTYPES);

    /* Check for active free space manager of this type */
    if(f->shared->fs.man[type]) {
        H5FS_stat_t	fs_stat;		/* Information for free-space manager */

	/* Query free space manager info for this type */
	if(H5FS_stat_info(f, f->shared->fs.man[type], &fs_stat) < 0)
	    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't get free-space info")

	/* Are there sections to persist? */
	if(fs_stat.serial_sect_count) {
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Allocating free-space manager header and section info header\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */
	    /* Allocate space for free-space manager header */
	    if(H5FS_alloc_hdr(f, f->shared->fs.man[type], &f->shared->fs.man_addr[type], dxpl_id) < 0)
		HGOTO_ERROR(H5E_FSPACE, H5E_NOSPACE, FAIL, "can't allocated free-space header")

	    /* Allocate space for free-space maanger section info header */
	    if(H5FS_alloc_sect(f, f->shared->fs.man[type], dxpl_id) < 0)
		HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "can't allocate free-space section info")

	    HDassert(f->shared->fs.man_addr[type]);

	    *fsaddr = f->shared->fs.man_addr[type];
	    *update = TRUE;
	} /* end if */
    } /* end if */
    else
        if(H5F_addr_defined(f->shared->fs.man_addr[type])) {
            *fsaddr = f->shared->fs.man_addr[type];
            *update = TRUE;
        } /* end else-if */

done:
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_realloc_fstype() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_delete_fstype
 *
 * Purpose:     Delete the free-space manager as specified by TYPE.
 *		Note that TYPE can be H5F_mem_page_t or H5FD_mem_t enum types.
 *
 * Return:      Success:        non-negative
 *              Failure:        negative
 *
 * Programmer: 	Vailin Choi; April 2013
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_delete_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type)
{
    herr_t ret_value = SUCCEED;	/* Return value */
    haddr_t tmp_fs_addr;       	/* Temporary holder for free space manager address */

    FUNC_ENTER_NOAPI_NOINIT

    /* check args */
    HDassert(f);
    if(H5F_PAGED_AGGR(f))
	HDassert((H5F_mem_page_t)type < H5F_MEM_PAGE_NTYPES);
    HDassert(H5F_addr_defined(f->shared->fs.man_addr[type]));

    /* Put address into temporary variable and reset it */
    /* (Avoids loopback in file space freeing routine) */
    tmp_fs_addr = f->shared->fs.man_addr[type];
    f->shared->fs.man_addr[type] = HADDR_UNDEF;

    /* Shift to "deleting" state, to make certain we don't track any
     *  file space freed as a result of deleting the free space manager.
     */
    f->shared->fs.man_state[type] = H5F_FS_STATE_DELETING;

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Before deleting free space manager\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG_MORE */

    /* Delete free space manager for this type */
    if(H5FS_delete(f, dxpl_id, tmp_fs_addr) < 0)
	HGOTO_ERROR(H5E_FSPACE, H5E_CANTFREE, FAIL, "can't delete free space manager")

    /* Shift [back] to closed state */
    HDassert(f->shared->fs.man_state[type] == H5F_FS_STATE_DELETING);
    f->shared->fs.man_state[type] = H5F_FS_STATE_CLOSED;

    /* Sanity check that the free space manager for this type wasn't started up again */
    HDassert(!H5F_addr_defined(f->shared->fs.man_addr[type]));

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_delete_fstype() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_free_fstype
 *
 * Purpose:     Free the header and section info header for the free-space manager
 *		as specified by TYPE.
 *		Note that TYPE can be H5F_mem_page_t or H5FD_mem_t enum types.
 *
 * Return:      Success:        non-negative
 *              Failure:        negative
 *
 * Programmer: Vailin Choi; April 2013
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_free_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type)
{
    H5FS_stat_t	fs_stat;		/* Information for free-space manager */
    herr_t ret_value = SUCCEED; 	/* Return value */

    FUNC_ENTER_NOAPI_NOINIT
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */

    /* check args */
    HDassert(f);
    if(H5F_PAGED_AGGR(f))
	HDassert((H5F_mem_page_t)type < H5F_MEM_PAGE_NTYPES);
    HDassert(f->shared->fs.man[type]);

    /* Switch to "about to be deleted" state */
    f->shared->fs.man_state[type] = H5F_FS_STATE_DELETING;

    /* Query the free space manager's information */
    if(H5FS_stat_info(f, f->shared->fs.man[type], &fs_stat) < 0)
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "can't get free-space info")

    /* Check if the free space manager has space in the file */
    if(H5F_addr_defined(fs_stat.addr) || H5F_addr_defined(fs_stat.sect_addr)) {
	/* Free the free space manager in the file */
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Free the space for the free-space manager header and section info header\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */
	if(H5FS_free(f, f->shared->fs.man[type], dxpl_id) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTRELEASE, FAIL, "can't release free-space headers")
	f->shared->fs.man_addr[type] = HADDR_UNDEF;
    } /* end if */

done:
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_free_fstype() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_close_fstype
 *
 * Purpose:     Close the free space manager of TYPE for file
 *		Note that TYPE can be H5F_mem_page_t or H5FD_mem_t enum types.
 *
 * Return:      Success:        non-negative
 *              Failure:        negative
 *
 * Programmer: Vailin Choi; July 1st, 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_close_fstype(H5F_t *f, hid_t dxpl_id, H5FD_mem_t type)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /*
     * Check arguments.
     */
    HDassert(f);
    if(H5F_PAGED_AGGR(f))
	HDassert((H5F_mem_page_t)type < H5F_MEM_PAGE_NTYPES);
    HDassert(f->shared);
    HDassert(f->shared->fs.man[type]);
    HDassert(f->shared->fs.man_state[type] != H5F_FS_STATE_CLOSED);

    /* Close an existing free space structure for the file */
    if(H5FS_close(f, dxpl_id, f->shared->fs.man[type]) < 0)
        HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't release free space info")
    f->shared->fs.man[type] = NULL;
    f->shared->fs.man_state[type] = H5F_FS_STATE_CLOSED;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_close_fstype() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_add_sect
 *
 * Purpose:	To add a section to the specified free-space manager.
 *
 * Return:      Success:        non-negative
 *              Failure:        negative
 *
 * Programmer:  Vailin Choi; April 2013
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5MF_add_sect(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, H5FS_t *fspace, H5MF_free_section_t *node)
{
    H5MF_sect_ud_t udata;		/* User data for callback */
    herr_t ret_value = SUCCEED; 	/* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert(f);
    HDassert(fspace);
    HDassert(node);

    /* Construct user data for callbacks */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.alloc_type = alloc_type;
    udata.allow_sect_absorb = TRUE;
    udata.allow_eoa_shrink_only = FALSE;
    udata.allow_small_shrink = FALSE;

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: adding node, node->sect_info.addr = %a, node->sect_info.size = %Hu\n", FUNC, node->sect_info.addr, node->sect_info.size);
#endif /* H5MF_ALLOC_DEBUG_MORE */
    /* Add the section */
    if(H5FS_sect_add(f, dxpl_id, fspace, (H5FS_section_info_t *)node, H5FS_ADD_RETURNED_SPACE, &udata) < 0)
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINSERT, FAIL, "can't re-add section to file free space")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_add_sect() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_find_sect
 *
 * Purpose:	To find a section from the specified free-space manager to fulfill the request.
 *		If found, re-add the left-over space back to the manager.
 *
 * Return:	TRUE if a section is found to fulfill the request
 *		FALSE if not
 *
 * Programmer:  Vailin Choi; April 2013
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5MF_find_sect(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, hsize_t size, H5FS_t *fspace, haddr_t *addr)
{
    H5MF_free_section_t *node;  /* Free space section pointer */
    htri_t ret_value;      	/* Whether an existing free list node was found */

    FUNC_ENTER_NOAPI(FAIL)

    HDassert(f);
    HDassert(fspace);

    /* Try to get a section from the free space manager */
    if((ret_value = H5FS_sect_find(f, dxpl_id, fspace, size, (H5FS_section_info_t **)&node)) < 0)
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, FAIL, "error locating free space in file")

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: section found = %t\n", FUNC, ret_value);
#endif /* H5MF_ALLOC_DEBUG_MORE */

    /* Check for actually finding section */
    if(ret_value) {
	/* Sanity check */
	HDassert(node);

	/* Retrieve return value */
	if(addr)
	    *addr = node->sect_info.addr;

	/* Check for eliminating the section */
	if(node->sect_info.size == size) {
#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: freeing node\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG_MORE */

	    /* Free section node */
	    if(H5MF_sect_free((H5FS_section_info_t *)node) < 0)
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTRELEASE, FAIL, "can't free simple section node")
	} /* end if */
        else {
	    /* Adjust information for section */
	    node->sect_info.addr += size;
	    node->sect_info.size -= size;

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: re-adding node, node->sect_info.size = %Hu\n", FUNC, node->sect_info.size);
#endif /* H5MF_ALLOC_DEBUG_MORE */

	    /* Re-add the section to the free-space manager */
	    if(H5MF_add_sect(f, alloc_type, dxpl_id, fspace, node) < 0)
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINSERT, FAIL, "can't re-add section to file free space")
	} /* end else */
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_find_sect() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_alloc
 *
 * Purpose:     Allocate SIZE bytes of file memory and return the relative
 *		address where that contiguous chunk of file memory exists.
 *		The TYPE argument describes the purpose for which the storage
 *		is being requested.
 *
 * Return:      Success:        The file address of new chunk.
 *              Failure:        HADDR_UNDEF
 *
 * Programmer:  Robb Matzke
 *              matzke@llnl.gov
 *              Jul 11 1997
 *
 *-------------------------------------------------------------------------
 */
haddr_t
H5MF_alloc(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, hsize_t size)
{
    H5FD_mem_t  fs_type;                /* Free space type (mapped from allocation type) */
    haddr_t ret_value = HADDR_UNDEF;    /* Return value */

    FUNC_ENTER_NOAPI(HADDR_UNDEF)
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: alloc_type = %u, size = %Hu\n", FUNC, (unsigned)alloc_type, size);
#endif /* H5MF_ALLOC_DEBUG */

    /* check arguments */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->lf);
    HDassert(size > 0);

    /* Get free-space manager type from allocation type */
    if(H5F_PAGED_AGGR(f))
	fs_type = (H5FD_mem_t)H5MF_ALLOC_TO_FS_PAGE_TYPE(f, alloc_type, size);
    else
	fs_type = H5MF_ALLOC_TO_FS_AGGR_TYPE(f, alloc_type);

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Check 1.0\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG_MORE */
    /* Check if the free space manager for the file has been initialized */
    if(!f->shared->fs.man[fs_type] && H5F_addr_defined(f->shared->fs.man_addr[fs_type])) {
	/* Open the free-space manager */
	if(H5MF_open_fstype(f, dxpl_id, fs_type) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTOPENOBJ, HADDR_UNDEF, "can't initialize file free space")
        HDassert(f->shared->fs.man[fs_type]);
    } /* end if */

    /* Search for large enough space in the free space manager */
    if(f->shared->fs.man[fs_type])
        if(H5MF_find_sect(f, alloc_type, dxpl_id, size, f->shared->fs.man[fs_type], &ret_value) < 0)
            HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "error locating find a node")

    /* If no space is found from the free-space manager, continue further action */
    if(!H5F_addr_defined(ret_value)) {
#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Check 2.0\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG_MORE */
        if(f->shared->fs.strategy == H5F_FSPACE_STRATEGY_PAGE) {
            if(f->shared->fs.page_size) { /* If paged aggregation, continue further action */
                if(HADDR_UNDEF == (ret_value = H5MF_alloc_pagefs(f, alloc_type, dxpl_id, size)))
                    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "allocation failed from paged aggregation")
            } /* end if */
            else { /* If paged aggregation is disabled, allocate from VFD */
                 if(HADDR_UNDEF == (ret_value = H5MF_vfd_alloc(f, dxpl_id, alloc_type, size, TRUE)))
                    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "allocation failed from vfd")
            } /* end else */
        } /* end if */
        else { /* For non-paged aggregation, continue further action */
            if(HADDR_UNDEF == (ret_value = H5MF_aggr_vfd_alloc(f, alloc_type, dxpl_id, size)))
                HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "allocation failed from aggr/vfd")
        } /* end else */
    } /* end if */
    HDassert(H5F_addr_defined(ret_value));
#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Check 3.0\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG_MORE */

    /* For paged aggregation, determine the section at EOF is small meta/raw/large */
    if(H5F_PAGED_AGGR(f))
	if(H5MF_set_last_small(f, alloc_type, dxpl_id, ret_value, size, FALSE) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSET, HADDR_UNDEF, "can't determine track_last_small")

done:
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving: ret_value = %a, size = %Hu\n", FUNC, ret_value, size);
#endif /* H5MF_ALLOC_DEBUG */
#ifdef H5MF_ALLOC_DEBUG_DUMP
H5MF_sects_dump(f, dxpl_id, stderr);
#endif /* H5MF_ALLOC_DEBUG_DUMP */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_alloc() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_alloc_pagefs
 *
 * Purpose:     Allocate space from either the large or small free-space manager.
 *		For "large" request:
 *		  Allocate request from VFD
 *		  Determine mis-aligned fragment and return the fragment to the
 *		    appropriate manager
 *		For "small" request:
 *		  Allocate a page from the large manager
 *		  Determine whether space is available from a mis-aligned fragment
 *		    being returned to the manager
 *		  Return left-over space to the manager after fulfilling request
 *
 * Return:      Success:        The file address of new chunk.
 *              Failure:        HADDR_UNDEF
 *
 * Programmer:  Vailin Choi; Dec 2012
 *
 *-------------------------------------------------------------------------
 */
static haddr_t
H5MF_alloc_pagefs(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, hsize_t size)
{
    H5F_mem_page_t ptype;		/* Free-space mananger type */
    unsigned ctype;			/* Section class type */
    H5MF_free_section_t *node = NULL;  	/* Free space section pointer */
    haddr_t eoa = HADDR_UNDEF;      	/* EOA for the file */
    hsize_t frag_size = 0;             	/* Fragment size */
    htri_t node_found = FALSE;      	/* Whether an existing free list node was found */
    haddr_t new_page;			/* The address for the new file size page */
    haddr_t ret_value; 			/* Return value */

    FUNC_ENTER_NOAPI(HADDR_UNDEF)

#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: alloc_type = %u, size = %Hu\n", FUNC, (unsigned)alloc_type, size);
#endif /* H4MF_ALLOC_DEBUG */

    /* Determine the free-space manager type for paged aggregation */
    ptype = H5MF_ALLOC_TO_FS_PAGE_TYPE(f, alloc_type, size);

    switch(ptype) {
	case H5F_MEM_PAGE_GENERIC:  /* Large untyped */

	    /* Get the EOA for the file */
	    if(HADDR_UNDEF == (eoa = H5F_get_eoa(f, alloc_type)))
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, HADDR_UNDEF, "Unable to get eoa")

	    /* Calculate the mis-aligned fragment */
	    H5MF_EOA_MISALIGN(f, eoa, f->shared->fs.page_size, frag_size);

	    /* Check for overlapping into file's temporary allocation space */
	    if(H5F_addr_gt((eoa + size + frag_size), f->shared->fs.tmp_addr))
		HGOTO_ERROR(H5E_RESOURCE, H5E_BADRANGE, HADDR_UNDEF, "'normal' file space allocation request will overlap into 'temporary' file space")

	    /* Allocate from VFD */
	    if(HADDR_UNDEF == (ret_value = H5FD_alloc(f->shared->lf, dxpl_id, alloc_type, f, size+frag_size)))
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "can't allocate file space")

	    /* Return the aligned address */
	    ret_value += frag_size;

	    if(frag_size) { /* If there is a mis-aligned fragment at EOF */

		/* Initialize to "large" free-space section */
		ctype = H5MF_FSPACE_SECT_LARGE;
		/* Check the EOF section type when the file is last closed */
		if(f->shared->fs.last_small) { /* A small meta or raw data section type at EOF */
		    /* A "small" free-space section */
		    ctype = H5MF_FSPACE_SECT_SMALL;	
		    /* Determine the free-space manager type */
		    ptype = (f->shared->fs.last_small == H5F_FILE_SPACE_EOF_SMALL_RAW) ?  H5F_MEM_PAGE_RAW : H5F_MEM_PAGE_META;
		} /* end if */

		/* Start up the free-space manager */
		if(!(f->shared->fs.man[ptype]))
		    if(H5MF_start_fstype(f, dxpl_id, (H5FD_mem_t)ptype) < 0)
			HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, HADDR_UNDEF, "can't initialize file free space")

		/* Create free space section for the fragment: either a large or small section */
		if(NULL == (node = H5MF_sect_new(ctype, eoa, frag_size)))
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, HADDR_UNDEF, "can't initialize free space section")

		/* Add the fragment to either the large or small (meta or raw) free-space manager */
		if(H5MF_add_sect(f, alloc_type, dxpl_id, f->shared->fs.man[ptype], node) < 0)
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINSERT, HADDR_UNDEF, "can't re-add section to file free space")

		node = NULL;
	    } /* end if */

	    if(f->shared->fs.last_small)
		f->shared->fs.last_small = 0;
	    break;

	case H5F_MEM_PAGE_META:	/* Small meta data */
	case H5F_MEM_PAGE_RAW:	/* Small raw data */
	    if(f->shared->fs.man_state[ptype] == H5F_FS_STATE_DELETING) {
		if((ret_value = H5MF_close_allocate(f, alloc_type, dxpl_id, size)) == HADDR_UNDEF)
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "can't allocate file space")
		HGOTO_DONE(ret_value);
	    } /* end if */

	    /* Allocate one file space page */
	    if(HADDR_UNDEF == (new_page = H5MF_alloc(f, alloc_type, dxpl_id, f->shared->fs.page_size)))
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "can't allocate file space")

	    /* Check if the free space manager for the file has been initialized */
	    if(!(f->shared->fs.man[ptype]) && H5F_addr_defined(f->shared->fs.man_addr[ptype]))
		if(H5MF_open_fstype(f, dxpl_id, (H5FD_mem_t)ptype) < 0)
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTOPENOBJ, HADDR_UNDEF, "can't initialize file free space")

	    /*
	     * Check the small free space manager again for the space request as there might
	     * possibly a "mis-aligned" fragment added to the small manager while allocating
	     * the page from the large manager.
	     */
	    if(f->shared->fs.man[ptype])
		node_found = H5MF_find_sect(f, alloc_type, dxpl_id, size, f->shared->fs.man[ptype], &ret_value);

	    if(node_found) { /* Space is found from small manager */
		HDassert(H5F_addr_defined(new_page));
		/* Free the newly allocated page */
		if(H5MF_xfree(f, alloc_type, dxpl_id, new_page, f->shared->fs.page_size) < 0)
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTFREE, HADDR_UNDEF, "can't free the new page")
	    } /* end if */
            else { /* Space is not found from the small manager */
		/* Start up the free-space manager */
		if(!(f->shared->fs.man[ptype]))
		    if(H5MF_start_fstype(f, dxpl_id, (H5FD_mem_t)ptype) < 0)
			HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, HADDR_UNDEF, "can't initialize file free space")
		HDassert(f->shared->fs.man[ptype]);

		/* Create section for remaining space in the page */
		if(NULL == (node = H5MF_sect_new(H5MF_FSPACE_SECT_SMALL, (new_page + size), (f->shared->fs.page_size - size))))
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, HADDR_UNDEF, "can't initialize free space section")

		/* Add the remaining space in the page to the manager */
		if(H5MF_add_sect(f, alloc_type, dxpl_id, f->shared->fs.man[ptype], node) < 0)
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINSERT, HADDR_UNDEF, "can't re-add section to file free space")

		node = NULL;

		ret_value = new_page;
	    } /* end else */
	    break;

	case H5F_MEM_PAGE_NTYPES:
	default:
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "can't allocate file space: unrecognized type")
	    break;
    } /* end switch */

done:

#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving: ret_value = %a, size = %Hu\n", FUNC, ret_value, size);
#endif /* H5MF_ALLOC_DEBUG */
#ifdef H5MF_ALLOC_DEBUG_DUMP
H5MF_sects_dump(f, dxpl_id, stderr);
#endif /* H5MF_ALLOC_DEBUG_DUMP */

    /* Release section node, if allocated and not added to section list or merged */
    if(node)
        if(H5MF_sect_free((H5FS_section_info_t *)node) < 0)
	    HDONE_ERROR(H5E_RESOURCE, H5E_CANTRELEASE, HADDR_UNDEF, "can't free section node")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_alloc_pagefs() */

/*-------------------------------------------------------------------------
 * Function:    H5MF_close_allocate
 *
 * Purpose:     To allocate file space for free-space manager header (H5FS_alloc_hdr())
 *		and section info (H5FS_alloc_sect()) in H5MF_close() when
 *		persisting free-space.
 *		Note that any mis-aligned fragment at closing is dropped to the floor
 *		so that it won't change the section info size.
 *
 * Return:      Success:        The file address of new chunk.
 *              Failure:        HADDR_UNDEF
 *
 * Programmer:  Vailin Choi; Jan 2013
 *
 *-------------------------------------------------------------------------
 */
haddr_t
H5MF_close_allocate(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, hsize_t size)
{
    H5F_mem_page_t ptype;		/* The free-space manager type for paged aggregation */
    haddr_t eoa = HADDR_UNDEF;      	/* EOA for the file */
    hsize_t frag_size = 0;             	/* Fragment size */
    haddr_t ret_value; 			/* Return value */

    FUNC_ENTER_NOAPI(HADDR_UNDEF)

    /*
     * Check arguments.
     */
    HDassert(f);
    HDassert(f->shared);

    /* Get the EOA for the file */
    if(HADDR_UNDEF == (eoa = H5F_get_eoa(f, alloc_type)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, HADDR_UNDEF, "Unable to get eoa")

    if(H5F_PAGED_AGGR(f)) { /* Paged aggregation */
	ptype = H5MF_ALLOC_TO_FS_PAGE_TYPE(f, alloc_type, size);

	/* Can allocate from EOF only if the request has the same type as the last section in the EOF page */
	if((f->shared->fs.track_last_small == H5F_FILE_SPACE_EOF_SMALL_META && ptype == H5F_MEM_PAGE_META) ||
               (f->shared->fs.track_last_small == H5F_FILE_SPACE_EOF_SMALL_RAW && ptype == H5F_MEM_PAGE_RAW)) {
	    HDassert(size < f->shared->fs.page_size);	

	    /* Can allocate from EOF if the request falls within the same page -- not crossing page boundary */
	    if((eoa / f->shared->fs.page_size) == ((eoa+size-1) / f->shared->fs.page_size)) {
		/* Check for overlapping into file's temporary allocation space */
		if(H5F_addr_gt((eoa + size), f->shared->fs.tmp_addr))
		    HGOTO_ERROR(H5E_RESOURCE, H5E_BADRANGE, HADDR_UNDEF, "'normal' file space allocation request will overlap into 'temporary' file space")
		if(HADDR_UNDEF == (ret_value = H5FD_alloc(f->shared->lf, dxpl_id, alloc_type, f, size)))
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "can't allocate file space")
		HGOTO_DONE(ret_value);
	    } /* end if */
	} /* end if */

	/* Calculate the misaligned fragment which is dropped to the floor */
	H5MF_EOA_MISALIGN(f, eoa, f->shared->fs.page_size, frag_size);

	/* Check for overlapping into file's temporary allocation space */
	if(H5F_addr_gt((eoa + size + frag_size), f->shared->fs.tmp_addr))
	    HGOTO_ERROR(H5E_RESOURCE, H5E_BADRANGE, HADDR_UNDEF, "'normal' file space allocation request will overlap into 'temporary' file space")

	/* Allocate the request + misaligned fragment */
	if(HADDR_UNDEF == (ret_value = H5FD_alloc(f->shared->lf, dxpl_id, alloc_type, f, size+frag_size)))
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "can't allocate file space")

	/* Return the aligned address */
	if(frag_size) {
	    HDassert(H5F_addr_eq(ret_value, eoa));
	    ret_value += frag_size;
	} /* end if */

	/* Update track_last_small for paged aggregation */
	if(size >= f->shared->fs.page_size)
	    f->shared->fs.track_last_small = 0;
	else if(ptype == H5F_MEM_PAGE_META)
	    f->shared->fs.track_last_small = H5F_FILE_SPACE_EOF_SMALL_META;
	else if(ptype == H5F_MEM_PAGE_RAW)
	    f->shared->fs.track_last_small = H5F_FILE_SPACE_EOF_SMALL_RAW;
    } /* end if */
    else {
	/* Allocate the request but not freeing the mis-aligned fragment if any */
	/* The misaligned fragment is dropped to the floor */
	if(HADDR_UNDEF == (ret_value = H5MF_vfd_alloc(f, dxpl_id, alloc_type, size, FALSE)))
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, HADDR_UNDEF, "can't allocate file space")
    } /* end else */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_close_allocate() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_set_last_small
 *
 * Purpose:     Determine the section at EOF is a small meta/raw or large data section.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Vailin Choi; Dec 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_set_last_small(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id,
    haddr_t addr, hsize_t size, hbool_t xfree)
{
    haddr_t eoa = HADDR_UNDEF; 	/* Initial EOA for the file */
    herr_t ret_value = SUCCEED;	/* Return value */

    FUNC_ENTER_NOAPI(FAIL)

#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering: alloc_type = %u, addr = %a, size = %Hu, xfree = %t\n", FUNC, (unsigned)alloc_type, addr, size, xfree);
#endif /* H5MF_ALLOC_DEBUG */

    HDassert(f);
    HDassert(H5F_addr_defined(addr));
    HDassert(H5F_PAGED_AGGR(f));

    /* Get the EOA for the file */
    if(HADDR_UNDEF == (eoa = H5F_get_eoa(f, alloc_type)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "Unable to get eoa")

    if(size < f->shared->fs.page_size) { /* For small section */
	/* If the section is on the same page as EOF, the last section at EOF is a small section */
	if(((eoa-1) / f->shared->fs.page_size) == (addr / f->shared->fs.page_size)) {

	    H5F_mem_page_t ptype = H5MF_ALLOC_TO_FS_PAGE_TYPE(f, alloc_type, size);
	    f->shared->fs.track_last_small = (ptype == H5F_MEM_PAGE_META) ? H5F_FILE_SPACE_EOF_SMALL_META : H5F_FILE_SPACE_EOF_SMALL_RAW;
	} /* end if */
    } /* end if */
    else
        if(H5F_addr_eq(addr+size, eoa)) { /* For large section */
            HDassert(!(addr % f->shared->fs.page_size));
            if(xfree) { /* from H5MF_xfree() */
                H5F_mem_page_t ptype;	/* Local index variable */

                /* Check the last section of each free-space manager */
                for(ptype = H5F_MEM_PAGE_META; ptype < H5F_MEM_PAGE_NTYPES; H5_INC_ENUM(H5F_mem_page_t, ptype)) {
                    haddr_t sect_addr = HADDR_UNDEF;	/* Section address */
                    hsize_t sect_size = 0;			/* Section size */

                    if(f->shared->fs.man[ptype]) {
                        /* Get the last section of the free-space manager */
                        if(H5FS_sect_query_last(f, dxpl_id, f->shared->fs.man[ptype], &sect_addr, &sect_size) < 0)
                            HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "can't query last section on merge list")

                        /*
                         * If the section being freed adjoins the last section,
                         * the section at EOF is determined by the type of the last section.
                         * If not, EOF is at page boundary and the tracking does not matter.
                         */
                        if(H5F_addr_defined(sect_addr) && H5F_addr_eq(sect_addr + sect_size, addr)) {
                            if(ptype == H5F_MEM_PAGE_GENERIC)
                                f->shared->fs.track_last_small = 0;
                            else
                                f->shared->fs.track_last_small = (ptype == H5F_MEM_PAGE_META) ? H5F_FILE_SPACE_EOF_SMALL_META : H5F_FILE_SPACE_EOF_SMALL_RAW;
                            break;
                        } /* end if */
                    } /* end if */
                } /* end for */
            }
            else /* from H5MF_alloc() */
                /* The section at EOF is not a small section */
                f->shared->fs.track_last_small = 0;
        } /* end if */

done:
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving, track_last_small = %u\n", FUNC, (unsigned)f->shared->fs.track_last_small);
#endif /* H5MF_ALLOC_DEBUG */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_set_last_small() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_alloc_tmp
 *
 * Purpose:     Allocate temporary space in the file
 *
 * Note:	The address returned is non-overlapping with any other address
 *		in the file and suitable for insertion into the metadata
 *		cache.
 *
 *		The address is _not_ suitable for actual file I/O and will
 *		cause an error if it is so used.
 *
 *		The space allocated with this routine should _not_ be freed,
 *		it should just be abandoned.  Calling H5MF_xfree() with space
 *              from this routine will cause an error.
 *
 * Return:      Success:        Temporary file address
 *              Failure:        HADDR_UNDEF
 *
 * Programmer:  Quincey Koziol
 *              Thursday, June  4, 2009
 *
 *-------------------------------------------------------------------------
 */
haddr_t
H5MF_alloc_tmp(H5F_t *f, hsize_t size)
{
    haddr_t eoa;                /* End of allocated space in the file */
    haddr_t ret_value;          /* Return value */

    FUNC_ENTER_NOAPI(HADDR_UNDEF)
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: size = %Hu\n", FUNC, size);
#endif /* H5MF_ALLOC_DEBUG */

    /* check args */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->lf);
    HDassert(size > 0);

    /* Retrieve the 'eoa' for the file */
    if(HADDR_UNDEF == (eoa = H5FD_get_eoa(f->shared->lf, H5FD_MEM_DEFAULT)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, HADDR_UNDEF, "driver get_eoa request failed")

    /* Compute value to return */
    ret_value = f->shared->fs.tmp_addr - size;

    /* Check for overlap into the actual allocated space in the file */
    if(H5F_addr_le(ret_value, eoa))
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, HADDR_UNDEF, "driver get_eoa request failed")

    /* Adjust temporary address allocator in the file */
    f->shared->fs.tmp_addr = ret_value;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_alloc_tmp() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_xfree
 *
 * Purpose:     Frees part of a file, making that part of the file
 *              available for reuse.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Robb Matzke
 *              matzke@llnl.gov
 *              Jul 17 1997
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5MF_xfree(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, haddr_t addr,
    hsize_t size)
{
    H5FD_mem_t  fs_type;                /* Free space type (mapped from allocation type) */
    H5MF_free_section_t *node = NULL;   /* Free space section pointer */
    unsigned ctype;			/* section class type */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering - alloc_type = %u, addr = %a, size = %Hu\n", FUNC, (unsigned)alloc_type, addr, size);
#endif /* H5MF_ALLOC_DEBUG */

    /* check arguments */
    HDassert(f);
    if(!H5F_addr_defined(addr) || 0 == size)
        HGOTO_DONE(SUCCEED);
    HDassert(addr != 0);        /* Can't deallocate the superblock :-) */

    /* Determine the EOF section type */
    if(H5F_PAGED_AGGR(f))
	if(H5MF_set_last_small(f, alloc_type, dxpl_id, addr, size, TRUE) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, FAIL, "can't determine track_last_small")

    /* Check for attempting to free space that's a 'temporary' file address */
    if(H5F_addr_le(f->shared->fs.tmp_addr, addr))
        HGOTO_ERROR(H5E_RESOURCE, H5E_BADRANGE, FAIL, "attempting to free temporary file space")

    /* Check if the space to free intersects with the file's metadata accumulator */
    if(H5F_accum_free(f, dxpl_id, alloc_type, addr, size) < 0)
        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTFREE, FAIL, "can't check free space intersection w/metadata accumulator")

    /* Get free-space manager type from allocation type */
    if(H5F_PAGED_AGGR(f))
	fs_type = (H5FD_mem_t)H5MF_ALLOC_TO_FS_PAGE_TYPE(f, alloc_type, size);
    else
	fs_type = H5MF_ALLOC_TO_FS_AGGR_TYPE(f, alloc_type);

    /* Check if the free space manager for the file has been initialized */
    if(!f->shared->fs.man[fs_type]) {
        /* If there's no free space manager for objects of this type,
         *  see if we can avoid creating one by checking if the freed
         *  space is at the end of the file
         */
#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: fs.man_addr = %a\n", FUNC, f->shared->fs.man_addr[fs_type]);
#endif /* H5MF_ALLOC_DEBUG_MORE */
        if(!H5F_addr_defined(f->shared->fs.man_addr[fs_type])) {
            htri_t status;          /* "can absorb" status for section into */

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Trying to avoid starting up free space manager\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG_MORE */
            /* Try to shrink the file or absorb the block into a block aggregator */
            if((status = H5MF_try_shrink(f, alloc_type, dxpl_id, addr, size)) < 0)
                HGOTO_ERROR(H5E_FSPACE, H5E_CANTMERGE, FAIL, "can't check for absorbing block")
            else if(status > 0)
                /* Indicate success */
                HGOTO_DONE(SUCCEED)
	    else if(size < f->shared->fs.threshold) {
#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: dropping addr = %a, size = %Hu, on the floor!\n", FUNC, addr, size);
#endif /* H5MF_ALLOC_DEBUG_MORE */
		HGOTO_DONE(SUCCEED)
	    }
        } /* end if */

        /* If we are deleting the free space manager, leave now, to avoid
         *  [re-]starting it.
         * Note: this drops the space to free on the floor...
         */
        if(f->shared->fs.man_state[fs_type] == H5F_FS_STATE_DELETING) {
#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: dropping addr = %a, size = %Hu, on the floor!\n", FUNC, addr, size);
#endif /* H5MF_ALLOC_DEBUG_MORE */
            HGOTO_DONE(SUCCEED)
        } /* end if */

        /* There's either already a free space manager, or the freed
         *  space isn't at the end of the file, so start up (or create)
         *  the file space manager
         */
        if(H5MF_start_fstype(f, dxpl_id, fs_type) < 0)
            HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, FAIL, "can't initialize file free space")
    } /* end if */

    /* Create the free-space section for the freed section */
    ctype = H5MF_SECT_CLASS_TYPE(f, size);
    if(NULL == (node = H5MF_sect_new(ctype, addr, size)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, FAIL, "can't initialize free space section")

    /* If size of the freed section is larger than threshold, add it to the free space manager */
    if(size >= f->shared->fs.threshold) {
	HDassert(f->shared->fs.man[fs_type]);

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Before H5FS_sect_add()\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG_MORE */

        /* Add to the free space for the file */
	if(H5MF_add_sect(f, alloc_type, dxpl_id, f->shared->fs.man[fs_type], node) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINSERT, FAIL, "can't add section to file free space")
	node = NULL;

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: After H5FS_sect_add()\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG_MORE */

    } /* end if */
    else {
        htri_t merged;          /* Whether node was merged */
	H5MF_sect_ud_t udata; 	/* User data for callback */

	/* Construct user data for callbacks */
	udata.f = f;
	udata.dxpl_id = dxpl_id;
	udata.alloc_type = alloc_type;
	udata.allow_sect_absorb = TRUE;
	udata.allow_eoa_shrink_only = FALSE;
	udata.allow_small_shrink = FALSE;

        /* Try to merge the section that is smaller than threshold */
	if((merged = H5FS_sect_try_merge(f, dxpl_id, f->shared->fs.man[fs_type], (H5FS_section_info_t *)node, H5FS_ADD_RETURNED_SPACE, &udata)) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINSERT, FAIL, "can't merge section to file free space")
	else if(merged == TRUE) /* successfully merged */
	    /* Indicate that the node was used */
            node = NULL;
    } /* end else */

done:
    /* Release section node, if allocated and not added to section list or merged */
    if(node)
        if(H5MF_sect_free((H5FS_section_info_t *)node) < 0)
            HDONE_ERROR(H5E_RESOURCE, H5E_CANTRELEASE, FAIL, "can't free simple section node")

#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving, ret_value = %d\n", FUNC, ret_value);
#endif /* H5MF_ALLOC_DEBUG */
#ifdef H5MF_ALLOC_DEBUG_DUMP
H5MF_sects_dump(f, dxpl_id, stderr);
#endif /* H5MF_ALLOC_DEBUG_DUMP */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_xfree() */


/*-------------------------------------------------------------------------
 * Function:	H5MF_try_extend
 *
 * Purpose:	Extend a block in the file if possible.
 *
 * Notes:
 *	For paged aggregation--
 *	A small block cannot be extended across page boundary.
 *	1) Try extending the block if it is at EOF
 *	2) Try extending the block into a free-space section
 *	3) For a small meta block that is within page end threshold--
 *	     check if extension is possible
 *
 * Return:	Success:	TRUE(1)  - Block was extended
 *                              FALSE(0) - Block could not be extended
 * 		Failure:	FAIL
 *
 * Programmer:	Quincey Koziol
 *              Friday, June 11, 2004
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5MF_try_extend(H5F_t *f, hid_t dxpl_id, H5FD_mem_t alloc_type, haddr_t addr,
    hsize_t size, hsize_t extra_requested)
{
    haddr_t     end;            		/* End of block to extend */
    H5FD_mem_t  map_type;       		/* Mapped type */
    htri_t 	allow_extend = TRUE;		/* Possible to extend the block */
    htri_t	ret_value = FALSE;      	/* Return value */

    FUNC_ENTER_NOAPI(FAIL)
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering: alloc_type = %u, addr = %a, size = %Hu, extra_requested = %Hu\n", FUNC, (unsigned)alloc_type, addr, size, extra_requested);
#endif /* H5MF_ALLOC_DEBUG */

    /* Sanity check */
    HDassert(f);
    HDassert(H5F_INTENT(f) & H5F_ACC_RDWR);

    /* Set mapped type, treating global heap as raw data */
    map_type = (alloc_type == H5FD_MEM_GHEAP) ? H5FD_MEM_DRAW : alloc_type;

    /* Compute end of block to extend */
    end = addr + size;

    /* For paged aggregation and a small section: determine whether page boundary can be crossed for the extension */
    if(H5F_PAGED_AGGR(f) && size < f->shared->fs.page_size)
	if((addr / f->shared->fs.page_size) != ((addr + size + extra_requested-1) / f->shared->fs.page_size))
	    allow_extend = FALSE;

    if(allow_extend) {

	/* Try extending the block if it is at EOF */
	if((ret_value = H5FD_try_extend(f->shared->lf, map_type, f, end, extra_requested)) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTEXTEND, FAIL, "error extending file")

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: H5FD_try_extend = %t\n", FUNC, ret_value);
#endif /* H5MF_ALLOC_DEBUG_MORE */

	/* For non-paged aggregation */
	if(ret_value == FALSE && !H5F_PAGED_AGGR(f)) {
	    H5F_blk_aggr_t *aggr;   /* Aggregator to use */

	    /* Check if the block is able to extend into aggregation block */
	    aggr = (map_type == H5FD_MEM_DRAW) ?  &(f->shared->fs.sdata_aggr) : &(f->shared->fs.meta_aggr);
	    if((ret_value = H5MF_aggr_try_extend(f, aggr, map_type, end, extra_requested)) < 0)
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTEXTEND, FAIL, "error extending aggregation block")

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: H5MF_aggr_try_extend = %t\n", FUNC, ret_value);
#endif /* H5MF_ALLOC_DEBUG_MORE */
	}

	if(ret_value == FALSE) {

	    H5FD_mem_t fs_type;       	/* Free-space manager type */
	    H5MF_sect_ud_t udata;	/* User data */

	    /* Construct user data for callbacks */
	    udata.f = f;
	    udata.dxpl_id = dxpl_id;
	    udata.alloc_type = alloc_type;
	    udata.allow_small_shrink = FALSE;

	    /* Get free-space manager type from allocation type */
	    if(H5F_PAGED_AGGR(f))
		fs_type = (H5FD_mem_t)H5MF_ALLOC_TO_FS_PAGE_TYPE(f, alloc_type, size);
	    else
		fs_type = H5MF_ALLOC_TO_FS_AGGR_TYPE(f, alloc_type);

            /* Check if the free space for the file has been initialized */
	    if(!f->shared->fs.man[fs_type] && H5F_addr_defined(f->shared->fs.man_addr[fs_type]))
		/* Open the free-space manager */
		if(H5MF_open_fstype(f, dxpl_id, fs_type) < 0)
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, FAIL, "can't initialize file free space")

	    /* Check if the block is able to extend into a free-space section */
            if(f->shared->fs.man[fs_type]) {
                if((ret_value = H5FS_sect_try_extend(f, dxpl_id, f->shared->fs.man[fs_type], addr, size, extra_requested, H5FS_ADD_RETURNED_SPACE, &udata)) < 0)
                    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTEXTEND, FAIL, "error extending block in free space manager")
#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Try to H5FS_sect_try_extend = %t\n", FUNC, ret_value);
#endif /* H5MF_ALLOC_DEBUG_MORE */
	    }

	    /* For paged aggregation: If the extended request for a small meta section is within page end threshold */
	    if(ret_value == FALSE && H5F_PAGED_AGGR(f) && map_type != H5FD_MEM_DRAW) {
		hsize_t prem = f->shared->fs.page_size - (end % f->shared->fs.page_size);

		if(prem <= H5F_PGEND_META_THRES(f) && prem >= extra_requested)
		    ret_value = TRUE;
#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Try to extend into the page end threshold = %t\n", FUNC, ret_value);
#endif /* H5MF_ALLOC_DEBUG_MORE */
	    }

	} /* end if */

    } /* allow_extend */

done:
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving: ret_value = %t\n", FUNC, ret_value);
#endif /* H5MF_ALLOC_DEBUG */
#ifdef H5MF_ALLOC_DEBUG_DUMP
H5MF_sects_dump(f, dxpl_id, stderr);
#endif /* H5MF_ALLOC_DEBUG_DUMP */

    /* For paged aggregation --determine the section at EOF is small or not */
    if(ret_value == TRUE && H5F_PAGED_AGGR(f))
	if(H5MF_set_last_small(f, alloc_type, dxpl_id, addr, size+extra_requested, FALSE) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, FAIL, "can't determine track_last_small")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_try_extend() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_try_shrink
 *
 * Purpose:     Try to shrink the size of a file with a block or absorb it
 *              into a block aggregator.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@hdfgroup.org
 *              Feb 14 2008
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5MF_try_shrink(H5F_t *f, H5FD_mem_t alloc_type, hid_t dxpl_id, haddr_t addr,
    hsize_t size)
{
    H5MF_free_section_t *node = NULL;   /* Free space section pointer */
    H5MF_sect_ud_t udata;               /* User data for callback */
    H5FS_section_class_t *sect_cls;	/* Section class */
    htri_t ret_value;                   /* Return value */

    FUNC_ENTER_NOAPI(FAIL)
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering - alloc_type = %u, addr = %a, size = %Hu\n", FUNC, (unsigned)alloc_type, addr, size);
#endif /* H5MF_ALLOC_DEBUG */

    /* check arguments */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->lf);
    HDassert(H5F_addr_defined(addr));
    HDassert(size > 0);

    /* Set up free-space section class information */
    sect_cls = H5MF_SECT_CLS_TYPE(f, size);
    HDassert(sect_cls);

    /* Create free-space section for block */
    if(NULL == (node = H5MF_sect_new(sect_cls->type, addr, size)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, FAIL, "can't initialize free space section")

    /* Construct user data for callbacks */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.alloc_type = alloc_type;
    udata.allow_sect_absorb = FALSE;    /* Force section to be absorbed into aggregator */
    udata.allow_eoa_shrink_only = FALSE;
    udata.allow_small_shrink = TRUE;

    /* Check if the block can shrink the container */
    if(sect_cls->can_shrink) {
	if((ret_value = (*sect_cls->can_shrink)((const H5FS_section_info_t *)node, &udata)) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTMERGE, FAIL, "can't check if section can shrink container")
	if(ret_value > 0) {
	    HDassert(sect_cls->shrink);
	    if(H5F_PAGED_AGGR(f))
		if(H5MF_set_last_small(f, alloc_type, dxpl_id, addr, size, TRUE) < 0)
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, FAIL, "can't determine track_last_small")

	    if((*sect_cls->shrink)((H5FS_section_info_t **)&node, &udata) < 0)
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSHRINK, FAIL, "can't shrink container")
	} /* end if */
    } /* end if */

done:
    /* Free section node allocated */
    if(node && H5MF_sect_free((H5FS_section_info_t *)node) < 0)
        HDONE_ERROR(H5E_RESOURCE, H5E_CANTRELEASE, FAIL, "can't free simple section node")

#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving, ret_value = %d\n", FUNC, ret_value);
#endif /* H5MF_ALLOC_DEBUG */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_try_shrink() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_close
 *
 * Purpose:     Close the free space tracker(s) for a file:
 *		  paged or non-paged aggregation
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:  Vailin Choi; Dec 2012
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5MF_close(H5F_t *f, hid_t dxpl_id)
{
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(FAIL)
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */

    /* check args */
    HDassert(f);
    HDassert(f->shared);

    if(H5F_PAGED_AGGR(f)) {
	if((ret_value = H5MF_close_pagefs(f, dxpl_id)) < 0)
	    HGOTO_ERROR(H5E_FILE, H5E_CANTFREE, FAIL, "can't close free-space managers for 'page' file space")
    } /* end if */
    else {
	if((ret_value = H5MF_close_aggrfs(f, dxpl_id)) < 0)
	    HGOTO_ERROR(H5E_FILE, H5E_CANTFREE, FAIL, "can't close free-space managers for 'aggr' file space")
    } /* end else */

done:
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_close() */

/*-------------------------------------------------------------------------
 * Function:    H5MF_close_aggrfs
 *
 * Purpose:     Close the free space tracker(s) for a file: non-paged aggregation
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:  Quincey Koziol
 *              Tuesday, January 22, 2008
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_close_aggrfs(H5F_t *f, hid_t dxpl_id)
{
    H5FD_mem_t type;          	/* Memory type for iteration */
    herr_t ret_value = SUCCEED;	/* Return value */

    FUNC_ENTER_NOAPI(FAIL)
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */

    /* check args */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->lf);
    HDassert(f->shared->sblock);

    /* Free the space in aggregators */
    /* (for space not at EOF, it may be put into free space managers) */
    if(H5MF_free_aggrs(f, dxpl_id) < 0)
        HGOTO_ERROR(H5E_FILE, H5E_CANTFREE, FAIL, "can't free aggregators")

    /* Trying shrinking the EOA for the file */
    if(H5MF_close_shrink_eoa(f, dxpl_id) < 0)
        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSHRINK, FAIL, "can't shrink eoa")

    /* Making free-space managers persistent for superblock version >= 2 */
    if(f->shared->sblock->super_vers >= HDF5_SUPERBLOCK_VERSION_2
            && f->shared->fs.persist) {
        H5O_fsinfo_t fsinfo;		/* File space info message */
        hbool_t update = FALSE;		/* To update info for the message */

        /* Check to remove file space info message from superblock extension */
        if(H5F_addr_defined(f->shared->sblock->ext_addr))
            if(H5F_super_ext_remove_msg(f, dxpl_id, H5O_FSINFO_ID) < 0)
                HGOTO_ERROR(H5E_RESOURCE, H5E_CANTRELEASE, FAIL, "error in removing message from superblock extension")

	/* Free free-space manager header and/or section info header */
	for(type = H5FD_MEM_SUPER; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type)) {

	    /* Check for free space manager of this type */
	    if(f->shared->fs.man[type])
		/* Free the free space manager in the file (will re-allocate later) */
		if(H5MF_free_fstype(f, dxpl_id, type) < 0)
		    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't free the free space manager")

	    fsinfo.fs_addr[type-1] = HADDR_UNDEF;
	} /* end for */

	/* Set up file space info message */
	fsinfo.strategy = f->shared->fs.strategy;
	fsinfo.persist = f->shared->fs.persist;
	fsinfo.threshold = f->shared->fs.threshold;
	fsinfo.page_size = f->shared->fs.page_size;
	HDassert(!f->shared->fs.track_last_small);
	fsinfo.last_small = f->shared->fs.track_last_small;
	fsinfo.pgend_meta_thres = f->shared->fs.pgend_meta_thres;

	/* Write file space info message to superblock extension object header */
	/* Create the superblock extension object header in advance if needed */
	if(H5F_super_ext_write_msg(f, dxpl_id, &fsinfo, H5O_FSINFO_ID, H5O_MSG_FLAG_DONTSHARE|H5O_MSG_FLAG_MARK_IF_UNKNOWN, TRUE) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_WRITEERROR, FAIL, "error in writing message to superblock extension")

        /* Re-allocate free-space manager header and/or section info header */
	for(type = H5FD_MEM_SUPER; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type)) {
	    haddr_t *fsaddr = &(fsinfo.fs_addr[type-1]);

	    if(H5MF_realloc_fstype(f, dxpl_id, type, fsaddr, &update) < 0)
		HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't re-allocate the free space manager")
	} /* end for */

	/* Update the file space info message in superblock extension object header */
	if(update)
            if(H5F_super_ext_write_msg(f, dxpl_id, &fsinfo, H5O_FSINFO_ID, H5O_MSG_FLAG_DONTSHARE|H5O_MSG_FLAG_MARK_IF_UNKNOWN, FALSE) < 0)
	        HGOTO_ERROR(H5E_RESOURCE, H5E_WRITEERROR, FAIL, "error in writing message to superblock extension")

	/* Trying shrinking the EOA for the file */
	/* (in case any free space is now at the EOA) */
	if(H5MF_close_shrink_eoa(f, dxpl_id) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSHRINK, FAIL, "can't shrink eoa")

	/* Final close of free-space managers */
	for(type = H5FD_MEM_DEFAULT; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type)) {
	    if(f->shared->fs.man[type])
		 if(H5MF_close_fstype(f, dxpl_id, type) < 0)
                    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't close the free space manager")
	    f->shared->fs.man_addr[type] = HADDR_UNDEF;
	} /* end for */
    } /* end if */
    else {  /* super_vers can be 0, 1, 2 */
	/* Iterate over all the free space types that have managers and get each free list's space */
	for(type = H5FD_MEM_DEFAULT; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type)) {

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Check 1.0 - type = %u, fs_man = %p, fs_addr = %a\n", FUNC, (unsigned)type, f->shared->fs.man[type], f->shared->fs.man_addr[type]);
#endif /* H5MF_ALLOC_DEBUG_MORE */

	    /* If the free space manager for this type is open, close it */
	    if(f->shared->fs.man[type])
		if(H5MF_close_fstype(f, dxpl_id, type) < 0)
		    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't close the free space manager")

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Check 2.0 - type = %u, fs_man = %p, fs_addr = %a\n", FUNC, (unsigned)type, f->shared->fs.man[type], f->shared->fs.man_addr[type]);
#endif /* H5MF_ALLOC_DEBUG_MORE */

	    /* If there is free space manager info for this type, delete it */
	    if(H5F_addr_defined(f->shared->fs.man_addr[type]))
		if(H5MF_delete_fstype(f, dxpl_id, type) < 0)
		    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't delete the free space manager")
	} /* end for */
    } /* end else */

    /* Free the space in aggregators (again) */
    /* (in case any free space information re-started them) */
    if(H5MF_free_aggrs(f, dxpl_id) < 0)
        HGOTO_ERROR(H5E_FILE, H5E_CANTFREE, FAIL, "can't free aggregators")

    /* Trying shrinking the EOA for the file */
    /* (in case any free space is now at the EOA) */
    if(H5MF_close_shrink_eoa(f, dxpl_id) < 0)
        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSHRINK, FAIL, "can't shrink eoa")

done:
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_close_aggrfs() */

/*-------------------------------------------------------------------------
 * Function:    H5MF_close_pagefs
 *
 * Purpose:     Close the free space tracker(s) for a file: paged aggregation
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:  Vailin Choi; Dec 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_close_pagefs(H5F_t *f, hid_t dxpl_id)
{
    H5F_mem_page_t ptype; 	/* Memory type for iteration */
    H5FD_mem_t type; 		/* Memory type for iteration */
    H5O_fsinfo_t fsinfo;	/* File space info message */
    herr_t ret_value = SUCCEED;	/* Return value */

    FUNC_ENTER_NOAPI(FAIL)
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Entering\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */

    /* check args */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->lf);
    HDassert(f->shared->sblock);
    HDassert(f->shared->fs.page_size);
    HDassert(f->shared->sblock->super_vers >= HDF5_SUPERBLOCK_VERSION_2);

    /* Trying shrinking the EOA for the file */
    if(H5MF_close_shrink_eoa(f, dxpl_id) < 0)
        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSHRINK, FAIL, "can't shrink eoa")

    /* Set up file space info message */
    fsinfo.strategy = f->shared->fs.strategy;
    fsinfo.persist = f->shared->fs.persist;
    fsinfo.threshold = f->shared->fs.threshold;
    fsinfo.page_size = f->shared->fs.page_size;
    fsinfo.last_small = f->shared->fs.track_last_small;
    fsinfo.pgend_meta_thres = f->shared->fs.pgend_meta_thres;

    for(type = H5FD_MEM_SUPER; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type))
	fsinfo.fs_addr[type - 1] = HADDR_UNDEF;

    if(f->shared->fs.persist) {
        hbool_t update = FALSE;		/* To update info for the message */

        /* Check to remove file space info message from superblock extension */
        if(H5F_addr_defined(f->shared->sblock->ext_addr))
            if(H5F_super_ext_remove_msg(f, dxpl_id, H5O_FSINFO_ID) < 0)
                HGOTO_ERROR(H5E_RESOURCE, H5E_CANTRELEASE, FAIL, "error in removing message from superblock extension")

	/* Free free-space manager header and/or section info header */
	for(ptype = H5F_MEM_PAGE_META; ptype < H5F_MEM_PAGE_NTYPES; H5_INC_ENUM(H5F_mem_page_t, ptype)) {

	    /* Check for free space manager of this type */
	    if(f->shared->fs.man[ptype])
		if(H5MF_free_fstype(f, dxpl_id, (H5FD_mem_t)ptype) < 0)
		    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't free the free space manager")
	} /* end for */

	/* Write file space info message to superblock extension object header */
	/* Create the superblock extension object header in advance if needed */
	if(H5F_super_ext_write_msg(f, dxpl_id, &fsinfo, H5O_FSINFO_ID, H5O_MSG_FLAG_DONTSHARE|H5O_MSG_FLAG_MARK_IF_UNKNOWN, TRUE) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_WRITEERROR, FAIL, "error in writing message to superblock extension")

	/* Trying shrinking the EOA for the file */
	/* (in case any free space is now at the EOA) */
	if(H5MF_close_shrink_eoa(f, dxpl_id) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSHRINK, FAIL, "can't shrink eoa")

	/* Re-allocate free-space manager header and/or section info header */
	for(ptype = H5F_MEM_PAGE_META; ptype < H5F_MEM_PAGE_NTYPES; H5_INC_ENUM(H5F_mem_page_t, ptype)) {
	    haddr_t *fsaddr = &(fsinfo.fs_addr[ptype]);

	    if(H5MF_realloc_fstype(f, dxpl_id, (H5FD_mem_t)ptype, fsaddr, &update) < 0)
		HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't re-allocate the free space manager")
	} /* end for */


	/* Update the file space info message in superblock extension object header */
	if(update) {
	    fsinfo.last_small = f->shared->fs.track_last_small;
            if(H5F_super_ext_write_msg(f, dxpl_id, &fsinfo, H5O_FSINFO_ID, H5O_MSG_FLAG_DONTSHARE|H5O_MSG_FLAG_MARK_IF_UNKNOWN, FALSE) < 0)
	        HGOTO_ERROR(H5E_RESOURCE, H5E_WRITEERROR, FAIL, "error in writing message to superblock extension")
	} /* end if */

	/* Final close of free-space managers */
	for(ptype = H5F_MEM_PAGE_META; ptype < H5F_MEM_PAGE_NTYPES; H5_INC_ENUM(H5F_mem_page_t, ptype)) {
	    if(f->shared->fs.man[ptype])
		if(H5MF_close_fstype(f, dxpl_id, (H5FD_mem_t)ptype) < 0)
		    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't close the free space manager")
	    f->shared->fs.man_addr[ptype] = HADDR_UNDEF;
	} /* end for */
    } /* end if */
    else {
	/* Iterate over all the free space types that have managers and get each free list's space */
	for(ptype = H5F_MEM_PAGE_META; ptype < H5F_MEM_PAGE_NTYPES; H5_INC_ENUM(H5F_mem_page_t, ptype)) {

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Check 1.0 - ptype = %u, fs_man = %p, fs_addr = %a\n", FUNC, (unsigned)ptype, f->shared->fs.man[ptype], f->shared->fs.man_addr[ptype]);
#endif /* H5MF_ALLOC_DEBUG_MORE */

	    /* Close the manager */
	    if(f->shared->fs.man[ptype])
		if(H5MF_close_fstype(f, dxpl_id, (H5FD_mem_t)ptype) < 0)
		    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't close the free space manager")

#ifdef H5MF_ALLOC_DEBUG_MORE
HDfprintf(stderr, "%s: Check 2.0 - ptype = %u, fs_man = %p, fs_addr = %a\n", FUNC, (unsigned)ptype, f->shared->fs.man[ptype], f->shared->fs.man_addr[ptype]);
#endif /* H5MF_ALLOC_DEBUG_MORE */

	    /* If there is free space manager info for this type, delete it */
	    if(H5F_addr_defined(f->shared->fs.man_addr[ptype]))
		if(H5MF_delete_fstype(f, dxpl_id, (H5FD_mem_t)ptype) < 0)
		    HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't delete the free space manager")
	} /* end for */

	/* Write file space info message to superblock extension object header */
	/* Create the superblock extension object header in advance if needed */
	if(H5F_super_ext_write_msg(f, dxpl_id, &fsinfo, H5O_FSINFO_ID, H5O_MSG_FLAG_DONTSHARE|H5O_MSG_FLAG_MARK_IF_UNKNOWN, FALSE) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_WRITEERROR, FAIL, "error in writing message to superblock extension")
    } /* end else */

    /* Trying shrinking the EOA for the file */
    /* (in case any free space is now at the EOA) */
    if(H5MF_close_shrink_eoa(f, dxpl_id) < 0)
        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSHRINK, FAIL, "can't shrink eoa")

done:
#ifdef H5MF_ALLOC_DEBUG
HDfprintf(stderr, "%s: Leaving\n", FUNC);
#endif /* H5MF_ALLOC_DEBUG */
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_close_pagefs() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_close_shrink_eoa
 *
 * Purpose:     Shrink the EOA while closing
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:  Quincey Koziol
 *              Saturday, July 7, 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_close_shrink_eoa(H5F_t *f, hid_t dxpl_id)
{
    H5F_mem_t type;
    H5F_mem_page_t ptype;        /* Memory type for iteration */
    hbool_t eoa_shrank;		/* Whether an EOA shrink occurs */
    htri_t status;		/* Status value */
    H5MF_sect_ud_t udata;	/* User data for callback */
    herr_t ret_value = SUCCEED;	/* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* check args */
    HDassert(f);
    HDassert(f->shared);

    /* Construct user data for callbacks */
    udata.f = f;
    udata.dxpl_id = dxpl_id;
    udata.allow_sect_absorb = FALSE;
    udata.allow_eoa_shrink_only = TRUE;
    udata.allow_small_shrink = TRUE;

    /* Iterate until no more EOA shrinking occurs */
    do {
	eoa_shrank = FALSE;

	if(H5F_PAGED_AGGR(f)) {
	    /* Check the last section of each free-space manager */
	    for(ptype = H5F_MEM_PAGE_META; ptype < H5F_MEM_PAGE_NTYPES; H5_INC_ENUM(H5F_mem_page_t, ptype)) {
		if(f->shared->fs.man[ptype]) {
		    udata.alloc_type = H5MF_PAGE_TO_ALLOC_TYPE(ptype);
		    if((status = H5FS_sect_try_shrink_eoa(f, dxpl_id, f->shared->fs.man[ptype], &udata)) < 0)
			HGOTO_ERROR(H5E_FSPACE, H5E_CANTSHRINK, FAIL, "can't check for shrinking eoa")
		    else if(status > 0)
			eoa_shrank = TRUE;
		} /* end if */
	    } /* end for */
	} else {
	    /* Check the last section of each free-space manager */
	    for(type = H5FD_MEM_DEFAULT; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type)) {
		if(f->shared->fs.man[type]) {
		    udata.alloc_type = type;
		    if((status = H5FS_sect_try_shrink_eoa(f, dxpl_id, f->shared->fs.man[type], &udata)) < 0)
			HGOTO_ERROR(H5E_FSPACE, H5E_CANTSHRINK, FAIL, "can't check for shrinking eoa")
		    else if(status > 0)
			eoa_shrank = TRUE;
		} /* end if */
	    } /* end for */

	    /* check the two aggregators */
	    if((status = H5MF_aggrs_try_shrink_eoa(f, dxpl_id)) < 0)
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTSHRINK, FAIL, "can't check for shrinking eoa")
	    else if(status > 0)
		eoa_shrank = TRUE;

	} /* end else */

    } while(eoa_shrank);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_close_shrink_eoa() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_get_freespace
 *
 * Purpose:     Retrieve the amount of free space in the file
 *
 * Return:      Success:        Amount of free space in file
 *              Failure:        Negative
 *
 * Programmer:  Quincey Koziol
 *              Monday, October  6, 2003
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5MF_get_freespace(H5F_t *f, hid_t dxpl_id, hsize_t *tot_space, hsize_t *meta_size)
{
    haddr_t eoa;                /* End of allocated space in the file */
    haddr_t ma_addr = HADDR_UNDEF;    /* Base "metadata aggregator" address */
    hsize_t ma_size = 0;        /* Size of "metadata aggregator" */
    haddr_t sda_addr = HADDR_UNDEF;    /* Base "small data aggregator" address */
    hsize_t sda_size = 0;       /* Size of "small data aggregator" */
    hsize_t tot_fs_size = 0;    /* Amount of all free space managed */
    hsize_t tot_meta_size = 0;  /* Amount of metadata for free space managers */
    H5FD_mem_t type;            /* Memory type for iteration */
    H5FD_mem_t start_type;     	/* Memory type for iteration */
    H5FD_mem_t end_type;     	/* Memory type for iteration */
    htri_t fs_started[H5FD_MEM_NTYPES]; /* Indicate whether the free-space manager has been started */
    hbool_t eoa_shrank;		/* Whether an EOA shrink occurs */
    herr_t ret_value = SUCCEED; /* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* check args */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->lf);

    /* Determine start/end points for loop */
    if(H5F_PAGED_AGGR(f)) {
	start_type = (H5FD_mem_t)H5F_MEM_PAGE_META;
	end_type = (H5FD_mem_t)H5F_MEM_PAGE_NTYPES;
    } /* end if */
    else {
	start_type = H5FD_MEM_DEFAULT;
	end_type = H5FD_MEM_NTYPES;
    } /* end else */

    /* Retrieve the 'eoa' for the file */
    if(HADDR_UNDEF == (eoa = H5FD_get_eoa(f->shared->lf, H5FD_MEM_DEFAULT)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "driver get_eoa request failed")

    if(!H5F_PAGED_AGGR(f)) {
	/* Retrieve metadata aggregator info, if available */
	if(H5MF_aggr_query(f, &(f->shared->fs.meta_aggr), &ma_addr, &ma_size) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "can't query metadata aggregator stats")

	/* Retrieve 'small data' aggregator info, if available */
	if(H5MF_aggr_query(f, &(f->shared->fs.sdata_aggr), &sda_addr, &sda_size) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "can't query small data aggregator stats")
    } /* end if */

    /* Iterate over all the free space types that have managers and get each free list's space */
    for(type = start_type; type < end_type; H5_INC_ENUM(H5FD_mem_t, type)) {

	fs_started[type] = FALSE;

	/* Check if the free space for the file has been initialized */
	if(!f->shared->fs.man[type] && H5F_addr_defined(f->shared->fs.man_addr[type])) {
	    if(H5MF_open_fstype(f, dxpl_id, type) < 0)
		HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, FAIL, "can't initialize file free space")
	    HDassert(f->shared->fs.man[type]);
            fs_started[type] = TRUE;
	} /* end if */

	/* Check if there's free space of this type */
	if(f->shared->fs.man[type]) {
	    hsize_t type_fs_size = 0;    /* Amount of free space managed for each type */
            hsize_t type_meta_size = 0;  /* Amount of free space metadata for each type */

	    /* Retrieve free space size from free space manager */
            if(H5FS_sect_stats(f->shared->fs.man[type], &type_fs_size, NULL) < 0)
                HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "can't query free space stats")
            if(H5FS_size(f, f->shared->fs.man[type], &type_meta_size) < 0)
                HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "can't query free space metadata stats")

            /* Increment total free space for types */
            tot_fs_size += type_fs_size;
            tot_meta_size += type_meta_size;
	} /* end if */
    } /* end for */

    /* Iterate until no more EOA shrink occurs */
    do {
	eoa_shrank = FALSE;

	/* Check the last section of each free-space manager */
	for(type = start_type; type < end_type; H5_INC_ENUM(H5FD_mem_t, type)) {
	    haddr_t sect_addr = HADDR_UNDEF;
	    hsize_t sect_size = 0;

	    if(f->shared->fs.man[type]) {
		if(H5FS_sect_query_last(f, dxpl_id, f->shared->fs.man[type], &sect_addr, &sect_size) < 0)
		    HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "can't query last section on merge list")

		/* Deduct space from previous accumulation if the section is at EOA */
		if(H5F_addr_defined(sect_addr) && H5F_addr_eq(sect_addr + sect_size, eoa)) {
		    eoa = sect_addr;
		    eoa_shrank = TRUE;
		    tot_fs_size -= sect_size;
		} /* end if */
	    } /* end if */
	} /* end for */

	if(!H5F_PAGED_AGGR(f)) {
	    /* Check the metadata and raw data aggregators */
	    if(ma_size > 0 && H5F_addr_eq(ma_addr + ma_size, eoa)) {
		eoa = ma_addr;
		eoa_shrank = TRUE;
		ma_size = 0;
	    } /* end if */
	    if(sda_size > 0 && H5F_addr_eq(sda_addr + sda_size, eoa)) {
		eoa = sda_addr;
		eoa_shrank = TRUE;
		sda_size = 0;
	    } /* end if */
	} /* end if */
    } while(eoa_shrank);

    /* Close the free-space managers if they were opened earlier in this routine */
    for(type = start_type; type < end_type; H5_INC_ENUM(H5FD_mem_t, type)) {
	if(fs_started[type])
            if(H5MF_close_fstype(f, dxpl_id, type) < 0)
                HGOTO_ERROR(H5E_RESOURCE, H5E_CANTINIT, FAIL, "can't close file free space")
    } /* end for */

    /* Set the value(s) to return */
    /* (The metadata & small data aggregators count as free space now, since they aren't at EOA) */
    if(tot_space)
	*tot_space = tot_fs_size + ma_size + sda_size;
    if(meta_size)
	*meta_size = tot_meta_size;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MF_get_freespace() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_get_free_sections()
 *
 * Purpose: 	To retrieve free-space section information for
 *		paged or non-paged aggregation
 *
 * Return:	SUCCEED/FAIL
 *
 * Programmer:  Vailin Choi; Dec 2012
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5MF_get_free_sections(H5F_t *f, hid_t dxpl_id, H5F_fspace_type_t type, size_t nsects, H5F_sect_info_t *sect_info)
{
    size_t total_sects = 0;			/* Total number of sections */
    H5MF_sect_iter_ud_t sect_udata;     	/* User data for callback */
    H5F_mem_page_t start_type, end_type;   	/* Memory types to iterate over */
    H5F_mem_page_t ty;     			/* Memory type for iteration */
    ssize_t ret_value;         			/* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* check args */
    HDassert(f);
    HDassert(f->shared);
    HDassert(f->shared->lf);

    /* Determine start/end points for loop */
    if(H5F_PAGED_AGGR(f)) { /* Paged aggregation */
	if(type == H5F_FSPACE_TYPE_ALL) {
	    start_type = H5F_MEM_PAGE_META;
	    end_type = H5F_MEM_PAGE_NTYPES;
	} /* end if */
        else {
	    start_type = end_type = (H5F_mem_page_t)type;
	    H5_INC_ENUM(H5F_mem_page_t, end_type);
	} /* end else */
    } /* end if */
    else { /* Non-paged aggregation */
	if(type == H5F_FSPACE_TYPE_GENERIC)
	    HGOTO_DONE(0)
        else
            if(type == H5F_FSPACE_TYPE_RAW) { /* H5FD_MEM_DRAW, H5FD_MEM_GHEAP */
                start_type = (H5F_mem_page_t)H5FD_MEM_DRAW;
                end_type = (H5F_mem_page_t)H5FD_MEM_GHEAP;
                H5_INC_ENUM(H5F_mem_page_t, end_type);
            } /* end if */
            else { /* H5F_FSPACE_TYPE_ALL, H5F_FSPACE_TYPE_META (will skip raw data when looping) */
                start_type = (H5F_mem_page_t)H5FD_MEM_SUPER;
                end_type = (H5F_mem_page_t)H5FD_MEM_NTYPES;
            } /* end else */
    } /* end else */

    /* Set up user data for section iteration */
    sect_udata.sects = sect_info;
    sect_udata.sect_count = nsects;
    sect_udata.sect_idx = 0;

    /* Iterate over memory types, retrieving the number of sections of each type */
    for(ty = start_type; ty < end_type; H5_INC_ENUM(H5F_mem_page_t, ty)) {
	hbool_t fs_started = FALSE;	/* The free-space manager is opened or not */
	size_t nums = 0;		/* The number of free-space sections */

	if(!H5F_PAGED_AGGR(f))
	    if(type == H5F_FSPACE_TYPE_META && (ty == (H5F_mem_page_t)H5FD_MEM_DRAW || ty == (H5F_mem_page_t)H5FD_MEM_GHEAP))
		continue;

	if(!f->shared->fs.man[ty] && H5F_addr_defined(f->shared->fs.man_addr[ty])) {
	    if(H5MF_open_fstype(f, dxpl_id, (H5FD_mem_t)ty) < 0)
		HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't open the free space manager")
	    HDassert(f->shared->fs.man[ty]);
            fs_started = TRUE;
	} /* end if */

	/* Check if there's free space sections of this type */
	if(f->shared->fs.man[ty])
	    if(H5MF_get_free_sects(f, dxpl_id, f->shared->fs.man[ty], &sect_udata, &nums) < 0)
		HGOTO_ERROR(H5E_FSPACE, H5E_CANTRELEASE, FAIL, "can't get section info for the free space manager")

        /* Increment total # of sections */
	total_sects += nums;

	/* Close the free space manager of this type, if we started it here */
        if(fs_started)
            if(H5MF_close_fstype(f, dxpl_id, (H5FD_mem_t)ty) < 0)
	        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTCLOSEOBJ, FAIL, "can't close file free space")
    } /* end for */

    /* Set return value */
    ret_value = (ssize_t)total_sects;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5MF_get_free_sections() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_sects_cb()
 *
 * Purpose:	Iterator callback for each free-space section
 *		Retrieve address and size into user data
 *
 * Return:	Always succeed
 *
 * Programmer:  Vailin Choi
 *	        July 1st, 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_sects_cb(H5FS_section_info_t *_sect, void *_udata)
{
    H5MF_free_section_t *sect = (H5MF_free_section_t *)_sect;
    H5MF_sect_iter_ud_t *udata = (H5MF_sect_iter_ud_t *)_udata;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    if(udata->sect_idx < udata->sect_count) {
        udata->sects[udata->sect_idx].addr = sect->sect_info.addr;
        udata->sects[udata->sect_idx].size  = sect->sect_info.size;
        udata->sect_idx++;
    } /* end if */

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* H5MF_sects_cb() */


/*-------------------------------------------------------------------------
 * Function:    H5MF_get_free_sects
 *
 * Purpose:	Retrieve section information for the specified free-space manager.
 *
 * Return:      Success:        non-negative
 *              Failure:        negative
 *
 * Programmer:  Vailin Choi; Dec 2012
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5MF_get_free_sects(H5F_t *f, hid_t dxpl_id, H5FS_t *fspace, H5MF_sect_iter_ud_t *sect_udata, size_t *nums)
{
    hsize_t hnums = 0;          	/* # of sections */
    herr_t ret_value = SUCCEED; 	/* Return value */

    FUNC_ENTER_NOAPI(FAIL)

    /* check args */
    HDassert(f);
    HDassert(sect_udata);
    HDassert(nums);
    HDassert(fspace);

    /* Query how many sections of this type */
    if(H5FS_sect_stats(fspace, NULL, &hnums) < 0)
	HGOTO_ERROR(H5E_RESOURCE, H5E_CANTGET, FAIL, "can't query free space stats")
    H5_ASSIGN_OVERFLOW(*nums, hnums, hsize_t, size_t);

    /* Check if we should retrieve the section info */
    if(sect_udata->sects && *nums > 0) {
	/* Iterate over all the free space sections of this type, adding them to the user's section info */
	if(H5FS_sect_iterate(f, dxpl_id, fspace, H5MF_sects_cb, sect_udata) < 0)
	    HGOTO_ERROR(H5E_RESOURCE, H5E_BADITER, FAIL, "can't iterate over sections")
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5MF_get_free_sects() */

