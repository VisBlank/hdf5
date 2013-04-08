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
 * Created:             H5Ofsinfo.c
 *                      Feb 2009
 *			Vailin Choi
 *
 * Purpose:             File space info message.
 *
 *-------------------------------------------------------------------------
 */

#define H5O_PACKAGE		/* suppress error about including H5Opkg	*/

#include "H5private.h"		/* Generic Functions	*/
#include "H5Eprivate.h"		/* Error handling	*/
#include "H5FLprivate.h"	/* Free lists          	*/
#include "H5Opkg.h"             /* Object headers	*/

/* PRIVATE PROTOTYPES */
static void *H5O_fsinfo_decode(H5F_t *f, hid_t dxpl_id, H5O_t *open_oh, unsigned mesg_flags, unsigned *ioflags, const uint8_t *p);
static herr_t H5O_fsinfo_encode(H5F_t *f, hbool_t disable_shared, uint8_t *p, const void *_mesg);
static void *H5O_fsinfo_copy(const void *_mesg, void *_dest);
static size_t H5O_fsinfo_size(const H5F_t *f, hbool_t disable_shared, const void *_mesg);
static herr_t H5O_fsinfo_free(void *mesg);
static herr_t H5O_fsinfo_debug(H5F_t *f, hid_t dxpl_id, const void *_mesg,
    FILE * stream, int indent, int fwidth);

/* This message derives from H5O message class */
const H5O_msg_class_t H5O_MSG_FSINFO[1] = {{
    H5O_FSINFO_ID,            	/* message id number             	*/
    "fsinfo",                 	/* message name for debugging    	*/
    sizeof(H5O_fsinfo_t),     	/* native message size           	*/
    0,				/* messages are sharable?        	*/
    H5O_fsinfo_decode,        	/* decode message                	*/
    H5O_fsinfo_encode,        	/* encode message                	*/
    H5O_fsinfo_copy,          	/* copy the native value         	*/
    H5O_fsinfo_size,          	/* size of free-space manager info message */
    NULL,                   	/* default reset method         	*/
    H5O_fsinfo_free,	        /* free method				*/
    NULL,        		/* file delete method			*/
    NULL,			/* link method				*/
    NULL,			/* set share method			*/
    NULL,		    	/* can share method			*/
    NULL,			/* pre copy native value to file 	*/
    NULL,			/* copy native value to file    	*/
    NULL,			/* post copy native value to file	*/
    NULL,			/* get creation index			*/
    NULL,			/* set creation index			*/
    H5O_fsinfo_debug          	/* debug the message            	*/
}};

/* Declare a free list to manage the H5O_fsinfo_t struct */
H5FL_DEFINE_STATIC(H5O_fsinfo_t);


/*-------------------------------------------------------------------------
 * Function:    H5O_fsinfo_decode
 *
 * Purpose:     Decode a message and return a pointer to a newly allocated one.
 *
 * Return:      Success:        Ptr to new message in native form.
 *              Failure:        NULL
 *
 * Programmer:  Vailin Choi; Feb 2009
 *
 * Modifications:
 *	Vailin Choi; Dec 2012
 * 	Decode new fields due to paged aggregation for handling file space.
 *
 *	Vailin Choi; April 2013
 * 	The message will store all 6 addresses of the free-space managers.
 *-------------------------------------------------------------------------
 */
