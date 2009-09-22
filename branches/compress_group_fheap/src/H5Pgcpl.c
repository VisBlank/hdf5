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
 * Created:		H5Pgcpl.c
 *			August 29 2006
 *			Quincey Koziol <koziol@ncsa.uiuc.edu>
 *
 * Purpose:		Group creation property list class routines
 *
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/
#define H5P_PACKAGE		/*suppress error about including H5Ppkg	  */

/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5Ppkg.h"		/* Property lists		  	*/


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

/* Property class callbacks */
static herr_t H5P_gcrt_reg_prop(H5P_genclass_t *pclass);
static herr_t H5P_gcrt_copy(hid_t new_plist_t, hid_t old_plist_t, void *copy_data);
static herr_t H5P_gcrt_close(hid_t dxpl_id, void *close_data);


/*********************/
/* Package Variables */
/*********************/

/* Group creation property list class library initialization object */
const H5P_libclass_t H5P_CLS_GCRT[1] = {{
    "group create",		/* Class name for debugging     */
    &H5P_CLS_OBJECT_CREATE_g,	/* Parent class ID              */
    &H5P_CLS_GROUP_CREATE_g,	/* Pointer to class ID          */
    &H5P_LST_GROUP_CREATE_g,	/* Pointer to default property list ID */
    H5P_gcrt_reg_prop,		/* Default property registration routine */
    NULL,		        /* Class creation callback      */
    NULL,		        /* Class creation callback info */
    H5P_gcrt_copy,		/* Class copy callback          */
    NULL,		        /* Class copy callback info     */
    H5P_gcrt_close,		/* Class close callback         */
    NULL 		        /* Class close callback info    */
}};


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/



/*-------------------------------------------------------------------------
 * Function:    H5P_gcrt_reg_prop
 *
 * Purpose:     Initialize the group creation property list class
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              October 31, 2006
 *-------------------------------------------------------------------------
 */
static herr_t
H5P_gcrt_reg_prop(H5P_genclass_t *pclass)
{
    H5O_ginfo_t ginfo = H5G_CRT_GROUP_INFO_DEF;     /* Default group info settings */
    H5O_linfo_t linfo = H5G_CRT_LINK_INFO_DEF;      /* Default link info settings */
    H5O_pline_t pline = H5G_CRT_LINK_PIPELINE_DEF;  /* Default I/O pipeline setting */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5P_gcrt_reg_prop)

    /* Register group info property */
    if(H5P_register(pclass, H5G_CRT_GROUP_INFO_NAME, H5G_CRT_GROUP_INFO_SIZE,
             &ginfo, NULL, NULL, NULL, NULL, NULL, NULL, NULL) < 0)
         HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register link info property */
    if(H5P_register(pclass, H5G_CRT_LINK_INFO_NAME, H5G_CRT_LINK_INFO_SIZE,
             &linfo, NULL, NULL, NULL, NULL, NULL, NULL, NULL) < 0)
         HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register the data pipeline property */
    if(H5P_register(pclass, H5G_CRT_LINK_PIPELINE_NAME, H5G_CRT_LINK_PIPELINE_SIZE, &pline, NULL, NULL, NULL, NULL, NULL, H5G_CRT_LINK_PIPELINE_CMP, NULL) < 0)
       HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_gcrt_reg_prop() */


