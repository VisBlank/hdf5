/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:		H5HFdtable.c
 *			Apr 10 2006
 *			Quincey Koziol <koziol@ncsa.uiuc.edu>
 *
 * Purpose:		"Doubling table" routines for fractal heaps.
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/

#define H5HF_PACKAGE		/*suppress error about including H5HFpkg  */

/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5HFpkg.h"		/* Fractal heaps			*/
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5Vprivate.h"		/* Vectors and arrays 			*/

/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/


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
 * Function:	H5HF_dtable_init
 *
 * Purpose:	Initialize values for doubling table
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Mar  6 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_dtable_init(H5HF_dtable_t *dtable)
{
    hsize_t tmp_block_size;             /* Temporary block size */
    hsize_t acc_block_off;              /* Accumulated block offset */
    size_t u;                           /* Local index variable */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5HF_dtable_init)

    /*
     * Check arguments.
     */
    HDassert(dtable);

    /* Compute/cache some values */
    dtable->first_row_bits = H5V_log2_of2(dtable->cparam.start_block_size) +
            H5V_log2_of2(dtable->cparam.width);
    dtable->max_root_rows = (dtable->cparam.max_index - dtable->first_row_bits) + 1;
    dtable->max_direct_rows = (H5V_log2_of2(dtable->cparam.max_direct_size) -
            H5V_log2_of2(dtable->cparam.start_block_size)) + 2;
    dtable->num_id_first_row = dtable->cparam.start_block_size * dtable->cparam.width;
    dtable->max_dir_blk_off_size = H5HF_SIZEOF_OFFSET_LEN(dtable->cparam.max_direct_size);

    /* Build table of block sizes for each row */
    if(NULL == (dtable->row_block_size = H5MM_malloc(dtable->max_root_rows * sizeof(hsize_t))))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "can't create doubling table block size table")
    if(NULL == (dtable->row_block_off = H5MM_malloc(dtable->max_root_rows * sizeof(hsize_t))))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "can't create doubling table block offset table")
    if(NULL == (dtable->row_dblock_free = H5MM_malloc(dtable->max_root_rows * sizeof(hsize_t))))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "can't create doubling table direct block free space table")
    tmp_block_size = dtable->cparam.start_block_size;
    acc_block_off = dtable->cparam.start_block_size * dtable->cparam.width;
    dtable->row_block_size[0] = dtable->cparam.start_block_size;
    dtable->row_block_off[0] = 0;
    for(u = 1; u < dtable->max_root_rows; u++) {
        dtable->row_block_size[u] = tmp_block_size;
        dtable->row_block_off[u] = acc_block_off;
        tmp_block_size *= 2;
        acc_block_off *= 2;
    } /* end for */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5HF_dtable_init() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_dtable_lookup
 *
 * Purpose:	Compute the row & col of an offset in a doubling-table
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Mar  6 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_dtable_lookup(const H5HF_dtable_t *dtable, hsize_t off, unsigned *row, unsigned *col)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5HF_dtable_lookup)

    /*
     * Check arguments.
     */
    HDassert(dtable);
    HDassert(row);
    HDassert(col);

    /* Check for offset in first row */
    if(off < dtable->num_id_first_row) {
        *row = 0;
        *col = off / dtable->cparam.start_block_size;
    } /* end if */
    else {
        unsigned high_bit = H5V_log2_gen(off);  /* Determine the high bit in the offset */
        hsize_t off_mask = 1 << high_bit;       /* Compute mask for determining column */

        *row = (high_bit - dtable->first_row_bits) + 1;
        *col = (off - off_mask) / dtable->row_block_size[*row];
    } /* end else */

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5HF_dtable_lookup() */


/*-------------------------------------------------------------------------
 * Function:	H5HF_dtable_dest
 *
 * Purpose:	Release information for doubling table
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Mar 27 2006
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5HF_dtable_dest(H5HF_dtable_t *dtable)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5HF_dtable_dest)

    /*
     * Check arguments.
     */
    HDassert(dtable);

    /* Free the block size lookup table for the doubling table */
    H5MM_xfree(dtable->row_block_size);

    /* Free the block offset lookup table for the doubling table */
    H5MM_xfree(dtable->row_block_off);

    /* Free the direct block free space lookup table for the doubling table */
    H5MM_xfree(dtable->row_dblock_free);

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5HF_dtable_dest() */

