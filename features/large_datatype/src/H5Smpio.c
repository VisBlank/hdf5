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

/*
 * Programmer:  rky 980813
 *
 * Purpose:	Functions to read/write directly between app buffer and file.
 *
 * 		Beware of the ifdef'ed print statements.
 *		I didn't make them portable.
 */

#define H5S_PACKAGE		/*suppress error about including H5Spkg	  */


#include "H5private.h"		/* Generic Functions			*/
#include "H5Dprivate.h"		/* Datasets				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fprivate.h"		/* File access				*/
#include "H5FDprivate.h"	/* File drivers				*/
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5MMprivate.h"        /* Memory management                    */
#include "H5Oprivate.h"		/* Object headers		  	*/
#include "H5Pprivate.h"         /* Property lists                       */
#include "H5Spkg.h"		/* Dataspaces 				*/
#include "H5Vprivate.h"		/* Vector and array functions		*/

#ifdef H5_HAVE_PARALLEL

static herr_t H5S_mpio_all_type(const H5S_t *space, size_t elmt_size,
    MPI_Datatype *new_type, int *count, hbool_t *is_derived_type);
static herr_t H5S_mpio_none_type(MPI_Datatype *new_type, int *count,
    hbool_t *is_derived_type);
static herr_t H5S_mpio_hyper_type(const H5S_t *space, size_t elmt_size,
    MPI_Datatype *new_type, int *count, hbool_t *is_derived_type);
static herr_t H5S_mpio_span_hyper_type(const H5S_t *space, size_t elmt_size,
    MPI_Datatype *new_type, int *count, hbool_t *is_derived_type);
static herr_t H5S_obtain_datatype(const hsize_t down[], H5S_hyper_span_t* span,
    const MPI_Datatype *elmt_type, MPI_Datatype *span_type, size_t elmt_size);
static herr_t H5S_mpio_create_large_type (hsize_t total_bytes, MPI_Aint strid_bytes,
                                          MPI_Datatype old_type, MPI_Datatype *new_type);

#define H5S_MPIO_INITIAL_ALLOC_COUNT    256


/*-------------------------------------------------------------------------
 * Function:	H5S_mpio_all_type
 *
 * Purpose:	Translate an HDF5 "all" selection into an MPI type.
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Outputs:	*new_type	  the MPI type corresponding to the selection
 *		*count		  how many objects of the new_type in selection
 *				  (useful if this is the buffer type for xfer)
 *		*is_derived_type  0 if MPI primitive type, 1 if derived
 *
 * Programmer:	rky 980813
 *
 * Modifications:
 *              Mohamad Chaarawi
 *              Adding support for large datatypes (beyond the limit of a 
 *              32 bit integer.
 *-------------------------------------------------------------------------
 */
static herr_t
H5S_mpio_all_type(const H5S_t *space, size_t elmt_size,
    MPI_Datatype *new_type, int *count, hbool_t *is_derived_type)
{
    hsize_t	total_bytes;
    hssize_t	snelmts;                /* Total number of elmts	(signed) */
    hsize_t	nelmts;                 /* Total number of elmts	*/
    herr_t	ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5S_mpio_all_type)

    /* Check args */
    HDassert(space);

    /* Just treat the entire extent as a block of bytes */
    if((snelmts = (hssize_t)H5S_GET_EXTENT_NPOINTS(space)) < 0)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "src dataspace has invalid selection")
    H5_ASSIGN_OVERFLOW(nelmts, snelmts, hssize_t, hsize_t);

    total_bytes = (hsize_t)elmt_size * nelmts;

    /* Check if the total bytes to be selected fits into a 32 bit integer 
     * If yes then just set the new_type to MPI_BYTE and count to total_bytes
     * Otherwise create a derived datatype by iterating as many times as needed
     * in order for a selection greater than 2^31 Bytes to be possible */
    if(2147483647 >= total_bytes) {
        printf("SMALL ALL SELECTION\n");
        /* fill in the return values */
        *new_type = MPI_BYTE;
        H5_ASSIGN_OVERFLOW(*count, total_bytes, hsize_t, int);
        *is_derived_type = FALSE;
    }
    else {
        printf ("LARGE ALL SELECTION\n");
        if (H5S_mpio_create_large_type (total_bytes, 0, MPI_BYTE, new_type) < 0) {
            HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,
                        "couldn't ccreate a large datatype from the all selection")
        }
        *count = 1;
        *is_derived_type = TRUE;
    }

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5S_mpio_all_type() */


/*-------------------------------------------------------------------------
 * Function:	H5S_mpio_none_type
 *
 * Purpose:	Translate an HDF5 "none" selection into an MPI type.
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Outputs:	*new_type	  the MPI type corresponding to the selection
 *		*count		  how many objects of the new_type in selection
 *				  (useful if this is the buffer type for xfer)
 *		*is_derived_type  0 if MPI primitive type, 1 if derived
 *
 * Programmer:	Quincey Koziol, October 29, 2002
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5S_mpio_none_type(MPI_Datatype *new_type, int *count, hbool_t *is_derived_type)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5S_mpio_none_type)

    /* fill in the return values */
    *new_type = MPI_BYTE;
    *count = 0;
    *is_derived_type = FALSE;

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* H5S_mpio_none_type() */


