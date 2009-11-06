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
 * Created:		H5Pdapl.c
 *			October 27, 2008
 *			Neil Fortner <nfortne2@hdfgroup.org>
 *
 * Purpose:		Dataset access property list class routines
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
#include "H5private.h"      /* Generic Functions */
#include "H5Eprivate.h"     /* Error handling */
#include "H5Fprivate.h"	    /* Files */
#include "H5Zprivate.h"	    /* Filter pipeline */
#include "H5Ppkg.h"         /* Property lists */


/****************/
/* Local Macros */
/****************/

/* ========= Dataset Access properties ============ */
/* Definition for the information of Direct IO */
#define H5Z_DIRECT_IO_SIZE sizeof(H5FD_direct_fapl_t)
/*---------------------------------------------------------change it back to FALSE later--*/
#define H5Z_DIRECT_IO_DEF  {4096, 4096, 64*1024, FALSE}

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
static herr_t H5P_pacc_reg_prop(H5P_genclass_t *pclass);


/*********************/
/* Package Variables */
/*********************/

/* Dataset access property list class library initialization object */
const H5P_libclass_t H5P_CLS_PACC[1] = {{
    "filter pipeline access",	/* Class name for debugging     */
    &H5P_CLS_ROOT_g,	        /* Parent class ID              */
    &H5P_CLS_PIPELINE_ACCESS_g,	/* Pointer to class ID          */
    &H5P_LST_PIPELINE_ACCESS_g,	/* Pointer to default property list ID */
    H5P_pacc_reg_prop,		/* Default property registration routine */
    NULL,		        /* Class creation callback      */
    NULL,		        /* Class creation callback info */
    NULL,		        /* Class copy callback          */
    NULL,		        /* Class copy callback info     */
    NULL,		        /* Class close callback         */
    NULL 		        /* Class close callback info    */
}};


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/



/*-------------------------------------------------------------------------
 * Function:    H5P_pacc_reg_prop
 *
 * Purpose:     Register the filter pipeline access property list class's
 *              properties
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Raymond Lu
 *              18 September 2009
 *-------------------------------------------------------------------------
 */
static herr_t
H5P_pacc_reg_prop(H5P_genclass_t *pclass)
{
    H5FD_direct_fapl_t direct_info = H5Z_DIRECT_IO_DEF;
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5P_pacc_reg_prop)

    /* Register the size of raw data chunk cache (elements) */
    if(H5P_register(pclass, H5Z_DIRECT_IO_NAME, H5Z_DIRECT_IO_SIZE, &direct_info, NULL, NULL, NULL, NULL, NULL, NULL, NULL) < 0)
         HGOTO_ERROR(H5E_PLIST, H5E_CANTINSERT, FAIL, "can't insert property into class")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5P_pacc_reg_prop() */
