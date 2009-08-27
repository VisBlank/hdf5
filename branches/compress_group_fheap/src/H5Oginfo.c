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
 * Created:             H5Oginfo.c
 *                      Aug 23 2005
 *                      Quincey Koziol <koziol@ncsa.uiuc.edu>
 *
 * Purpose:             Group Information messages.
 *
 *-------------------------------------------------------------------------
 */

#define H5O_PACKAGE		/*suppress error about including H5Opkg	  */

#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5FLprivate.h"	/* Free lists                           */
#include "H5HFprivate.h"        /* Fractal heap				*/
#include "H5Opkg.h"             /* Object headers			*/


/* PRIVATE PROTOTYPES */
static void *H5O_ginfo_decode(H5F_t *f, hid_t dxpl_id, H5O_t *open_oh,
    unsigned mesg_flags, unsigned *ioflags, const uint8_t *p);
static herr_t H5O_ginfo_encode(H5F_t *f, hbool_t disable_shared, uint8_t *p, const void *_mesg);
static void *H5O_ginfo_copy(const void *_mesg, void *_dest);
static size_t H5O_ginfo_size(const H5F_t *f, hbool_t disable_shared, const void *_mesg);
static herr_t H5O_ginfo_reset(void *_mesg);
static herr_t H5O_ginfo_free(void *_mesg);
static herr_t H5O_ginfo_pre_copy_file(H5F_t *file_src, const void *mesg_src,
    hbool_t *deleted, const H5O_copy_t *cpy_info, void *_udata);
static herr_t H5O_ginfo_debug(H5F_t *f, hid_t dxpl_id, const void *_mesg,
			     FILE * stream, int indent, int fwidth);

/* This message derives from H5O message class */
const H5O_msg_class_t H5O_MSG_GINFO[1] = {{
    H5O_GINFO_ID,            	/*message id number             */
    "ginfo",                 	/*message name for debugging    */
    sizeof(H5O_ginfo_t),     	/*native message size           */
    0,				/* messages are sharable?       */
    H5O_ginfo_decode,        	/*decode message                */
    H5O_ginfo_encode,        	/*encode message                */
    H5O_ginfo_copy,          	/*copy the native value         */
    H5O_ginfo_size,          	/*size of symbol table entry    */
    H5O_ginfo_reset,         	/*reset method                  */
    H5O_ginfo_free,	        /* free method			*/
    NULL,	        	/* file delete method		*/
    NULL,			/* link method			*/
    NULL, 			/*set share method		*/
    NULL,		    	/*can share method		*/
    H5O_ginfo_pre_copy_file,	/* pre copy native value to file */
    NULL,			/* copy native value to file    */
    NULL,			/* post copy native value to file    */
    NULL,			/* get creation index		*/
    NULL,			/* set creation index		*/
    H5O_ginfo_debug          	/*debug the message             */
}};

/* Flags for group info flag encoding */
#define H5O_GINFO_STORE_PHASE_CHANGE    0x01
#define H5O_GINFO_STORE_EST_ENTRY_INFO  0x02
#define H5O_GINFO_STORE_FHEAP_CPARAM    0x04
#define H5O_GINFO_STORE_PLINE           0x08
#define H5O_GINFO_ALL_FLAGS             (H5O_GINFO_STORE_PHASE_CHANGE \
        | H5O_GINFO_STORE_EST_ENTRY_INFO | H5O_GINFO_STORE_FHEAP_CPARAM \
        | H5O_GINFO_STORE_PLINE)

/* Declare a free list to manage the H5O_ginfo_t struct */
H5FL_DEFINE(H5O_ginfo_t);

/* Declare a free list to manage the H5O_pline_t struct */
H5FL_EXTERN(H5O_pline_t);


/*-------------------------------------------------------------------------
 * Function:    H5O_ginfo_decode
 *
 * Purpose:     Decode a message and return a pointer to
 *              a newly allocated one.
 *
 * Return:      Success:        Ptr to new message in native order.
 *
 *              Failure:        NULL
 *
 * Programmer:  Quincey Koziol
 *              koziol@ncsa.uiuc.edu
 *              Aug 30 2005
 *
 *-------------------------------------------------------------------------
 */