/*-------------------------------------------------------------------------
 * Function:	H5S_mpio_hyper_type
 *
 * Purpose:	Translate an HDF5 hyperslab selection into an MPI type.
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Outputs:	*new_type	  the MPI type corresponding to the selection
 *		*count		  how many objects of the new_type in selection
 *				  (useful if this is the buffer type for xfer)
 *		*is_derived_type  0 if MPI primitive type, 1 if derived
 *
 * Programmer:	rky 980813
 *
 * Modifications:
 *              Mohamad Chaarawi
 *              Adding support for large datatypes (beyond the limit of a 
 *              32 bit integer.
 *-------------------------------------------------------------------------
 */
static herr_t
H5S_mpio_hyper_type(const H5S_t *space, size_t elmt_size,
    MPI_Datatype *new_type, int *count, hbool_t *is_derived_type)
{
    H5S_sel_iter_t sel_iter;    /* Selection iteration info */
    hbool_t sel_iter_init = FALSE;    /* Selection iteration info has been initialized */

    struct dim {	/* less hassle than malloc/free & ilk */
        hssize_t start;
        hsize_t strid;
        hsize_t block;
        hsize_t xtent;
        hsize_t count;
    } d[H5S_MAX_RANK];

    hsize_t		offset[H5S_MAX_RANK];
    hsize_t		max_xtent[H5S_MAX_RANK];
    H5S_hyper_dim_t	*diminfo;		/* [rank] */
    unsigned		rank;
    int			block_length[3];
    MPI_Datatype	inner_type, outer_type, old_types[3];
    MPI_Aint            extent_len, displacement[3];
    unsigned		u;			/* Local index variable */
    int			i;			/* Local index variable */
    int                 mpi_code;               /* MPI return code */
    herr_t		ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT(H5S_mpio_hyper_type)

    /* Check args */
    HDassert(space);
    HDassert(sizeof(MPI_Aint) >= sizeof(elmt_size));

    /* Initialize selection iterator */
    if(H5S_select_iter_init(&sel_iter, space, elmt_size) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTINIT, FAIL, "unable to initialize selection iterator")
    sel_iter_init = TRUE;	/* Selection iteration info has been initialized */

    /* Abbreviate args */
    diminfo = sel_iter.u.hyp.diminfo;
    HDassert(diminfo);

    /* make a local copy of the dimension info so we can operate with them */

    /* Check if this is a "flattened" regular hyperslab selection */
    if(sel_iter.u.hyp.iter_rank != 0 && sel_iter.u.hyp.iter_rank < space->extent.rank) {
        /* Flattened selection */
        rank = sel_iter.u.hyp.iter_rank;
        HDassert(rank <= H5S_MAX_RANK);	/* within array bounds */
#ifdef H5S_DEBUG
  if(H5DEBUG(S))
            HDfprintf(H5DEBUG(S), "%s: Flattened selection\n",FUNC);
#endif
        for(u = 0; u < rank; ++u) {
            H5_CHECK_OVERFLOW(diminfo[u].start, hsize_t, hssize_t)
            d[u].start = (hssize_t)diminfo[u].start + sel_iter.u.hyp.sel_off[u];
            d[u].strid = diminfo[u].stride;
            d[u].block = diminfo[u].block;
            d[u].count = diminfo[u].count;
            d[u].xtent = sel_iter.u.hyp.size[u];
#ifdef H5S_DEBUG
       if(H5DEBUG(S)){
            HDfprintf(H5DEBUG(S), "%s: start=%Hd  stride=%Hu  count=%Hu  block=%Hu  xtent=%Hu",
                FUNC, d[u].start, d[u].strid, d[u].count, d[u].block, d[u].xtent );
            if (u==0)
                HDfprintf(H5DEBUG(S), "  rank=%u\n", rank );
            else
                HDfprintf(H5DEBUG(S), "\n" );
      }
#endif
            if(0 == d[u].block)
                goto empty;
            if(0 == d[u].count)
                goto empty;
            if(0 == d[u].xtent)
                goto empty;
        } /* end for */
    } /* end if */
    else {
        /* Non-flattened selection */
        rank = space->extent.rank;
        HDassert(rank <= H5S_MAX_RANK);	/* within array bounds */
        if(0 == rank)
            goto empty;
#ifdef H5S_DEBUG
  if(H5DEBUG(S))
            HDfprintf(H5DEBUG(S),"%s: Non-flattened selection\n",FUNC);
#endif
        for(u = 0; u < rank; ++u) {
            H5_CHECK_OVERFLOW(diminfo[u].start, hsize_t, hssize_t)
            d[u].start = (hssize_t)diminfo[u].start + space->select.offset[u];
            d[u].strid = diminfo[u].stride;
            d[u].block = diminfo[u].block;
            d[u].count = diminfo[u].count;
            d[u].xtent = space->extent.size[u];
#ifdef H5S_DEBUG
  if(H5DEBUG(S)){
    HDfprintf(H5DEBUG(S), "%s: start=%Hd  stride=%Hu  count=%Hu  block=%Hu  xtent=%Hu",
              FUNC, d[u].start, d[u].strid, d[u].count, d[u].block, d[u].xtent );
    if (u==0)
        HDfprintf(H5DEBUG(S), "  rank=%u\n", rank );
    else
        HDfprintf(H5DEBUG(S), "\n" );
  }
#endif
            if(0 == d[u].block)
                goto empty;
            if(0 == d[u].count)
                goto empty;
            if(0 == d[u].xtent)
                goto empty;
        } /* end for */
    } /* end else */

/**********************************************************************
    Compute array "offset[rank]" which gives the offsets for a multi-
    dimensional array with dimensions "d[i].xtent" (i=0,1,...,rank-1).
**********************************************************************/
    offset[rank - 1] = 1;
    max_xtent[rank - 1] = d[rank - 1].xtent;
#ifdef H5S_DEBUG
  if(H5DEBUG(S)) {
     i = ((int)rank) - 1;
     HDfprintf(H5DEBUG(S), " offset[%2d]=%Hu; max_xtent[%2d]=%Hu\n",
                          i, offset[i], i, max_xtent[i]);
  }
#endif
    for(i = ((int)rank) - 2; i >= 0; --i) {
        offset[i] = offset[i + 1] * d[i + 1].xtent;
        max_xtent[i] = max_xtent[i + 1] * d[i].xtent;
#ifdef H5S_DEBUG
  if(H5DEBUG(S))
    HDfprintf(H5DEBUG(S), " offset[%2d]=%Hu; max_xtent[%2d]=%Hu\n",
                          i, offset[i], i, max_xtent[i]);
#endif
    } /* end for */

    /*  Create a type covering the selected hyperslab.
     *  Multidimensional dataspaces are stored in row-major order.
     *  The type is built from the inside out, going from the
     *  fastest-changing (i.e., inner) dimension * to the slowest (outer). */

/*******************************************************
*  Construct contig type for inner contig dims:
*******************************************************/
#ifdef H5S_DEBUG
  if(H5DEBUG(S)) {
    HDfprintf(H5DEBUG(S), "%s: Making contig type %Zu MPI_BYTEs\n", FUNC, elmt_size);
    for(i = ((int)rank) - 1; i >= 0; --i)
        HDfprintf(H5DEBUG(S), "d[%d].xtent=%Hu \n", i, d[i].xtent);
  }
#endif

    /* Check if the number of elements to form the inner type fits into a 32 bit integer.
     * If yes then just create the innertype with MPI_Type_contiguous.
     * Otherwise create a compound datatype by iterating as many times as needed
     * in order for the innertype to be created */
    if(2147483647 >= elmt_size) {
        if(MPI_SUCCESS != (mpi_code = MPI_Type_contiguous((int)elmt_size, MPI_BYTE, 
                                                          &inner_type)))
            HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
    }
    else {
        printf ("LARGE inner selection in hyper_type\n");
        if (H5S_mpio_create_large_type (elmt_size, 0, MPI_BYTE, &inner_type) < 0) {
            HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,
                        "couldn't ccreate a large inner datatype in hyper selection")
        }
    }