/*-------------------------------------------------------------------------
 * Function:       H5P_gcrt_copy
 *
 * Purpose:        Callback routine which is called whenever any group
 *                 creation property list is copied.  This routine copies
 *                 the properties from the old list to the new list.
 *
 * Return:         Success:        Non-negative
 *                 Failure:        Negative
 *
 * Programmer:     Neil Fortner
 *                 Monday, September 21, 2009
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static herr_t
H5P_gcrt_copy(hid_t dst_plist_id, hid_t src_plist_id, void UNUSED *copy_data)
{
    H5O_pline_t    src_pline, dst_pline;        /* Source & destination pipelines */
    H5P_genplist_t *src_plist;                  /* Pointer to source property list */
    H5P_genplist_t *dst_plist;                  /* Pointer to destination property list */
    herr_t         ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5P_gcrt_copy)

    /* Verify property list IDs */
    if(NULL == (dst_plist = (H5P_genplist_t *)H5I_object(dst_plist_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset creation property list")
    if(NULL == (src_plist = (H5P_genplist_t *)H5I_object(src_plist_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset creation property list")

    /* Get the link pipeline property from the old property list */
    if(H5P_get(src_plist, H5G_CRT_LINK_PIPELINE_NAME, &src_pline) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get pipeline")

    /* Make copy of link pipeline */
    if(NULL == H5O_msg_copy(H5O_PLINE_ID, &src_pline, &dst_pline))
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINIT, FAIL, "can't copy link pipeline")

    /* Set the link pipeline property for the destination property list */
    if(H5P_set(dst_plist, H5G_CRT_LINK_PIPELINE_NAME, &dst_pline) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set pipeline")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_gcrt_copy() */


/*-------------------------------------------------------------------------
 * Function:	H5P_gcrt_close
 *
 * Purpose:	Callback routine which is called whenever any group create
 *              property list is closed.  This routine performs any generic
 *              cleanup needed on the properties the library put into the list.
 *
 * Return:	Success:	Non-negative
 *		Failure:	Negative
 *
 * Programmer:     Neil Fortner
 *                 Monday, September 21, 2009
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static herr_t
H5P_gcrt_close(hid_t dcpl_id, void UNUSED *close_data)
{
    H5O_pline_t     pline;              /* I/O pipeline */
    H5P_genplist_t *plist;              /* Property list */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5P_gcrt_close)

    /* Check arguments */
    if(NULL == (plist = (H5P_genplist_t *)H5I_object(dcpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a dataset creation property list")

    /* Get the link pipeline property from the old property list */
    if(H5P_get(plist, H5G_CRT_LINK_PIPELINE_NAME, &pline) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get pipeline")

    /* Clean up any values set for the link pipeline */
    if(H5O_msg_reset(H5O_PLINE_ID, &pline) < 0)
        HGOTO_ERROR(H5E_DATASET, H5E_CANTFREE, FAIL, "can't release pipeline info")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_gcrt_close() */


/*-------------------------------------------------------------------------
 * Function:    H5Pset_local_heap_size_hint
 *
 * Purpose:     Set the "size hint" for creating local heaps for a group.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              August 29, 2005
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_local_heap_size_hint(hid_t plist_id, size_t size_hint)
{
    H5P_genplist_t *plist;      /* Property list pointer */
    H5O_ginfo_t ginfo;          /* Group information structure */
    herr_t ret_value = SUCCEED;       /* Return value */

    FUNC_ENTER_API(H5Pset_local_heap_size_hint, FAIL)
    H5TRACE2("e", "iz", plist_id, size_hint);

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get value */
    if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Update field */
    ginfo.lheap_size_hint = size_hint;

    /* Set value */
    if(H5P_set(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set group info")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_local_heap_size_hint() */


/*-------------------------------------------------------------------------
 * Function:    H5Pget_local_heap_size_hint
 *
 * Purpose:     Returns the local heap size hint, which is used for creating
 *              groups
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              August 29, 2005
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_local_heap_size_hint(hid_t plist_id, size_t *size_hint /*out*/)
{
    herr_t ret_value = SUCCEED;   /* return value */

    FUNC_ENTER_API(H5Pget_local_heap_size_hint, FAIL)
    H5TRACE2("e", "ix", plist_id, size_hint);

    if(size_hint) {
        H5P_genplist_t *plist;      /* Property list pointer */
        H5O_ginfo_t ginfo;          /* Group information structure */

        /* Get the plist structure */
        if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
            HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

        /* Get value */
        if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

        /* Update field */
        *size_hint = ginfo.lheap_size_hint;
    } /* end if */

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_local_heap_size_hint() */


/*-------------------------------------------------------------------------
 * Function:    H5Pset_link_phase_change
 *
 * Purpose:     Set the maximum # of links to store "compactly" and the
 *              minimum # of links to store "densely".  (These should
 *              overlap).
 *
 * Note:        Currently both of these must be updated at the same time.
 *
 * Note:        Come up with better name & description! -QAK
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              August 29, 2005
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_link_phase_change(hid_t plist_id, unsigned max_compact, unsigned min_dense)
{
    H5P_genplist_t *plist;              /* Property list pointer */
    H5O_ginfo_t ginfo;                  /* Group information structure */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(H5Pset_link_phase_change, FAIL)
    H5TRACE3("e", "iIuIu", plist_id, max_compact, min_dense);

    /* Range check values */
    if(max_compact < min_dense)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "max compact value must be >= min dense value")
    if(max_compact > 65535)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "max compact value must be < 65536")
    if(min_dense > 65535)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "min dense value must be < 65536")

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get group info */
    if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Update fields */
    if(max_compact != H5G_CRT_GINFO_MAX_COMPACT || min_dense != H5G_CRT_GINFO_MIN_DENSE)
        ginfo.store_link_phase_change = TRUE;
    else
        ginfo.store_link_phase_change = FALSE;
    ginfo.max_compact = (uint16_t)max_compact;
    ginfo.min_dense = (uint16_t)min_dense;

    /* Set group info */
    if(H5P_set(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set group info")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_link_phase_change() */


/*-------------------------------------------------------------------------
 * Function:    H5Pget_link_phase_change
 *
 * Purpose:     Returns the max. # of compact links & the min. # of dense
 *              links, which are used for storing groups
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              August 29, 2005
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_link_phase_change(hid_t plist_id, unsigned *max_compact /*out*/, unsigned *min_dense /*out*/)
{
    herr_t ret_value = SUCCEED;   /* return value */

    FUNC_ENTER_API(H5Pget_link_phase_change, FAIL)
    H5TRACE3("e", "ixx", plist_id, max_compact, min_dense);

    /* Get values */
    if(max_compact || min_dense) {
        H5P_genplist_t *plist;      /* Property list pointer */
        H5O_ginfo_t ginfo;          /* Group information structure */

        /* Get the plist structure */
        if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
            HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

        /* Get group info */
        if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

        if(max_compact)
            *max_compact = ginfo.max_compact;
        if(min_dense)
            *min_dense = ginfo.min_dense;
    } /* end if */

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_link_phase_change() */


/*-------------------------------------------------------------------------
 * Function:    H5Pset_est_link_info
 *
 * Purpose:     Set the estimates for the number of entries and length of each
 *              entry name in a group.
 *
 * Note:        Currently both of these must be updated at the same time.
 *
 * Note:        EST_NUM_ENTRIES applies only when the number of entries is less
 *              than the MAX_COMPACT # of entries (from H5Pset_link_phase_change).
 *
 * Note:        Come up with better name & description? -QAK
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              September  6, 2005
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_est_link_info(hid_t plist_id, unsigned est_num_entries, unsigned est_name_len)
{
    H5P_genplist_t *plist;              /* Property list pointer */
    H5O_ginfo_t ginfo;                  /* Group information structure */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(H5Pset_est_link_info, FAIL)
    H5TRACE3("e", "iIuIu", plist_id, est_num_entries, est_name_len);

    /* Range check values */
    if(est_num_entries > 65535)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "est. number of entries must be < 65536")
    if(est_name_len > 65535)
        HGOTO_ERROR(H5E_ARGS, H5E_BADRANGE, FAIL, "est. name length must be < 65536")

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get group info */
    if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Update fields */
    if(est_num_entries != H5G_CRT_GINFO_EST_NUM_ENTRIES || est_name_len != H5G_CRT_GINFO_EST_NAME_LEN)
        ginfo.store_est_entry_info = TRUE;
    else
        ginfo.store_est_entry_info = FALSE;
    ginfo.est_num_entries = (uint16_t)est_num_entries;
    ginfo.est_name_len = (uint16_t)est_name_len;

    /* Set group info */
    if(H5P_set(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set group info")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_est_link_info() */


/*-------------------------------------------------------------------------
 * Function:    H5Pget_est_link_info
 *
 * Purpose:     Returns the est. # of links in a group & the est. length of
 *              the name of each link.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              September  6, 2005
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_est_link_info(hid_t plist_id, unsigned *est_num_entries /*out*/, unsigned *est_name_len /*out*/)
{
    herr_t ret_value = SUCCEED;   /* return value */

    FUNC_ENTER_API(H5Pget_est_link_info, FAIL)
    H5TRACE3("e", "ixx", plist_id, est_num_entries, est_name_len);

    /* Get values */
    if(est_num_entries || est_name_len) {
        H5P_genplist_t *plist;      /* Property list pointer */
        H5O_ginfo_t ginfo;          /* Group information structure */

        /* Get the plist structure */
        if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
            HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

        /* Get group info */
        if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

        if(est_num_entries)
            *est_num_entries = ginfo.est_num_entries;
        if(est_name_len)
            *est_name_len = ginfo.est_name_len;
    } /* end if */

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_est_link_info() */


/*-------------------------------------------------------------------------
 * Function:    H5Pset_link_creation_order
 *
 * Purpose:     Set the flags for creation order of links in a group
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              September 12, 2006
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_link_creation_order(hid_t plist_id, unsigned crt_order_flags)
{
    H5P_genplist_t *plist;              /* Property list pointer */
    H5O_linfo_t linfo;                  /* Link information structure */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_API(H5Pset_link_creation_order, FAIL)
    H5TRACE2("e", "iIu", plist_id, crt_order_flags);

        /* Check for bad combination of flags */
    if(!(crt_order_flags & H5P_CRT_ORDER_TRACKED) && (crt_order_flags & H5P_CRT_ORDER_INDEXED))
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "tracking creation order is required for index")

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get link info */
    if(H5P_get(plist, H5G_CRT_LINK_INFO_NAME, &linfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get link info")

    /* Update fields */
    linfo.track_corder = (hbool_t)((crt_order_flags & H5P_CRT_ORDER_TRACKED) ? TRUE : FALSE);
    linfo.index_corder = (hbool_t)((crt_order_flags & H5P_CRT_ORDER_INDEXED) ? TRUE : FALSE);

    /* Set link info */
    if(H5P_set(plist, H5G_CRT_LINK_INFO_NAME, &linfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set link info")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_link_creation_order() */


/*-------------------------------------------------------------------------
 * Function:    H5Pget_link_creation_order
 *
 * Purpose:     Returns the flag indicating that creation order is tracked
 *              for links in a group.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              September 12, 2006
 *-------------------------------------------------------------------------
 */
herr_t
H5Pget_link_creation_order(hid_t plist_id, unsigned *crt_order_flags /*out*/)
{
    herr_t ret_value = SUCCEED;   /* return value */

    FUNC_ENTER_API(H5Pget_link_creation_order, FAIL)
    H5TRACE2("e", "ix", plist_id, crt_order_flags);

    /* Get values */
    if(crt_order_flags) {
        H5P_genplist_t *plist;      /* Property list pointer */
        H5O_linfo_t linfo;          /* Link information structure */

        /* Reset the value to return */
        *crt_order_flags = 0;

        /* Get the plist structure */
        if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
            HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

        /* Get link info */
        if(H5P_get(plist, H5G_CRT_LINK_INFO_NAME, &linfo) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get link info")

        *crt_order_flags |= linfo.track_corder ? H5P_CRT_ORDER_TRACKED : 0;
        *crt_order_flags |= linfo.index_corder ? H5P_CRT_ORDER_INDEXED : 0;
    } /* end if */

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_link_creation_order() */