static void *
H5O_ginfo_decode(H5F_t *f, hid_t dxpl_id, H5O_t *open_oh,
    unsigned UNUSED mesg_flags, unsigned *ioflags, const uint8_t *p)
{
    H5O_ginfo_t         *ginfo = NULL;  /* Pointer to group information message */
    H5O_pline_t         *pline = NULL;  /* Pointer to temp pipeline message */
    unsigned char       flags;          /* Flags for encoding group info */
    void                *ret_value;     /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_ginfo_decode)

    /* check args */
    HDassert(p);

    /* Allocate space for message */
    if(NULL == (ginfo = H5FL_CALLOC(H5O_ginfo_t)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")

    /* Version of message */
    ginfo->version = *p++;
    if(ginfo->version > H5O_GINFO_VERSION_LATEST)
        HGOTO_ERROR(H5E_OHDR, H5E_CANTLOAD, NULL, "bad version number for message")

    /* Get the flags for the group */
    flags = *p++;
    if(flags & ~H5O_GINFO_ALL_FLAGS)
        HGOTO_ERROR(H5E_OHDR, H5E_CANTLOAD, NULL, "bad flag value for message")
    ginfo->store_link_phase_change = (flags & H5O_GINFO_STORE_PHASE_CHANGE) ? TRUE : FALSE;
    ginfo->store_est_entry_info = (flags & H5O_GINFO_STORE_EST_ENTRY_INFO) ? TRUE : FALSE;
    if(ginfo->version < H5O_GINFO_VERSION_1 && 
            (flags & ~(H5O_GINFO_STORE_PHASE_CHANGE | H5O_GINFO_STORE_EST_ENTRY_INFO)))
        HGOTO_ERROR(H5E_OHDR, H5E_CANTLOAD, NULL, "bad flag value for message version")

    /* Get the max. # of links to store compactly & the min. # of links to store densely */
    if(ginfo->store_link_phase_change) {
        UINT16DECODE(p, ginfo->max_compact)
        UINT16DECODE(p, ginfo->min_dense)
    } /* end if */
    else {
        ginfo->max_compact = H5G_CRT_GINFO_MAX_COMPACT;
        ginfo->min_dense = H5G_CRT_GINFO_MIN_DENSE;
    } /* end else */

    /* Get the estimated # of entries & name lengths */
    if(ginfo->store_est_entry_info) {
        UINT16DECODE(p, ginfo->est_num_entries)
        UINT16DECODE(p, ginfo->est_name_len)
    } /* end if */
    else {
        ginfo->est_num_entries = H5G_CRT_GINFO_EST_NUM_ENTRIES;
        ginfo->est_name_len = H5G_CRT_GINFO_EST_NAME_LEN;
    } /* end if */

    /* Decode the fractal heap creation parameters (if present) */
    ginfo->fheap_cparam.version = H5HF_CPARAM_VERSION_1;
    if(flags & H5O_GINFO_STORE_FHEAP_CPARAM) {
        /* Sanity check */
        HDassert(ginfo->version >= H5O_GINFO_VERSION_1);

        /* Set the flag to indicate that the fractal heap's creation parameters are stored */
        ginfo->store_fheap_cparam = TRUE;

        /* Table width */
        UINT16DECODE(p, ginfo->fheap_cparam.width);

        /* Starting block size */
        H5F_DECODE_LENGTH(f, p, ginfo->fheap_cparam.start_block_size);

        /* Maximum direct block size */
        H5F_DECODE_LENGTH(f, p, ginfo->fheap_cparam.max_direct_size);

        /* Maximum heap size (as # of bits) */
        UINT16DECODE(p, ginfo->fheap_cparam.max_index);

        /* Starting # of rows in root indirect block */
        UINT16DECODE(p, ginfo->fheap_cparam.start_root_rows);

        /* Whether to checksum direct blocks */
        ginfo->fheap_cparam.checksum_dblocks = *p++;

        /* Heap ID length */
        UINT16DECODE(p, ginfo->fheap_cparam.id_len);

        /* Max. size of "managed" objects */
        UINT32DECODE(p, ginfo->fheap_cparam.max_man_size);
    } else {
        /* Use default values */
        ginfo->fheap_cparam.width = H5G_CRT_FHEAP_MAN_WIDTH;
        ginfo->fheap_cparam.start_block_size = H5G_CRT_FHEAP_MAN_START_BLOCK_SIZE;
        ginfo->fheap_cparam.max_direct_size = H5G_CRT_FHEAP_MAN_MAX_DIRECT_SIZE;
        ginfo->fheap_cparam.max_index = H5G_CRT_FHEAP_MAN_MAX_INDEX;
        ginfo->fheap_cparam.start_root_rows = H5G_CRT_FHEAP_MAN_START_ROOT_ROWS;
        ginfo->fheap_cparam.checksum_dblocks = H5G_CRT_FHEAP_CHECKSUM_DBLOCKS;
        ginfo->fheap_cparam.max_man_size = H5G_CRT_FHEAP_MAX_MAN_SIZE;
        ginfo->fheap_cparam.id_len = H5G_CRT_FHEAP_ID_LEN;
    } /* end else */

    /* Decode the filter pipeline info (if present) */
    if(flags & H5O_GINFO_STORE_PLINE) {
        /* Sanity check */
        HDassert(ginfo->version >= H5O_GINFO_VERSION_1);

        /* Decode the pipeline message */
        if(NULL == (pline = (H5O_pline_t *) H5O_MSG_PLINE->decode(f, dxpl_id, open_oh, 0, ioflags, p)))
            HGOTO_ERROR(H5E_OHDR, H5E_CANTDECODE, NULL, "can't decode heap pipeline")

        /* Copy the pipeline information to the ginfo struct */
        HDmemcpy(&(ginfo->pline), pline, sizeof(ginfo->pline));

        /* Free the temporary pipeline struct */
        pline = H5FL_FREE(H5O_pline_t, pline);
    } else {
        /* Set pline to default (empty) pipeline */
        ginfo->pline.sh_loc.msg_type_id = H5O_NULL_ID;
        ginfo->pline.sh_loc.u.loc.oh_addr = HADDR_UNDEF;
        ginfo->pline.version = H5O_PLINE_VERSION_1;
    } /* end else */

    /* The pointer is *not* guaranteed to point to the first byte past the
     * message at this point.  Do not add fields after the pipeline message
     * without modifying that code to advance the pointer. */

    /* Set return value */
    ret_value = ginfo;

done:
    if(ret_value == NULL)
        if(ginfo != NULL)
            (void)H5FL_FREE(H5O_ginfo_t, ginfo);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_ginfo_decode() */


/*-------------------------------------------------------------------------
 * Function:    H5O_ginfo_encode
 *
 * Purpose:     Encodes a message.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@ncsa.uiuc.edu
 *              Aug 30 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_ginfo_encode(H5F_t *f, hbool_t UNUSED disable_shared, uint8_t *p, const void *_mesg)
{
    const H5O_ginfo_t  *ginfo = (const H5O_ginfo_t *) _mesg;
    unsigned char       flags;          /* Flags for encoding group info */
    herr_t              ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_ginfo_encode)

    /* check args */
    HDassert(p);
    HDassert(ginfo);
    HDassert(!ginfo->store_fheap_cparam || ginfo->version >= H5O_GINFO_VERSION_1);

    /* Message version */
    *p++ = (uint8_t) ginfo->version;

    /* The flags for the group info */
    flags = ginfo->store_link_phase_change ?  H5O_GINFO_STORE_PHASE_CHANGE : 0;
    flags |= ginfo->store_est_entry_info ?  H5O_GINFO_STORE_EST_ENTRY_INFO : 0;

    /* Flags only present in version 1 and later */
    if(ginfo->version >= H5O_GINFO_VERSION_1) {
        /* The flag to indicate that the fheap creation params are stored */
        flags |= ginfo->store_fheap_cparam ? H5O_GINFO_STORE_FHEAP_CPARAM : 0;

        /* The flag to indicate if the pipeline message is encoded */
        flags |= ginfo->pline.nused ? H5O_GINFO_STORE_PLINE : 0;
    } /* end if */

    *p++ = flags;

    /* Store the max. # of links to store compactly & the min. # of links to store densely */
    if(ginfo->store_link_phase_change) {
        UINT16ENCODE(p, ginfo->max_compact)
        UINT16ENCODE(p, ginfo->min_dense)
    } /* end if */

    /* Estimated # of entries & name lengths */
    if(ginfo->store_est_entry_info) {
        UINT16ENCODE(p, ginfo->est_num_entries)
        UINT16ENCODE(p, ginfo->est_name_len)
    } /* end if */

    /* Encode fractal heap creation parameters (if appropriate) */
    if(ginfo->store_fheap_cparam) {
        /* Table width */
        UINT16ENCODE(p, ginfo->fheap_cparam.width);

        /* Starting block size */
        H5F_ENCODE_LENGTH(f, p, ginfo->fheap_cparam.start_block_size);

        /* Maximum direct block size */
        H5F_ENCODE_LENGTH(f, p, ginfo->fheap_cparam.max_direct_size);

        /* Maximum heap size (as # of bits) */
        UINT16ENCODE(p, ginfo->fheap_cparam.max_index);

        /* Starting # of rows in root indirect block */
        UINT16ENCODE(p, ginfo->fheap_cparam.start_root_rows);

        /* Whether to checksum direct blocks */
        *p++ = ginfo->fheap_cparam.checksum_dblocks ? (uint8_t)1 : (uint8_t)0;

        /* Heap ID length */
        UINT16ENCODE(p, ginfo->fheap_cparam.id_len);

        /* Max. size of "managed" objects */
        UINT32ENCODE(p, ginfo->fheap_cparam.max_man_size);
    } /* end if */

    /* Encode filter pipeline info (if uappropriate) */
    if(flags & H5O_GINFO_STORE_PLINE)
        if(H5O_MSG_PLINE->encode(f, FALSE, p, &ginfo->pline) < 0)
            HGOTO_ERROR(H5E_OHDR, H5E_CANTENCODE, FAIL, "can't encode heap pipeline")

    /* The pointer is *not* guaranteed to point to the first byte past the
     * message at this point.  Do not add fields after the pipeline message
     * without modifying that code to advance the pointer. */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_ginfo_encode() */


/*-------------------------------------------------------------------------
 * Function:    H5O_ginfo_copy
 *
 * Purpose:     Copies a message from _MESG to _DEST, allocating _DEST if
 *              necessary.
 *
 * Return:      Success:        Ptr to _DEST
 *
 *              Failure:        NULL
 *
 * Programmer:  Quincey Koziol
 *              koziol@ncsa.uiuc.edu
 *              Aug 30 2005
 *
 *-------------------------------------------------------------------------
 */
static void *
H5O_ginfo_copy(const void *_mesg, void *_dest)
{
    const H5O_ginfo_t   *ginfo = (const H5O_ginfo_t *)_mesg;
    H5O_ginfo_t         *dest = (H5O_ginfo_t *)_dest;
    void                *ret_value;     /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_ginfo_copy)

    /* check args */
    HDassert(ginfo);
    if(!dest && NULL == (dest = H5FL_MALLOC(H5O_ginfo_t)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")

    /* Shallow copy */
    *dest = *ginfo;

    /* Copy pipeline */
    if(ginfo->pline.nalloc)
        if(NULL == H5O_MSG_PLINE->copy(&ginfo->pline, &dest->pline))
            HGOTO_ERROR(H5E_OHDR, H5E_CANTINIT, NULL, "unable to copy pipeline message")

    /* Set return value */
    ret_value = dest;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_ginfo_copy() */


/*-------------------------------------------------------------------------
 * Function:    H5O_ginfo_size
 *
 * Purpose:     Returns the size of the raw message in bytes not counting
 *              the message type or size fields, but only the data fields.
 *              This function doesn't take into account alignment.
 *
 * Return:      Success:        Message data size in bytes without alignment.
 *
 *              Failure:        zero
 *
 * Programmer:  Quincey Koziol
 *              koziol@ncsa.uiuc.edu
 *              Aug 30 2005
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5O_ginfo_size(const H5F_t *f, hbool_t UNUSED disable_shared, const void *_mesg)
{
    const H5O_ginfo_t   *ginfo = (const H5O_ginfo_t *)_mesg;
    size_t pline_size;  /* Size of the embedded pipeline message */
    size_t ret_value;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_ginfo_size)

    /* Set return value */
    ret_value = 1 +                     /* Version */
                1 +                     /* Flags */
                (ginfo->store_link_phase_change ? (
                    2 +                 /* "Max compact" links */
                    2                   /* "Min dense" links */
                ) : 0) +
                (ginfo->store_est_entry_info ? (
                    2 +                 /* Estimated # of entries in group */
                    2                   /* Estimated length of name of entry in group */
                ) : 0) +
                (ginfo->store_fheap_cparam ? (
                    2 +                 /* Table width */
                    H5F_SIZEOF_SIZE(f) + /* Starting block size */
                    H5F_SIZEOF_SIZE(f) + /* Max direct block size */
                    2 +                 /* Max heap size (# of bits) */
                    2 +                 /* Starting # of rows in root indirect block */
                    1 +                 /* Whether to checksum direct blocks */
                    2 +                 /* Heap ID length */
                    4                   /* Max size of managed objects */
                ) : 0);

    /* Add pipeline message (if appropriate) */
    if(ginfo->pline.nused) {
        if((pline_size = H5O_MSG_PLINE->raw_size(f, TRUE, &ginfo->pline)) == 0)
            HGOTO_ERROR(H5E_OHDR, H5E_CANTCOUNT, 0, "unable to determine size of pipeline message")

        ret_value += pline_size;
    } /* end if */

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_ginfo_size() */


/*-------------------------------------------------------------------------
 * Function:	H5O_ginfo_reset
 *
 * Purpose:	Resets the group info message by freeing any filters in
 *              the pipeline.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Neil Fortner
 *              Friday, May 15, 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_ginfo_reset(void *mesg)
{
    H5O_ginfo_t *ginfo = (H5O_ginfo_t *) mesg;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT(H5O_ginfo_reset)

    HDassert(ginfo);

    if(H5O_MSG_PLINE->reset(&ginfo->pline) < 0)
        HGOTO_ERROR(H5E_OHDR, H5E_CANTRELEASE, FAIL, "unable to reset pipeline message")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_ginfo_reset() */



/*-------------------------------------------------------------------------
 * Function:	H5O_ginfo_free
 *
 * Purpose:	Free's the message
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Tuesday, August 30, 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_ginfo_free(void *mesg)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5O_ginfo_free)

    HDassert(mesg);

    (void)H5FL_FREE(H5O_ginfo_t, mesg);

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5O_ginfo_free() */


/*-------------------------------------------------------------------------
 * Function:    H5O_ginfo_pre_copy_file
 *
 * Purpose:     Perform any necessary actions before copying message between
 *              files
 *
 * Return:      Success:        Non-negative
 *
 *              Failure:        Negative
 *
 * Programmer:  Neil Fortner
 *              July 30, 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_ginfo_pre_copy_file(H5F_t UNUSED *file_src, const void *mesg_src,
    hbool_t UNUSED *deleted, const H5O_copy_t UNUSED *cpy_info, void *_udata)
{
    const H5O_ginfo_t *ginfo_src = (const H5O_ginfo_t *)mesg_src; /* Source ginfo */
    H5O_ginfo_t *udata = (H5O_ginfo_t *)_udata;     /* Group copying user data */
    herr_t             ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_ginfo_pre_copy_file)

    /* check args */
    HDassert(ginfo_src);
    HDassert(udata);
    HDassert(udata->fheap_cparam.version == 0); /* Verify that udata has not
                                                 * been written to - version
                                                 * must be >= 1 if written to.
                                                 */

    /* The user data should always be non-NULL, as every object with a ginfo
     * message should be a group.  Copy the ginfo message into the user data.
     */
    if(NULL == H5O_ginfo_copy(ginfo_src, udata))
        HGOTO_ERROR(H5E_OHDR, H5E_CANTINIT, FAIL, "unable to copy")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_ginfo_pre_copy_file() */


/*-------------------------------------------------------------------------
 * Function:    H5O_ginfo_debug
 *
 * Purpose:     Prints debugging info for a message.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              koziol@ncsa.uiuc.edu
 *              Aug 30 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_ginfo_debug(H5F_t *f, hid_t dxpl_id, const void *_mesg, FILE * stream,
	       int indent, int fwidth)
{
    const H5O_ginfo_t       *ginfo = (const H5O_ginfo_t *) _mesg;
    herr_t                  ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_ginfo_debug)

    /* check args */
    HDassert(f);
    HDassert(ginfo);
    HDassert(stream);
    HDassert(indent >= 0);
    HDassert(fwidth >= 0);

    HDfprintf(stream, "%*s%-*s %u\n", indent, "", fwidth,
	      "Max. compact links:", ginfo->max_compact);
    HDfprintf(stream, "%*s%-*s %u\n", indent, "", fwidth,
	      "Min. dense links:", ginfo->min_dense);
    HDfprintf(stream, "%*s%-*s %u\n", indent, "", fwidth,
	      "Estimated # of objects in group:", ginfo->est_num_entries);
    HDfprintf(stream, "%*s%-*s %u\n", indent, "", fwidth,
	      "Estimated length of object in group's name:",
	      ginfo->est_name_len);

    HDfprintf(stream, "%*sFractal heap creation parameters...\n", indent, "");
    if(H5HF_cparam_debug(&(ginfo->fheap_cparam), stream, indent + 3,
            MAX(0, fwidth - 3)) < 0)
        HGOTO_ERROR(H5E_OHDR, H5E_WRITEERROR, FAIL,
            "unable to display fractal heap creation info")

    HDfprintf(stream, "%*sFractal heap pipeline...\n", indent, "");
    if((H5O_MSG_PLINE->debug)(f, dxpl_id, &(ginfo->pline), stream, indent + 3,
            MAX(0, fwidth - 3)) < 0)
        HGOTO_ERROR(H5E_OHDR, H5E_WRITEERROR, FAIL,
                "unable to display pipeline message info")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_ginfo_debug() */


/*-------------------------------------------------------------------------
 * Function:	H5O_ginfo_set_latest_version
 *
 * Purpose:     Upgrades the provided ginfo message, and its embedded
 *              pipeline message to use the latest version of the file
 *              format.
 *
 * Return:	Success:	0
 *
 *		Failure:	Negative
 *
 * Programmer:	Neil Fortner
 *		nfortne2@hdfgroup.org
 *		May 19 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5O_ginfo_set_latest_version(struct H5O_ginfo_t *ginfo)
{
    herr_t ret_value = SUCCEED;     /* Return value */

    FUNC_ENTER_NOAPI(H5O_ginfo_set_latest_version, FAIL)

    HDassert(ginfo);

    /* Upgrade ginfo */
    ginfo->version = H5O_GINFO_VERSION_LATEST;

    /* Upgrade pline */
    if(H5O_pline_set_latest_version(&(ginfo->pline)) < 0)
        HGOTO_ERROR(H5E_LINK, H5E_CANTSET, FAIL, "can't set latest version of pipeline")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_ginfo_set_latest_version() */

