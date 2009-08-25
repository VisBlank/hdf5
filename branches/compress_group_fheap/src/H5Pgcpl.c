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
#define H5Z_PACKAGE		/*suppress error about including H5Zpkg	  */

/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5Ppkg.h"		/* Property lists		  	*/
#include "H5Zpkg.h"		/* Data filters				*/


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
static int H5P_ginfo_cmp(const void *ginfo1, const void *ginfo2, size_t size);

static int H5P_fheap_cparam_cmp(const H5HF_cparam_t *cparam1, const H5HF_cparam_t *cparam2);


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
    H5P_gcrt_copy,			/* Class copy callback          */
    NULL,		        /* Class copy callback info     */
    H5P_gcrt_close,			/* Class close callback         */
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
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5P_gcrt_reg_prop)

    /* Register group info property */
    if(H5P_register(pclass, H5G_CRT_GROUP_INFO_NAME, H5G_CRT_GROUP_INFO_SIZE,
             &ginfo, NULL, NULL, NULL, NULL, NULL, H5G_CRT_GROUP_INFO_CMP, NULL) < 0)
         HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

    /* Register link info property */
    if(H5P_register(pclass, H5G_CRT_LINK_INFO_NAME, H5G_CRT_LINK_INFO_SIZE,
             &linfo, NULL, NULL, NULL, NULL, NULL, NULL, NULL) < 0)
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
 *                 Wednesday, May 6, 2009
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static herr_t
H5P_gcrt_copy(hid_t dst_plist_id, hid_t src_plist_id, void UNUSED *copy_data)
{
    H5O_ginfo_t    src_ginfo, dst_ginfo;        /* Source & destination ginfo */
    H5P_genplist_t *src_plist;                  /* Pointer to source property list */
    H5P_genplist_t *dst_plist;                  /* Pointer to destination property list */
    herr_t         ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5P_gcrt_copy)

    /* Verify property list IDs */
    if(NULL == (dst_plist = (H5P_genplist_t *)H5I_object(dst_plist_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a group creation property list")
    if(NULL == (src_plist = (H5P_genplist_t *)H5I_object(src_plist_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a group creation property list")

    /* Get the group info property from the old property list */
    if(H5P_get(src_plist, H5G_CRT_GROUP_INFO_NAME, &src_ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Make copy of group info */
    if(NULL == H5O_msg_copy(H5O_GINFO_ID, &src_ginfo, &dst_ginfo))
        HGOTO_ERROR(H5E_PLIST, H5E_CANTINIT, FAIL, "can't copy group info")

    /* Set the group info property for the destination property list */
    if(H5P_set(dst_plist, H5G_CRT_GROUP_INFO_NAME, &dst_ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set group info")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_gcrt_copy() */


/*-------------------------------------------------------------------------
 * Function:	H5P_gcrt_close
 *
 * Purpose:	Callback routine which is called whenever any dataset create
 *              property list is closed.  This routine performs any generic
 *              cleanup needed on the properties the library put into the list.
 *
 * Return:	Success:	Non-negative
 *		Failure:	Negative
 *
 * Programmer:	Neil Fortner
 *              Wednesday, May 6, 2009
 *
 *-------------------------------------------------------------------------
 */
/* ARGSUSED */
static herr_t
H5P_gcrt_close(hid_t dcpl_id, void UNUSED *close_data)
{
    H5O_ginfo_t     ginfo;              /* I/O pipeline */
    H5P_genplist_t *plist;              /* Property list */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5P_gcrt_close)

    /* Check arguments */
    if(NULL == (plist = (H5P_genplist_t *)H5I_object(dcpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a group creation property list")

    /* Get the link name pipeline property from the old property list */
    if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Clean up any values set for the link name pipeline in the ginfo
     * message */
    if(H5O_msg_reset(H5O_GINFO_ID, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTFREE, FAIL, "can't release group info")

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


/*-------------------------------------------------------------------------
 * Function:	H5Pset_fheap_cparams
 *
 * Purpose:	Sets the fractal heap creation parameters for the fractal
 *              heap in plist_id of type specified by type.  Currently
 *              only supports fractal heaps for group links.  Once support
 *              for other types is added, this will probably be moved to
 *              H5Pocpl.c.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Neil Fortner
 *              Monday, June 1, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5Pset_fheap_cparams(hid_t plist_id, H5HF_type_t type, const H5HF_cparam_t *cparam)
{
    H5P_genplist_t      *plist;                 /* Property list pointer */
    H5O_ginfo_t         ginfo;                  /* Group info */
    H5HF_cparam_t       cparam_def = H5G_CRT_FHEAP_CPARAM_DEF; /* Default fheap creation params */
    herr_t              ret_value = SUCCEED;    /* return value */

    FUNC_ENTER_API(H5Pset_fheap_cparams, FAIL)
    H5TRACE3("e", "iHt*Hc", plist_id, type, cparam);

    /* Check args */
    if(!cparam)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "structure not supplied")
    if(cparam->version != H5HF_CPARAM_VERSION_1)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid version number for structure")
    if(type <= H5HF_TYPE_UNKNOWN || type >= H5HF_TYPE_NTYPES)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid fractal heap type")

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get the group info property to modify */
    if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Modify the fractal heap creation parameters, and mark them as stored if
     * they are different from the default parameters */
    ginfo.fheap_cparam = *cparam;
    if(H5P_fheap_cparam_cmp(&ginfo.fheap_cparam, &cparam_def))
        ginfo.store_fheap_cparam = TRUE;
    else
        ginfo.store_fheap_cparam = FALSE;

    /* Put the updated group info back into the property list */
    if(H5P_set(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "can't set group info")

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pset_fheap_cparams() */


/*-------------------------------------------------------------------------
 * Function:	H5Pget_fheap_cparams
 *
 * Purpose:	Gets the fractal heap creation parameters for the fractal
 *              heap in plist_id of type specified by type.  Currently
 *              only supports fractal heaps for group links.  Once support
 *              for other types is added, this will probably be moved to
 *              H5Pocpl.c.
 *
 * Return:	TRUE if fheap creation params are set to be stored
 *              FALSE if not
 *              Negative on failure
 *
 * Programmer:	Neil Fortner
 *              Monday, June 1, 2009
 *
 *-------------------------------------------------------------------------
 */
htri_t
H5Pget_fheap_cparams(hid_t plist_id, H5HF_type_t type, H5HF_cparam_t *cparam)
{
    H5P_genplist_t      *plist;                 /* Property list pointer */
    H5O_ginfo_t         ginfo;                  /* Group info */
    htri_t              ret_value;              /* return value */

    FUNC_ENTER_API(H5Pget_fheap_cparams, FAIL)
    H5TRACE3("t", "iHt*Hc", plist_id, type, cparam);

    /* Check args */
    if(!cparam)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "structure not supplied")
    if(type <= H5HF_TYPE_UNKNOWN || type >= H5HF_TYPE_NTYPES)
        HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, FAIL, "invalid fractal heap type")

    /* Get the plist structure */
    if(NULL == (plist = H5P_object_verify(plist_id, H5P_GROUP_CREATE)))
        HGOTO_ERROR(H5E_ATOM, H5E_BADATOM, FAIL, "can't find object for ID")

    /* Get the group info property to query */
    if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Update the user supplied cparam struct */
    *cparam = ginfo.fheap_cparam;

    /* Check if fheap creation params are set to be stored, set return value */
    ret_value = ginfo.store_fheap_cparam != FALSE;

done:
    FUNC_LEAVE_API(ret_value)
} /* end H5Pget_fheap_cparams() */


/*-------------------------------------------------------------------------
 * Function:	H5P_gcpl_set_pline
 *
 * Purpose:	Sets the group link pipeline on a group creation property
 *              list.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Neil Fortner
 *              Monday, July 20, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5P_gcpl_set_pline(H5P_genplist_t *plist, const H5O_pline_t *pline)
{
    H5O_ginfo_t         ginfo;                  /* Group info */
    herr_t              ret_value=SUCCEED;      /* return value */

    FUNC_ENTER_NOAPI(H5P_gcpl_set_pline, FAIL)

    /* Check arguments */
    HDassert(plist);
    HDassert(pline);

    /* Get the group info from the gcpl */
    if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Copy the pline into the group info struct */
    /* No need to free/reset the pline as the functions that call this should
     * handle it through the H5Z* functions -NAF */
    ginfo.pline = *pline;

    /* Set the group info on the gcpl */
    if(H5P_set(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLINE, H5E_CANTINIT, FAIL, "unable to set group info")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_gcpl_set_pline() */


/*-------------------------------------------------------------------------
 * Function:	H5P_gcpl_get_pline
 *
 * Purpose:	Gets the group link pipeline from a group creation
 *              property list.  This pipeline is a *shallow* copy of the
 *              pipeline in the gcpl, so it must be reset properly if it
 *              is to be replaced by a different pipeline.  Modifying
 *              this pipeline and passing it back to H5P_gcpl_set_pline
 *              is OK.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Neil Fortner
 *              Monday, July 20, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5P_gcpl_get_pline(const H5P_genplist_t *plist, H5O_pline_t *pline)
{
    H5O_ginfo_t         ginfo;                  /* Group info */
    herr_t              ret_value=SUCCEED;      /* return value */

    FUNC_ENTER_NOAPI(H5P_gcpl_get_pline, FAIL)

    /* Check arguments */
    HDassert(plist);
    HDassert(pline);

    /* Get the group info from the gcpl */
    if(H5P_get(plist, H5G_CRT_GROUP_INFO_NAME, &ginfo) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get group info")

    /* Retrieve the pline from the group info struct */
    *pline = ginfo.pline;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_gcpl_get_pline() */


/*-------------------------------------------------------------------------
 * Function:       H5P_fheap_cparam_cmp
 *
 * Purpose:        Callback routine which is called whenever a group info
 *                 property in a property list is compared.
 *
 * Return:         positive if VALUE1 is greater than VALUE2, negative if
 *                      VALUE2 is greater than VALUE1 and zero if VALUE1 and
 *                      VALUE2 are equal.
 *
 * Programmer:     Neil Fortner
 *                 Monday, May 18, 2009
 *
 *-------------------------------------------------------------------------
 */
static int
H5P_fheap_cparam_cmp(const H5HF_cparam_t *cparam1, const H5HF_cparam_t *cparam2)
{
    herr_t ret_value = 0; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5P_fheap_cparam_cmp)

    if(cparam1->version < cparam2->version) HGOTO_DONE(-1)
    if(cparam1->version > cparam2->version) HGOTO_DONE(1)

    if(cparam1->width < cparam2->width) HGOTO_DONE(-1)
    if(cparam1->width > cparam2->width) HGOTO_DONE(1)

    if(cparam1->start_block_size < cparam2->start_block_size) HGOTO_DONE(-1)
    if(cparam1->start_block_size > cparam2->start_block_size) HGOTO_DONE(1)

    if(cparam1->max_direct_size < cparam2->max_direct_size) HGOTO_DONE(-1)
    if(cparam1->max_direct_size > cparam2->max_direct_size) HGOTO_DONE(1)

    if(cparam1->max_index < cparam2->max_index) HGOTO_DONE(-1)
    if(cparam1->max_index > cparam2->max_index) HGOTO_DONE(1)

    if(cparam1->start_root_rows < cparam2->start_root_rows) HGOTO_DONE(-1)
    if(cparam1->start_root_rows > cparam2->start_root_rows) HGOTO_DONE(1)

    if(!cparam1->checksum_dblocks && cparam2->checksum_dblocks) HGOTO_DONE(-1)
    if(cparam1->checksum_dblocks && !cparam2->checksum_dblocks) HGOTO_DONE(1)

    if(cparam1->max_man_size < cparam2->max_man_size) HGOTO_DONE(-1)
    if(cparam1->max_man_size > cparam2->max_man_size) HGOTO_DONE(1)

    if(cparam1->id_len < cparam2->id_len) HGOTO_DONE(-1)
    if(cparam1->id_len > cparam2->id_len) HGOTO_DONE(1)

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_fheap_cparam_cmp() */


/*-------------------------------------------------------------------------
 * Function:       H5P_ginfo_cmp
 *
 * Purpose:        Callback routine which is called whenever a group info
 *                 property in a property list is compared.
 *
 * Return:         positive if VALUE1 is greater than VALUE2, negative if
 *                      VALUE2 is greater than VALUE1 and zero if VALUE1 and
 *                      VALUE2 are equal.
 *
 * Programmer:     Neil Fortner
 *                 Monday, May 18, 2009
 *
 *-------------------------------------------------------------------------
 */
static int
H5P_ginfo_cmp(const void *_ginfo1, const void *_ginfo2, size_t UNUSED size)
{
    const H5O_ginfo_t *ginfo1 = (const H5O_ginfo_t *)_ginfo1,     /* Create local aliases for values */
        *ginfo2 = (const H5O_ginfo_t *)_ginfo2;
    herr_t ret_value = 0; /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5P_ginfo_cmp)

    /* Compare fields in top level of ginfo struct */
    if(ginfo1->version < ginfo2->version)  HGOTO_DONE(-1)
    if(ginfo1->version > ginfo2->version)  HGOTO_DONE(1)

    if(ginfo1->lheap_size_hint < ginfo2->lheap_size_hint)  HGOTO_DONE(-1)
    if(ginfo1->lheap_size_hint > ginfo2->lheap_size_hint)  HGOTO_DONE(1)

    if(!ginfo1->store_link_phase_change && ginfo2->store_link_phase_change)  HGOTO_DONE(-1)
    if(ginfo1->store_link_phase_change && !ginfo2->store_link_phase_change)  HGOTO_DONE(1)

    if(ginfo1->max_compact < ginfo2->max_compact)  HGOTO_DONE(-1)
    if(ginfo1->max_compact > ginfo2->max_compact)  HGOTO_DONE(1)

    if(ginfo1->min_dense < ginfo2->min_dense)  HGOTO_DONE(-1)
    if(ginfo1->min_dense > ginfo2->min_dense)  HGOTO_DONE(1)

    if(!ginfo1->store_est_entry_info && ginfo2->store_est_entry_info)  HGOTO_DONE(-1)
    if(ginfo1->store_est_entry_info && !ginfo2->store_est_entry_info)  HGOTO_DONE(1)

    if(ginfo1->est_num_entries < ginfo2->est_num_entries)  HGOTO_DONE(-1)
    if(ginfo1->est_num_entries > ginfo2->est_num_entries)  HGOTO_DONE(1)

    if(ginfo1->est_name_len < ginfo2->est_name_len)  HGOTO_DONE(-1)
    if(ginfo1->est_name_len > ginfo2->est_name_len)  HGOTO_DONE(1)

    if(!ginfo1->store_fheap_cparam && ginfo2->store_fheap_cparam)  HGOTO_DONE(-1)
    if(ginfo1->store_fheap_cparam && !ginfo2->store_fheap_cparam)  HGOTO_DONE(1)

    /* Compare the fheap creation parameters */
    ret_value = H5P_fheap_cparam_cmp(&ginfo1->fheap_cparam, &ginfo2->fheap_cparam);

    /* Compare the pipeline */
    if(ret_value == 0)
        ret_value = H5P_pipeline_cmp(&(((const H5O_ginfo_t *) ginfo1)->pline),
                &(((const H5O_ginfo_t *) ginfo2)->pline), sizeof(H5O_pline_t));

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_ginfo_cmp() */