/*******************************************************
*  Construct the type by walking the hyperslab dims
*  from the inside out:
*******************************************************/
    for(i = ((int)rank) - 1; i >= 0; --i) {
#ifdef H5S_DEBUG
  if(H5DEBUG(S))
    HDfprintf(H5DEBUG(S), "%s: Dimension i=%d \n"
            "start=%Hd count=%Hu block=%Hu stride=%Hu, xtent=%Hu max_xtent=%d\n",
            FUNC, i, d[i].start, d[i].count, d[i].block, d[i].strid, d[i].xtent, max_xtent[i]);
#endif

#ifdef H5S_DEBUG
  if(H5DEBUG(S))
    HDfprintf(H5DEBUG(S), "%s: i=%d  Making vector-type \n", FUNC,i);
#endif
       /****************************************
       *  Build vector type of the selection.
       ****************************************/

        /* If all the parameters fit into 32 bit integers create the vector type normally */
        if (2147483647 >= d[i].count && 
            2147483647 >= d[i].block && 
            2147483647 >= d[i].strid) {
            mpi_code = MPI_Type_vector((int)(d[i].count),   /* count */
                                       (int)(d[i].block),   /* blocklength */
                                       (int)(d[i].strid),   /* stride */
                                       inner_type,	    /* old type */
                                       &outer_type);        /* new type */
            if(MPI_SUCCESS != mpi_code)
                HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
        }
        /* Otherwise do this complicated little datatype processing */
        else {
            MPI_Aint strid_in_bytes, inner_extent;
            MPI_Datatype block_type;

            /* create a contiguous datatype inner_type x number of blocks.
             * Again need to check if the number of blocks can fit into 
             * the 32 bit integer */
            if (2147483647 < d[i].block) {
                printf ("LARGE block selection in hyper_type\n");
                if (H5S_mpio_create_large_type(d[i].block, 0, inner_type, 
                                               &block_type) < 0) {
                    HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,
                                "couldn't ccreate a large block datatype in hyper selection")
                }
            }
            else {
                if(MPI_SUCCESS != (mpi_code = MPI_Type_contiguous((int)d[i].block, 
                                                                  inner_type, 
                                                                  &block_type)))
                    HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
            }

            MPI_Type_extent (inner_type, &inner_extent);
            strid_in_bytes = inner_extent * (MPI_Aint)d[i].strid;

            /* Now if the count is larger than what a 32 bit integer can hold,
             * call the large creation function to handle that */
            if (2147483647 < d[i].count) {
                printf ("LARGE outer selection in hyper_type\n");
                if (H5S_mpio_create_large_type (d[i].count, strid_in_bytes, block_type,
                                                &outer_type) < 0) {
                    HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,
                                "couldn't ccreate a large outer datatype in hyper selection")
                }
            }
            /* otherwise a regular create_hvector will do */
            else {
                mpi_code = MPI_Type_create_hvector((int)d[i].count,/* count */
                                                   1,              /* blocklength */
                                                   strid_in_bytes, /* stride in bytes*/
                                                   block_type,	   /* old type */
                                                   &outer_type);   /* new type */
                if(MPI_SUCCESS != mpi_code)
                    HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
            }
            MPI_Type_free(&block_type);
        }

        MPI_Type_free(&inner_type);
        if(mpi_code != MPI_SUCCESS)
            HMPI_GOTO_ERROR(FAIL, "couldn't create MPI vector type", mpi_code)

        /****************************************
        *  Then build the dimension type as (start, vector type, xtent).
        ****************************************/
        /* calculate start and extent values of this dimension */
	displacement[1] = d[i].start * offset[i] * elmt_size;
        displacement[2] = (MPI_Aint)elmt_size * max_xtent[i];
        if(MPI_SUCCESS != (mpi_code = MPI_Type_extent(outer_type, &extent_len)))
            HMPI_GOTO_ERROR(FAIL, "MPI_Type_extent failed", mpi_code)

        /*************************************************
        *  Restructure this datatype ("outer_type")
        *  so that it still starts at 0, but its extent
        *  is the full extent in this dimension.
        *************************************************/
        if(displacement[1] > 0 || (int)extent_len < displacement[2]) {

            block_length[0] = 1;
            block_length[1] = 1;
            block_length[2] = 1;

            displacement[0] = 0;

            old_types[0] = MPI_LB;
            old_types[1] = outer_type;
            old_types[2] = MPI_UB;
#ifdef H5S_DEBUG
  if(H5DEBUG(S))
    HDfprintf(H5DEBUG(S), "%s: i=%d Extending struct type\n"
        "***displacements: %ld, %ld, %ld\n",
        FUNC, i, (long)displacement[0], (long)displacement[1], (long)displacement[2]);
#endif

            mpi_code = MPI_Type_struct(3,               /* count */
                                       block_length,    /* blocklengths */
                                       displacement,    /* displacements */
                                       old_types,       /* old types */
                                       &inner_type);    /* new type */

            MPI_Type_free(&outer_type);
    	    if(mpi_code != MPI_SUCCESS)
                HMPI_GOTO_ERROR(FAIL, "couldn't resize MPI vector type", mpi_code)
        } /* end if */
        else
            inner_type = outer_type;
    } /* end for */