static void *
H5O_fsinfo_decode(H5F_t *f, hid_t UNUSED dxpl_id, H5O_t UNUSED *open_oh,
    unsigned UNUSED mesg_flags, unsigned UNUSED *ioflags, const uint8_t *p)
{
    H5O_fsinfo_t 	*fsinfo = NULL; /* File space info message */
    H5FD_mem_t 		type;		/* Memory type for iteration */
    void        	*ret_value;  	/* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* check args */
    HDassert(f);
    HDassert(p);

    /* Allocate space for message */
    if(NULL == (fsinfo = H5FL_CALLOC(H5O_fsinfo_t)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")

    fsinfo->version = *p++; 			/* Message version */
    fsinfo->strategy = (H5F_fspace_strategy_t)*p++;	/* File space strategy */
    fsinfo->persist = *p++;			/* Free-space persist or not */
    H5F_DECODE_LENGTH(f, p, fsinfo->threshold);	/* Free-space section threshold */

    H5F_DECODE_LENGTH(f, p, fsinfo->fsp_size);	/* File space page size */
    UINT16DECODE(p, fsinfo->pgend_meta_thres);	/* Page end metdata threshold */
    fsinfo->last_small = *p++;			/* EOF file space section type */


    /* Addresses of free space managers */
    for(type = H5FD_MEM_SUPER; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type)) {
	H5F_addr_decode(f, &p, &(fsinfo->fs_addr[type-1]));
	if(!fsinfo->persist)
	    HDassert(!H5F_addr_defined(fsinfo->fs_addr[type-1]));
    }

    if(fsinfo->strategy == H5F_FSPACE_STRATEGY_PAGE && fsinfo->fsp_size) {
	HDassert(!H5F_addr_defined(fsinfo->fs_addr[H5F_MEM_PAGE_NTYPES]));
	HDassert(!H5F_addr_defined(fsinfo->fs_addr[H5F_MEM_PAGE_NTYPES+1]));
	HDassert(!H5F_addr_defined(fsinfo->fs_addr[H5F_MEM_PAGE_NTYPES+2]));
    }

    /* Set return value */
    ret_value = fsinfo;

done:
    if(ret_value == NULL && fsinfo != NULL)
        fsinfo = H5FL_FREE(H5O_fsinfo_t, fsinfo);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_fsinfo_decode() */


/*-------------------------------------------------------------------------
 * Function:    H5O_fsinfo_encode
 *
 * Purpose:     Encodes a message.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Vailin Choi; Feb 2009
 *
 * Modifications:
 *	Vailin Choi; Dec 2012
 * 	Encode new fields due to paged aggregation for handling file space.
 *
 *	Vailin Choi; April 2013
 * 	The message will store all 6 addresses of the free-space managers.
 *-------------------------------------------------------------------------
 */ 
static herr_t 
H5O_fsinfo_encode(H5F_t *f, hbool_t UNUSED disable_shared, uint8_t *p, const void *_mesg)
{
    const H5O_fsinfo_t  *fsinfo = (const H5O_fsinfo_t *)_mesg;
    H5FD_mem_t 	type;	/* Memory type for iteration */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* check args */
    HDassert(f);
    HDassert(p);
    HDassert(fsinfo);


    *p++ = fsinfo->version;	/* Message version */
    *p++ = fsinfo->strategy;	/* File space strategy */
    *p++ = fsinfo->persist;	/* Free-space persist or not */
    H5F_ENCODE_LENGTH(f, p, fsinfo->threshold); /* Free-space section size threshold */

    H5F_ENCODE_LENGTH(f, p, fsinfo->fsp_size);	/* File space page size */
    UINT16ENCODE(p, fsinfo->pgend_meta_thres);	/* Page end metadata threshold */
    *p++ = fsinfo->last_small;			/* EOF file space section type */

    if(fsinfo->strategy == H5F_FSPACE_STRATEGY_PAGE && fsinfo->fsp_size) {
	HDassert(!H5F_addr_defined(fsinfo->fs_addr[H5F_MEM_PAGE_NTYPES]));
	HDassert(!H5F_addr_defined(fsinfo->fs_addr[H5F_MEM_PAGE_NTYPES+1]));
	HDassert(!H5F_addr_defined(fsinfo->fs_addr[H5F_MEM_PAGE_NTYPES+2]));
    }

   /* Addresses of free-space managers */
   for(type = H5FD_MEM_SUPER; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type)) {
	if(!fsinfo->persist)
	    HDassert(!H5F_addr_defined(fsinfo->fs_addr[type-1]));
	H5F_addr_encode(f, &p, fsinfo->fs_addr[type-1]);
    }

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5O_fsinfo_encode() */


/*-------------------------------------------------------------------------
 * Function:    H5O_fsinfo_copy
 *
 * Purpose:     Copies a message from _MESG to _DEST, allocating _DEST if
 *              necessary.
 *
 * Return:      Success:        Ptr to _DEST
 *              Failure:        NULL
 *
 * Programmer:  Vailin Choi; Feb 2009
 *
 *-------------------------------------------------------------------------
 */
static void *
H5O_fsinfo_copy(const void *_mesg, void *_dest)
{
    const H5O_fsinfo_t   *fsinfo = (const H5O_fsinfo_t *)_mesg;
    H5O_fsinfo_t         *dest = (H5O_fsinfo_t *) _dest;
    void                *ret_value;     /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* check args */
    HDassert(fsinfo);
    if(!dest && NULL == (dest = H5FL_CALLOC(H5O_fsinfo_t)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")

    /* copy */
    *dest = *fsinfo;

    /* Set return value */
    ret_value = dest;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_fsinfo_copy() */


/*-------------------------------------------------------------------------
 * Function:    H5O_fsinfo_size
 *
 * Purpose:     Returns the size of the raw message in bytes not counting
 *              the message type or size fields, but only the data fields.
 *              This function doesn't take into account alignment.
 *
 * Return:      Success:        Message data size in bytes without alignment.
 *              Failure:        zero
 *
 * Programmer:  Vailin Choi; Feb 2009
 *
 * Modifications:
 *	Vailin Choi; Dec 2012
 * 	Changes to the size due to paged aggregation.
 *
 *	Vailin Choi; April 2013
 * 	The message will store all 6 addresses of the free-space managers.
 *-------------------------------------------------------------------------
 */
static size_t
H5O_fsinfo_size(const H5F_t *f, hbool_t UNUSED disable_shared, const void *_mesg)
{
    size_t ret_value;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    ret_value = 3 				/* Version & strategy & persist */
		+ (size_t)H5F_SIZEOF_SIZE(f)	/* Free-space section threshold */
		+ (size_t)H5F_SIZEOF_SIZE(f) 	/* File space page size */
		+ 2				/* Page end meta threshold */
		+ 1;				/* EOF file space section type */
    
    /* Free-space manager addresses */
    ret_value += (H5FD_MEM_NTYPES - 1) * (size_t)H5F_SIZEOF_ADDR(f);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_fsinfo_size() */


/*-------------------------------------------------------------------------
 * Function:    H5O_fsinfo_free
 *
 * Purpose:     Free's the message
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Vailin Choi; Feb 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_fsinfo_free(void *mesg)
{
    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(mesg);

    mesg = H5FL_FREE(H5O_fsinfo_t, mesg);

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5O_fsinfo_free() */


/*-------------------------------------------------------------------------
 * Function:    H5O_fsinfo_debug
 *
 * Purpose:     Prints debugging info for a message.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Vailin Choi; Feb 2009
 *
 * Modifcations:
 *	Vailin Choi; Dec 2012
 *	Changes to the message due paged aggregation for handling file space.
 * 
 *	Vailin Choi; April 2013
 *	Changes for file space handling strategies and the 6 addresses of
 *	the free-space managers.
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_fsinfo_debug(H5F_t UNUSED *f, hid_t UNUSED dxpl_id, const void *_mesg, FILE * stream,
	       int indent, int fwidth)
{
    const H5O_fsinfo_t	*fsinfo = (const H5O_fsinfo_t *) _mesg;
    H5FD_mem_t 	type;	/* Free-space types for iteration */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* check args */
    HDassert(f);
    HDassert(fsinfo);
    HDassert(stream);
    HDassert(indent >= 0);
    HDassert(fwidth >= 0);

    HDfprintf(stream, "%*s%-*s %u\n", indent, "", fwidth,
              "File space version:", fsinfo->version);

    HDfprintf(stream, "%*s%-*s ", indent, "", fwidth, "File space strategy:");
    switch(fsinfo->strategy) {
        case H5F_FSPACE_STRATEGY_AGGR:
	    HDfprintf(stream, "%s\n", "H5F_FSPACE_STRATEGY_AGGR");
            break;

        case H5F_FSPACE_STRATEGY_PAGE:
	    HDfprintf(stream, "%s\n", "H5F_FSPACE_STRATEGY_PAGE");
            break;

        case H5F_FSPACE_STRATEGY_NONE:
	    HDfprintf(stream, "%s\n", "H5F_FSPACE_STRATEGY_NONE");
            break;

        default:
	    HDfprintf(stream, "%s\n", "unknown");

    } /* end switch */

    HDfprintf(stream, "%*s%-*s %s\n", indent, "", fwidth,
              "Free-space persist:", fsinfo->persist ? "TRUE" : "FALSE");

    HDfprintf(stream, "%*s%-*s %Hu\n", indent, "", fwidth,
              "Free-space section threshold:", fsinfo->threshold);

    HDfprintf(stream, "%*s%-*s %Hu\n", indent, "", fwidth,
              "File space page size:", fsinfo->fsp_size);

    HDfprintf(stream, "%*s%-*s %u\n", indent, "", fwidth,
	"Page end metadata threshold:", fsinfo->pgend_meta_thres);

    if(fsinfo->strategy == H5F_FSPACE_STRATEGY_PAGE && fsinfo->fsp_size) {
	if(fsinfo->last_small) {
	    HDfprintf(stream, "%*s%-*s %s\n", indent, "", fwidth,
		"EOF file space section type:", fsinfo->last_small == H5F_FILE_SPACE_EOF_SMALL_META ? "small meta data" : "small raw data");
	} else
	    HDfprintf(stream, "%*s%-*s %s\n", indent, "", fwidth,
		"EOF file space section type:", "large generic");
    } else {
	HDassert(!fsinfo->last_small);
	HDfprintf(stream, "%*s%-*s %u\n", indent, "", fwidth,
	    "EOF file space section type:", fsinfo->last_small);
    }

    for(type = H5FD_MEM_SUPER; type < H5FD_MEM_NTYPES; H5_INC_ENUM(H5FD_mem_t, type))
	HDfprintf(stream, "%*s%-*s %a\n", indent, "", fwidth,
	    "Free space manager address:", fsinfo->fs_addr[type-1]);

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5O_fsinfo_debug() */