/***************************
*  End of loop, walking
*  thru dimensions.
***************************/

    /* At this point inner_type is actually the outermost type, even for 0-trip loop */
    *new_type = inner_type;
    if(MPI_SUCCESS != (mpi_code = MPI_Type_commit(new_type)))
        HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code)

    /* fill in the remaining return values */
    *count = 1;			/* only have to move one of these suckers! */
    *is_derived_type = TRUE;
    HGOTO_DONE(SUCCEED);

empty:
    /* special case: empty hyperslab */
    *new_type = MPI_BYTE;
    *count = 0;
    *is_derived_type = FALSE;

done:
    /* Release selection iterator */
    if(sel_iter_init)
        if(H5S_SELECT_ITER_RELEASE(&sel_iter) < 0)
            HDONE_ERROR(H5E_DATASPACE, H5E_CANTRELEASE, FAIL, "unable to release selection iterator")

#ifdef H5S_DEBUG
  if(H5DEBUG(S))
    HDfprintf(H5DEBUG(S), "Leave %s, count=%ld  is_derived_type=%t\n",
		FUNC, *count, *is_derived_type );
#endif
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5S_mpio_hyper_type() */


/*-------------------------------------------------------------------------
 * Function:	H5S_mpio_span_hyper_type
 *
 * Purpose:	Translate an HDF5 irregular hyperslab selection into an
                MPI type.
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Outputs:	*new_type	  the MPI type corresponding to the selection
 *		*count		  how many objects of the new_type in selection
 *				  (useful if this is the buffer type for xfer)
 *		*is_derived_type  0 if MPI primitive type, 1 if derived
 *
 * Programmer:  kyang
 *
 * Modifications:
 *              Mohamad Chaarawi
 *              Adding support for large datatypes (beyond the limit of a 
 *              32 bit integer.
 *-------------------------------------------------------------------------
 */
static herr_t
H5S_mpio_span_hyper_type(const H5S_t *space, size_t elmt_size,
    MPI_Datatype *new_type, int *count, hbool_t *is_derived_type)
{
    MPI_Datatype  elmt_type;            /* MPI datatype for an element */
    hbool_t elmt_type_is_derived = FALSE;       /* Whether the element type has been created */
    MPI_Datatype  span_type;            /* MPI datatype for overall span tree */
    hsize_t       down[H5S_MAX_RANK];   /* 'down' sizes for each dimension */
    int           mpi_code;             /* MPI return code */
    herr_t        ret_value = SUCCEED;  /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5S_mpio_span_hyper_type)

    /* Check args */
    HDassert(space);
    HDassert(space->extent.size);
    HDassert(space->select.sel_info.hslab->span_lst);
    HDassert(space->select.sel_info.hslab->span_lst->head);

    /* Create the base type for an element while checking the 32-bit int limit size*/
    if (2147483647 >= elmt_size) {
        if(MPI_SUCCESS != (mpi_code = MPI_Type_contiguous((int)elmt_size, 
                                                          MPI_BYTE, &elmt_type)))
            HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
    }
    else {
        printf ("LARGE element selection in span_hyper\n");
        if (H5S_mpio_create_large_type (elmt_size, 0, MPI_BYTE, &elmt_type) < 0) {
            HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,
                        "couldn't create a large element datatype in span_hyper selection")
        }
    }
    elmt_type_is_derived = TRUE;

    /* Compute 'down' sizes for each dimension */
    if(H5V_array_down(space->extent.rank, space->extent.size, down) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTGETSIZE, FAIL, "couldn't compute 'down' dimension sizes")

    /* Obtain derived data type */
    if(H5S_obtain_datatype(down, space->select.sel_info.hslab->span_lst->head, &elmt_type, &span_type, elmt_size) < 0)
        HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL, "couldn't obtain  MPI derived data type")
    if(MPI_SUCCESS != (mpi_code = MPI_Type_commit(&span_type)))
        HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code)
    *new_type = span_type;

    /* fill in the remaining return values */
    *count = 1;
    *is_derived_type = TRUE;

done:
    /* Release resources */
    if(elmt_type_is_derived)
        if(MPI_SUCCESS != (mpi_code = MPI_Type_free(&elmt_type)))
            HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code)

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5S_mpio_span_hyper_type() */


/*-------------------------------------------------------------------------
 * Function:	H5S_obtain datatype
 *
 * Purpose:	Obtain an MPI derived datatype based on span-tree
 *              implementation
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Outputs:	*span_type	 the MPI type corresponding to the selection
 *
 * Programmer:  kyang
 *
 * Modifications:
 *              Mohamad Chaarawi
 *              Adding support for large datatypes (beyond the limit of a 
 *              32 bit integer.
 *-------------------------------------------------------------------------
 */
static herr_t
H5S_obtain_datatype(const hsize_t *down, H5S_hyper_span_t *span,
    const MPI_Datatype *elmt_type, MPI_Datatype *span_type, size_t elmt_size)
{
    size_t                alloc_count;          /* Number of span tree nodes allocated at this level */
    size_t                outercount;           /* Number of span tree nodes at this level */
    MPI_Datatype          *inner_type = NULL;
    hbool_t inner_types_freed = FALSE;          /* Whether the inner_type MPI datatypes have been freed */
    hbool_t span_type_valid = FALSE;            /* Whether the span_type MPI datatypes is valid */
    hbool_t large_block = FALSE;                /* Wether the block length is larger than 32 bit integer */
    hsize_t                   *blocklen = NULL; /* MSC - changed from int to size_t because we don't want 
                                                 * to case tspan->nelem to a 32 bit int */
    MPI_Aint              *disp = NULL;
    H5S_hyper_span_t      *tspan;               /* Temporary pointer to span tree node */
    int                   mpi_code;             /* MPI return status code */
    herr_t                ret_value = SUCCEED;  /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5S_obtain_datatype)

    /* Sanity check */
    HDassert(span);

    /* Allocate the initial displacement & block length buffers */
    alloc_count = H5S_MPIO_INITIAL_ALLOC_COUNT;
    if(NULL == (disp = (MPI_Aint *)H5MM_malloc(alloc_count * sizeof(MPI_Aint))))
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTALLOC, FAIL, "can't allocate array of displacements")
    if(NULL == (blocklen = (hsize_t *)H5MM_malloc(alloc_count * sizeof(hsize_t))))
        HGOTO_ERROR(H5E_DATASPACE, H5E_CANTALLOC, FAIL, "can't allocate array of block lengths")

    /* if this is the fastest changing dimension, it is the base case for derived datatype. */
    if(NULL == span->down) {
        tspan = span;
        outercount = 0;
        while(tspan) {
            /* Check if we need to increase the size of the buffers */
            if(outercount >= alloc_count) {
                MPI_Aint     *tmp_disp;         /* Temporary pointer to new displacement buffer */
                hsize_t      *tmp_blocklen;     /* Temporary pointer to new block length buffer */

                /* Double the allocation count */
                alloc_count *= 2;

                /* Re-allocate the buffers */
                if(NULL == (tmp_disp = (MPI_Aint *)H5MM_realloc(disp, alloc_count * sizeof(MPI_Aint))))
                    HGOTO_ERROR(H5E_DATASPACE, H5E_CANTALLOC, FAIL, "can't allocate array of displacements")
                disp = tmp_disp;
                if(NULL == (tmp_blocklen = (hsize_t *)H5MM_realloc(blocklen, alloc_count * sizeof(hsize_t))))
                    HGOTO_ERROR(H5E_DATASPACE, H5E_CANTALLOC, FAIL, "can't allocate array of block lengths")
                blocklen = tmp_blocklen;
            } /* end if */

            /* Store displacement & block length */
            disp[outercount]      = (MPI_Aint)elmt_size * tspan->low;

            /* H5_CHECK_OVERFLOW(tspan->nelem, hsize_t, int) */
            blocklen[outercount]  = tspan->nelem;

            if (2147483647 < blocklen[outercount]) {
                large_block = TRUE; /* at least one block type is large, so set this flag to true */
            }
            tspan                 = tspan->next;
            outercount++;
        } /* end while */

        /* everything fits into integers, so cast them and use hindexed */
        if (2147483647 >= outercount && large_block == FALSE) {
            /* MSC blocklen array here must be of type int... how to fix it?
             * make a casted copy?? urgghhh */
            int *tmp_blocklen;
            int i;
            tmp_blocklen = (int *)malloc ((int)outercount * sizeof(int));
            for (i=0 ; i<(int)outercount; i++) {
                tmp_blocklen[i] = (int)blocklen[i];
            }
            if(MPI_SUCCESS != (mpi_code = MPI_Type_hindexed((int)outercount, 
                                                            tmp_blocklen, 
                                                            disp, 
                                                            *elmt_type, 
                                                            span_type)))
                HMPI_GOTO_ERROR(FAIL, "MPI_Type_hindexed failed", mpi_code)
        }
        /* something doesn't fit */
        else {
            size_t i;
            for (i=0 ; i<outercount ; i++) {
                MPI_Datatype temp_type, outer_type;

                /* create the block type from elmt_type while checking the 32 bit int limit */
                if (blocklen[i] > 2147483647) {
                    if (H5S_mpio_create_large_type (blocklen[i], 0, *elmt_type, &temp_type) < 0) {
                        HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,
                                    "couldn't create a large element datatype in span_hyper selection")
                    }
                }
                else {
                    if(MPI_SUCCESS != (mpi_code = MPI_Type_contiguous((int)blocklen[i], 
                                                                      *elmt_type, 
                                                                      &temp_type)))
                        HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
                }

                /* combine the current datatype that is created with this current block type */
                if (0 == i) { /* first iteration, there is no combined datatype yet */
                    *span_type = temp_type;
                }
                else {
                    int bl[2] = {1,1};
                    MPI_Aint ds[2] = {disp[i-1],disp[i]};
                    MPI_Datatype dt[2] = {*span_type, temp_type};

                    if (MPI_SUCCESS != (mpi_code = MPI_Type_create_struct (2,              /* count */
                                                                           bl,             /* blocklength */
                                                                           ds,             /* stride in bytes*/
                                                                           dt,             /* old type */
                                                                           &outer_type))){ /* new type */
                        HMPI_GOTO_ERROR(FAIL, "MPI_Type_create_struct failed", mpi_code)
                    }
                }

                *span_type = outer_type;
                MPI_Type_free(&outer_type);
                MPI_Type_free(&temp_type);
            }
        }

        span_type_valid = TRUE;
    } /* end if */
    else {
        size_t u;               /* Local index variable */

        if(NULL == (inner_type = (MPI_Datatype *)H5MM_malloc(alloc_count * sizeof(MPI_Datatype))))
            HGOTO_ERROR(H5E_DATASPACE, H5E_CANTALLOC, FAIL, "can't allocate array of inner MPI datatypes")

        tspan = span;
        outercount = 0;
        while(tspan) {
            MPI_Datatype down_type;     /* Temporary MPI datatype for a span tree node's children */
            MPI_Aint stride;            /* Distance between inner MPI datatypes */

            /* Check if we need to increase the size of the buffers */
            if(outercount >= alloc_count) {
                MPI_Aint     *tmp_disp;         /* Temporary pointer to new displacement buffer */
                hsize_t      *tmp_blocklen;     /* Temporary pointer to new block length buffer */
                MPI_Datatype *tmp_inner_type;   /* Temporary pointer to inner MPI datatype buffer */

                /* Double the allocation count */
                alloc_count *= 2;

                /* Re-allocate the buffers */
                if(NULL == (tmp_disp = (MPI_Aint *)H5MM_realloc(disp, alloc_count * sizeof(MPI_Aint))))
                    HGOTO_ERROR(H5E_DATASPACE, H5E_CANTALLOC, FAIL, "can't allocate array of displacements")
                disp = tmp_disp;
                if(NULL == (tmp_blocklen = (hsize_t *)H5MM_realloc(blocklen, alloc_count * sizeof(hsize_t))))
                    HGOTO_ERROR(H5E_DATASPACE, H5E_CANTALLOC, FAIL, "can't allocate array of block lengths")
                blocklen = tmp_blocklen;
                if(NULL == (tmp_inner_type = (MPI_Datatype *)H5MM_realloc(inner_type, alloc_count * sizeof(MPI_Datatype))))
                    HGOTO_ERROR(H5E_DATASPACE, H5E_CANTALLOC, FAIL, "can't allocate array of inner MPI datatypes")
            } /* end if */

            /* Displacement should be in byte and should have dimension information */
            /* First using MPI Type vector to build derived data type for this span only */
            /* Need to calculate the disp in byte for this dimension. */
            /* Calculate the total bytes of the lower dimension */
            disp[outercount]      = tspan->low * (*down) * elmt_size;
            blocklen[outercount]  = 1;

            /* Generate MPI datatype for next dimension down */
            if(H5S_obtain_datatype(down + 1, tspan->down->head, elmt_type, &down_type, elmt_size) < 0)
                HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL, "couldn't obtain  MPI derived data type")

            /* Build the MPI datatype for this node */
            stride = (*down) * elmt_size;

            /* If all the parameters fit into 32 bit integers create the vector type normally */
            if (2147483647 >= tspan->nelem) {
                H5_CHECK_OVERFLOW(tspan->nelem, hsize_t, int)
                    if(MPI_SUCCESS != (mpi_code = MPI_Type_hvector((int)tspan->nelem, 
                                                                   1, 
                                                                   stride, 
                                                                   down_type, 
                                                                   &inner_type[outercount]))) {
                    HMPI_GOTO_ERROR(FAIL, "MPI_Type_hvector failed", mpi_code)
                } /* end if */
            }
            /* Otherwise do the complicated little datatype processing */
            else {
                if (H5S_mpio_create_large_type (tspan->nelem, stride, down_type,
                                                &inner_type[outercount]) < 0) {
                    HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,
                                "couldn't ccreate a large outer datatype in spanhyper selection")
                    }
            }

            /* Release MPI datatype for next dimension down */
            if(MPI_SUCCESS != (mpi_code = MPI_Type_free(&down_type)))
                HMPI_GOTO_ERROR(FAIL, "MPI_Type_free failed", mpi_code)

            tspan = tspan->next;
            outercount++;
         } /* end while */

        /* building the whole vector datatype */
        /* everything fits into integers, so cast them and use hindexed */
        if (2147483647 >= outercount && large_block == FALSE) {
            /* MSC blocklen array here must be of type int... how to fix it?? 
               make a casted copy?? urgghhh again */
            int *tmp_blocklen;
            int i;
            tmp_blocklen = (int *)malloc ((int)outercount * sizeof(int));
            for (i=0 ; i<(int)outercount; i++) {
                tmp_blocklen[i] = (int)blocklen[i];
            }

            if(MPI_SUCCESS != (mpi_code = MPI_Type_struct((int)outercount, 
                                                          tmp_blocklen, 
                                                          disp, 
                                                          inner_type, 
                                                          span_type)))
                HMPI_GOTO_ERROR(FAIL, "MPI_Type_struct failed", mpi_code)
        }
        /* something doesn't fit */
        else {
            size_t i;
            for (i=0 ; i<outercount ; i++) {
                MPI_Datatype temp_type, outer_type;

                /* create the block type from elmt_type while checking the 32 bit int limit */
                if (blocklen[i] > 2147483647) {
                    if (H5S_mpio_create_large_type (blocklen[i], 0, inner_type[i], &temp_type) < 0) {
                        HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,
                                    "couldn't create a large element datatype in span_hyper selection")
                    }
                }
                else {
                    if(MPI_SUCCESS != (mpi_code = MPI_Type_contiguous((int)blocklen[i], 
                                                                      inner_type[i], 
                                                                      &temp_type)))
                        HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
                }

                /* combine the current datatype that is created with this current block type */
                if (0 == i) { /* first iteration, there is no combined datatype yet */
                    *span_type = temp_type;
                }
                else {
                    int bl[2] = {1,1};
                    MPI_Aint ds[2] = {disp[i-1],disp[i]};
                    MPI_Datatype dt[2] = {*span_type, temp_type};

                    if (MPI_SUCCESS != (mpi_code = MPI_Type_create_struct (2,              /* count */
                                                                           bl,             /* blocklength */
                                                                           ds,             /* stride in bytes*/
                                                                           dt,             /* old type */
                                                                           &outer_type))){ /* new type */
                        HMPI_GOTO_ERROR(FAIL, "MPI_Type_create_struct failed", mpi_code)
                    }
                }

                *span_type = outer_type;
                MPI_Type_free(&outer_type);
                MPI_Type_free(&temp_type);
            }
        }

        span_type_valid = TRUE;

        /* Release inner node types */
        for(u = 0; u < outercount; u++)
            if(MPI_SUCCESS != (mpi_code = MPI_Type_free(&inner_type[u])))
                HMPI_GOTO_ERROR(FAIL, "MPI_Type_free failed", mpi_code)
        inner_types_freed = TRUE;
    } /* end else */

done:
    /* General cleanup */
    if(inner_type != NULL) {
        if(!inner_types_freed) {
            size_t u;          /* Local index variable */

            for(u = 0; u < outercount; u++)
                if(MPI_SUCCESS != (mpi_code = MPI_Type_free(&inner_type[u])))
                    HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code)
        } /* end if */

        H5MM_free(inner_type);
    } /* end if */
    if(blocklen != NULL)
        H5MM_free(blocklen);
    if(disp != NULL)
        H5MM_free(disp);

    /* Error cleanup */
    if(ret_value < 0) {
        if(span_type_valid)
            if(MPI_SUCCESS != (mpi_code = MPI_Type_free(span_type)))
                HMPI_DONE_ERROR(FAIL, "MPI_Type_free failed", mpi_code)
    } /* end if */

  FUNC_LEAVE_NOAPI(ret_value)
} /* end H5S_obtain_datatype() */


/*-------------------------------------------------------------------------
 * Function:	H5S_mpio_space_type
 *
 * Purpose:	Translate an HDF5 dataspace selection into an MPI type.
 *		Currently handle only hyperslab and "all" selections.
 *
 * Return:	non-negative on success, negative on failure.
 *
 * Outputs:	*new_type	  the MPI type corresponding to the selection
 *		*count		  how many objects of the new_type in selection
 *				  (useful if this is the buffer type for xfer)
 *		*is_derived_type  0 if MPI primitive type, 1 if derived
 *
 * Programmer:	rky 980813
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5S_mpio_space_type(const H5S_t *space, size_t elmt_size,
    MPI_Datatype *new_type, int *count, hbool_t *is_derived_type)
{
    herr_t	ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5S_mpio_space_type)

    /* Check args */
    HDassert(space);
    HDassert(elmt_size);

    /* Creat MPI type based on the kind of selection */
    switch(H5S_GET_EXTENT_TYPE(space)) {
        case H5S_NULL:
        case H5S_SCALAR:
        case H5S_SIMPLE:
            switch(H5S_GET_SELECT_TYPE(space)) {
                case H5S_SEL_NONE:
                    if(H5S_mpio_none_type(new_type, count, is_derived_type) < 0)
                        HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,"couldn't convert 'none' selection to MPI type")
                    break;

                case H5S_SEL_ALL:
                    if(H5S_mpio_all_type(space, elmt_size, new_type, count, is_derived_type) < 0)
                        HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,"couldn't convert 'all' selection to MPI type")
                    break;

                case H5S_SEL_POINTS:
                    /* not yet implemented */
                    ret_value = FAIL;
                    break;

                case H5S_SEL_HYPERSLABS:
                    if((H5S_SELECT_IS_REGULAR(space) == TRUE)) {
                        if(H5S_mpio_hyper_type(space, elmt_size, new_type, count, is_derived_type) < 0)
                            HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,"couldn't convert regular 'hyperslab' selection to MPI type")
		    } /* end if */
                    else {
                        if(H5S_mpio_span_hyper_type(space, elmt_size, new_type, count, is_derived_type) < 0)
                            HGOTO_ERROR(H5E_DATASPACE, H5E_BADTYPE, FAIL,"couldn't convert irregular 'hyperslab' selection to MPI type")
                    } /* end else */
                    break;

		case H5S_SEL_ERROR:
		case H5S_SEL_N:
                default:
                    HDassert("unknown selection type" && 0);
                    break;
            } /* end switch */
            break;

        case H5S_NO_CLASS:
        default:
            HDassert("unknown data space type" && 0);
            break;
    } /* end switch */

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5S_mpio_space_type() */


/*-------------------------------------------------------------------------
 * Function:	H5S_mpio_create_large_type
 *
 * Purpose:	Create a large datatype of size larger than what a 32 bit integer 
 *              can hold.
 *
 * Return:	non-negative on success, negative on failure.
 *
 *		*new_type	  the new datatype created
 *
 * Programmer:	Mohamad Chaarawi
 *
 *-------------------------------------------------------------------------
 */
static herr_t H5S_mpio_create_large_type (hsize_t num_elements, 
                                          MPI_Aint strid_bytes,
                                          MPI_Datatype old_type, 
                                          MPI_Datatype *new_type) 
{
    int           num_inner_types = 1; /* num times the 2G datatype will be repeated */
    int           block_len[2];
    int           mpi_code; /* MPI return code */
    MPI_Datatype  inner_type, outer_type, type[2];
    MPI_Aint      disp[2], old_extent;
    herr_t	  ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5S_mpio_create_large_type)

    /* Create a contiguous datatype of size equal to the largest
     * number a 32 bit integer can hold x size of old type.
     * If the displacement is 0, then the type is contiguous, otherwise 
     * use type_hvector to create the type with the displacement provided  */
    if (0 == strid_bytes) {
        if(MPI_SUCCESS != (mpi_code = MPI_Type_contiguous(2147483647, 
                                                          old_type, 
                                                          &inner_type))) {
            HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
        }
    }
    else {
        if(MPI_SUCCESS != (mpi_code = MPI_Type_create_hvector (2147483647, 
                                                               1,
                                                               strid_bytes,
                                                               old_type, 
                                                               &inner_type))) {
            HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
        }
    }
    /* Calculate how many types of the created inner type are needed to
     * represent the buffer */
    while (num_elements > (hsize_t)(num_inner_types+1) * 2147483647) {
        num_inner_types ++;
    }

    /* Create a contiguous datatype of the buffer (minus the remainng < 2GB part) 
     * If a strid is present, use hvector */
    if (0 == strid_bytes) {
        if(MPI_SUCCESS != (mpi_code = MPI_Type_contiguous(num_inner_types, 
                                                          inner_type, 
                                                          &outer_type))) {
            HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
        }
    }
    else {
        if(MPI_SUCCESS != (mpi_code = MPI_Type_create_hvector (num_inner_types, 
                                                               1,
                                                               strid_bytes,
                                                               inner_type, 
                                                               &outer_type))) {
            HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
        }
    }

    MPI_Type_free(&inner_type);

    /* If there is a remaining part create a contiguous/vector datatype and then
     * a struct datatype for the buffer */
    if((hsize_t)num_inner_types * 2147483647 != num_elements) {
        if (strid_bytes == 0) {
            if(MPI_SUCCESS != (mpi_code = MPI_Type_contiguous
                               ((int)(num_elements - (hsize_t)num_inner_types*2147483647),
                                old_type, 
                                &inner_type))) {
                HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
            }
        }
        else {
            if(MPI_SUCCESS != (mpi_code = MPI_Type_create_hvector 
                               ((int)(num_elements - (hsize_t)num_inner_types*2147483647), 
                                1,
                                strid_bytes,
                                old_type, 
                                &inner_type))) {
                HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
            }
        }

        MPI_Type_extent (old_type, &old_extent);

        /* Set up the arguments for MPI_Type_struct constructor */
        type[0] = outer_type;
        type[1] = inner_type;
        block_len[0] = 1;
        block_len[1] = 1;
        disp[0] = 0;
        disp[1] = (old_extent+strid_bytes)*num_inner_types*2147483647;

        if(MPI_SUCCESS != (mpi_code = 
                           MPI_Type_struct(2, block_len, disp, type, new_type))) {
            HMPI_GOTO_ERROR(FAIL, "MPI_Type_contiguous failed", mpi_code)
        }

        MPI_Type_free(&outer_type);
        MPI_Type_free(&inner_type);
    }

    /* There are no remaining bytes so just set the new type to
     * the outer type created */
    else {
        *new_type = outer_type;
    }

    if(MPI_SUCCESS != (mpi_code = MPI_Type_commit(new_type)))
        HMPI_GOTO_ERROR(FAIL, "MPI_Type_commit failed", mpi_code)

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5S_mpio_create_large_type */
#endif  /* H5_HAVE_PARALLEL */

