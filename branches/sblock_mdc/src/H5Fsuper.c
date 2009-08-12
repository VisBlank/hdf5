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

/****************/
/* Module Setup */
/****************/

#define H5F_PACKAGE		/*suppress error about including H5Fpkg	  */

/* Interface initialization */
#define H5_INTERFACE_INIT_FUNC	H5F_init_super_interface


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions                    */
#include "H5ACprivate.h"        /* Metadata cache                       */
#include "H5Eprivate.h"		/* Error handling                       */
#include "H5Fpkg.h"             /* File access                          */
#include "H5FDprivate.h"	/* File drivers                         */
#include "H5Iprivate.h"		/* IDs                                  */
#include "H5MFprivate.h"	/* File memory management               */
#include "H5MMprivate.h"	/* Memory management                    */
#include "H5Pprivate.h"		/* Property lists                       */
#include "H5SMprivate.h"        /* Shared Object Header Messages        */


/****************/
/* Local Macros */
/****************/

/* Superblock sizes for various versions */
#define H5F_SIZEOF_CHKSUM 4     /* Checksum size in the file */

/* Fixed-size portion at the beginning of all superblocks */
#define H5F_SUPERBLOCK_FIXED_SIZE ( H5F_SIGNATURE_LEN                   \
        + 1) /* superblock version */

/* Macros for computing variable-size superblock size */
#define H5F_SUPERBLOCK_VARLEN_SIZE_COMMON                               \
        (2  /* freespace, and root group versions */			\
        + 1 /* reserved */                                              \
        + 3 /* shared header vers, size of address, size of lengths */  \
        + 1 /* reserved */                                              \
        + 4 /* group leaf k, group internal k */                        \
        + 4) /* consistency flags */
#define H5F_SUPERBLOCK_VARLEN_SIZE_V0(f)                                \
        ( H5F_SUPERBLOCK_VARLEN_SIZE_COMMON /* Common variable-length info */ \
        + H5F_SIZEOF_ADDR(f) /* base address */                         \
        + H5F_SIZEOF_ADDR(f) /* <unused> */				\
        + H5F_SIZEOF_ADDR(f) /* EOF address */                          \
        + H5F_SIZEOF_ADDR(f) /* driver block address */                 \
        + H5G_SIZEOF_ENTRY(f)) /* root group ptr */
#define H5F_SUPERBLOCK_VARLEN_SIZE_V1(f)                                \
        ( H5F_SUPERBLOCK_VARLEN_SIZE_COMMON /* Common variable-length info */ \
        + 2 /* indexed B-tree internal k */                             \
        + 2 /* reserved */                                              \
        + H5F_SIZEOF_ADDR(f) /* base address */                         \
        + H5F_SIZEOF_ADDR(f) /* <unused> */				\
        + H5F_SIZEOF_ADDR(f) /* EOF address */                          \
        + H5F_SIZEOF_ADDR(f) /* driver block address */                 \
        + H5G_SIZEOF_ENTRY(f)) /* root group ptr */
#define H5F_SUPERBLOCK_VARLEN_SIZE_V2(f)                                \
        ( 2 /* size of address, size of lengths */                      \
        + 1 /* consistency flags */                                     \
        + H5F_SIZEOF_ADDR(f) /* base address */                         \
        + H5F_SIZEOF_ADDR(f) /* superblock extension address */         \
        + H5F_SIZEOF_ADDR(f) /* EOF address */                          \
        + H5F_SIZEOF_ADDR(f) /* root group object header address */     \
        + H5F_SIZEOF_CHKSUM) /* superblock checksum (keep this last) */
#define H5F_SUPERBLOCK_VARLEN_SIZE(v, f) (				\
        (v == 0 ? H5F_SUPERBLOCK_VARLEN_SIZE_V0(f) : 0)			\
        + (v == 1 ? H5F_SUPERBLOCK_VARLEN_SIZE_V1(f) : 0)               \
        + (v == 2 ? H5F_SUPERBLOCK_VARLEN_SIZE_V2(f) : 0))

/* Total size of superblock, depends on superblock version */
#define H5F_SUPERBLOCK_SIZE(v, f) ( H5F_SUPERBLOCK_FIXED_SIZE           \
        + H5F_SUPERBLOCK_VARLEN_SIZE(v, f))

/* Driver info block macros */
#define H5F_DRVINFOBLOCK_HDR_SIZE 16

/* Maximum size of super-block buffers */
#define H5F_MAX_SUPERBLOCK_SIZE  134
#define H5F_MAX_DRVINFOBLOCK_SIZE  1024


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

/* Declare a free list to manage the H5F_super_t struct */
H5FL_DEFINE(H5F_super_t);


/*******************/
/* Local Variables */
/*******************/


/*--------------------------------------------------------------------------
NAME
   H5F_init_super_interface -- Initialize interface-specific information
USAGE
    herr_t H5F_init_super_interface()

RETURNS
    Non-negative on success/Negative on failure
DESCRIPTION
    Initializes any interface-specific data or routines.  (Just calls
    H5F_init() currently).

--------------------------------------------------------------------------*/
static herr_t
H5F_init_super_interface(void)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5F_init_super_interface)

    FUNC_LEAVE_NOAPI(H5F_init())
} /* H5F_init_super_interface() */


/*-------------------------------------------------------------------------
 * Function:	H5F_locate_signature
 *
 * Purpose:	Finds the HDF5 superblock signature in a file.	The signature
 *		can appear at address 0, or any power of two beginning with
 *		512.
 *
 * Return:	Success:	The absolute format address of the signature.
 *
 *		Failure:	HADDR_UNDEF
 *
 * Programmer:	Robb Matzke
 *		Friday, November  7, 1997
 *
 *-------------------------------------------------------------------------
 */
haddr_t
H5F_locate_signature(H5FD_t *file, hid_t dxpl_id)
{
    haddr_t	    addr, eoa;
    uint8_t	    buf[H5F_SIGNATURE_LEN];
    unsigned	    n, maxpow;
    haddr_t         ret_value;       /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5F_locate_signature)

    /* Find the least N such that 2^N is larger than the file size */
    if(HADDR_UNDEF == (addr = H5FD_get_eof(file)) || HADDR_UNDEF == (eoa = H5FD_get_eoa(file, H5FD_MEM_SUPER)))
	HGOTO_ERROR(H5E_IO, H5E_CANTINIT, HADDR_UNDEF, "unable to obtain EOF/EOA value")
    for(maxpow = 0; addr; maxpow++)
        addr >>= 1;
    maxpow = MAX(maxpow, 9);

    /*
     * Search for the file signature at format address zero followed by
     * powers of two larger than 9.
     */
    for(n = 8; n < maxpow; n++) {
	addr = (8 == n) ? 0 : (haddr_t)1 << n;
	if(H5FD_set_eoa(file, H5FD_MEM_SUPER, addr + H5F_SIGNATURE_LEN) < 0)
	    HGOTO_ERROR(H5E_IO, H5E_CANTINIT, HADDR_UNDEF, "unable to set EOA value for file signature")
	if(H5FD_read(file, dxpl_id, H5FD_MEM_SUPER, addr, (size_t)H5F_SIGNATURE_LEN, buf) < 0)
	    HGOTO_ERROR(H5E_IO, H5E_CANTINIT, HADDR_UNDEF, "unable to read file signature")
	if(!HDmemcmp(buf, H5F_SIGNATURE, (size_t)H5F_SIGNATURE_LEN))
            break;
    } /* end for */

    /*
     * If the signature was not found then reset the EOA value and return
     * failure.
     */
    if(n >= maxpow) {
	(void)H5FD_set_eoa(file, H5FD_MEM_SUPER, eoa); /* Ignore return value */
	HGOTO_ERROR(H5E_IO, H5E_CANTINIT, HADDR_UNDEF, "unable to find a valid file signature")
    } /* end if */

    /* Set return value */
    ret_value = addr;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_locate_signature() */


/*-------------------------------------------------------------------------
 * Function:    H5F_super_read
 *
 * Purpose:     Reads the superblock from the file or from the BUF. If
 *              ADDR is a valid address, then it reads it from the file.
 *              If not, then BUF must be non-NULL for it to read from the
 *              BUF.
 *
 * Return:      Success:        SUCCEED
 *              Failure:        FAIL
 *
 * Programmer:  Bill Wendling
 *              wendling@ncsa.uiuc.edu
 *              Sept 12, 2003
 *
 * Modifications:
 *
 *              Mike McGreevy, May 26, 2009
 *              Moved primary functional code out of H5F_super_read
 *              and into cache callbacks. This function now calls
 *              H5AC_protect which will read the superblock from disk.
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_super_read(H5F_t *f, hid_t dxpl_id, hbool_t dirty_sblock_after_read)
{
    H5P_genplist_t     *c_plist;            /* File creation property list                  */
    H5F_file_t         *shared;             /* shared part of `file'                        */
    H5FD_t             *lf;                 /* file driver part of `shared'                 */
    H5F_super_t *       sblock = NULL;      /* superblock structure                         */
    hbool_t             dirtied = FALSE;    /* Bool for sblock protect call                 */
    H5AC_protect_t      rw;                 /* read/write permissions for file              */
    unsigned            sblock_flags;       /* flags used in superblock unprotect call      */
    herr_t              ret_value = SUCCEED; /* return value                                */

    FUNC_ENTER_NOAPI(H5F_super_read, FAIL)

    /* Short cuts */
    shared = f->shared;
    lf = shared->lf;

    /* Get the shared file creation property list */
    if(NULL == (c_plist = (H5P_genplist_t *)H5I_object(shared->fcpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "can't get property list")

    /* Find the superblock */
    if(HADDR_UNDEF == (f->shared->super_addr = H5F_locate_signature(lf, dxpl_id)))
        HGOTO_ERROR(H5E_FILE, H5E_NOTHDF5, FAIL, "unable to find file signature")

    /* Determine file intent for superblock protect */
    if(H5F_INTENT(f) & H5F_ACC_RDWR)
        rw = H5AC_WRITE;
    else
        rw = H5AC_READ;

    /* Look up the superblock */
    if(NULL == (sblock = (H5F_super_t *)H5AC_protect(f, dxpl_id, H5AC_SUPERBLOCK, f->shared->super_addr, NULL, &dirtied, rw)))
        HGOTO_ERROR(H5E_CACHE, H5E_CANTPROTECT, FAIL, "unable to load superblock")
    
    /* Pin the superblock in the cache */
    if(H5AC_pin_protected_entry(f, sblock) < 0)
        HGOTO_ERROR(H5E_FSPACE, H5E_CANTPIN, FAIL, "unable to pin superblock")

done:
    if((rw == H5AC_WRITE) && ((dirtied) || dirty_sblock_after_read))
        sblock_flags = H5AC__DIRTIED_FLAG;
    else
        sblock_flags = H5AC__NO_FLAGS_SET;

    /* Release the superblock */
    if(sblock && H5AC_unprotect(f, dxpl_id, H5AC_SUPERBLOCK, f->shared->super_addr, sblock, sblock_flags) < 0)
        HDONE_ERROR(H5E_CACHE, H5E_CANTUNPROTECT, FAIL, "unable to close superblock")

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_super_read() */


/*-------------------------------------------------------------------------
 * Function:    H5F_super_init
 *
 * Purpose:     Allocates the superblock for the file and initializes
 *              information about the superblock in memory.  Writes extension
 *              messages if any are needed.
 *
 * Return:      Success:        SUCCEED
 *              Failure:        FAIL
 *
 * Programmer:  Quincey Koziol
 *              koziol@ncsa.uiuc.edu
 *              Sept 15, 2003
 *
 * Modifications:
 *
 *              Mike McGreevy, May 26, 2009
 *              Some property list initialization routine was moved
 *              from H5F_open() to here, since the superblock won't exist
 *              until this function is called. After initialization, the 
 *              superblock is then added to the cache, as opposed to
 *              the shared file structure.
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_super_init(H5F_t *f, hid_t dxpl_id)
{
    H5F_super_t *   sblock;             /* superblock cache structure                 */
    H5P_genplist_t *plist;              /* File creation property list                */
    hsize_t         userblock_size;     /* Size of userblock, in bytes                */
    hsize_t         superblock_size;    /* Size of superblock, in bytes               */
    size_t          driver_size;        /* Size of driver info block (bytes)          */
    unsigned super_vers = HDF5_SUPERBLOCK_VERSION_DEF; /* Superblock version for file */
    haddr_t         super_addr;         /* Address of superblock                      */
    hbool_t         need_ext;           /* Whether the superblock extension is needed */
    herr_t          ret_value = SUCCEED; /* Return Value                              */

    FUNC_ENTER_NOAPI(H5F_super_init, FAIL)

    /* Allocate space for the superblock */
    if(NULL == (sblock = H5FL_CALLOC(H5F_super_t)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed")

    /* Initialize various address information */
    sblock->base_addr = HADDR_UNDEF;
    sblock->extension_addr = HADDR_UNDEF;
    sblock->driver_addr = HADDR_UNDEF;
    sblock->root_addr = HADDR_UNDEF;

    /* Get the shared file creation property list */
    if(NULL == (plist = (H5P_genplist_t *)H5I_object(f->shared->fcpl_id)))
        HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not a property list")

    /* Initialize sym_leaf_k */
    if(H5P_get(plist, H5F_CRT_SYM_LEAF_NAME, &sblock->sym_leaf_k) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "can't get byte number for object size")

    /* Initialize btree_k */
    if(H5P_get(plist, H5F_CRT_BTREE_RANK_NAME, &sblock->btree_k[0]) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "unable to get rank for btree internal nodes")

    /* Bump superblock version if we are to use the latest version of the format */
    if(f->shared->latest_format)
        super_vers = HDF5_SUPERBLOCK_VERSION_LATEST;
    /* Bump superblock version to create superblock extension for SOHM info */
    else if(f->shared->sohm_nindexes > 0)
        super_vers = HDF5_SUPERBLOCK_VERSION_2;
    /* Check for non-default indexed storage B-tree internal 'K' value
     * and set the version # of the superblock to 1 if it is a non-default
     * value.
     */
    else if(sblock->btree_k[H5B_CHUNK_ID] != HDF5_BTREE_CHUNK_IK_DEF)
        super_vers = HDF5_SUPERBLOCK_VERSION_1;

    /* If a newer superblock version is required, set it here */
    if(super_vers != HDF5_SUPERBLOCK_VERSION_DEF) {
        H5P_genplist_t *c_plist;              /* Property list */

        if(NULL == (c_plist = (H5P_genplist_t *)H5I_object(f->shared->fcpl_id)))
            HGOTO_ERROR(H5E_ARGS, H5E_BADTYPE, FAIL, "not property list")
        if(H5P_set(c_plist, H5F_CRT_SUPER_VERS_NAME, &super_vers) < 0)
            HGOTO_ERROR(H5E_PLIST, H5E_CANTSET, FAIL, "unable to set superblock version")
    } /* end if */

    /*
     * The superblock starts immediately after the user-defined
     * header, which we have already insured is a proper size. The
     * base address is set to the same thing as the superblock for
     * now.
     */
    if(H5P_get(plist, H5F_CRT_USER_BLOCK_NAME, &userblock_size) < 0)
        HGOTO_ERROR(H5E_FILE, H5E_CANTGET, FAIL, "unable to get userblock size")

    sblock->base_addr = userblock_size;
    sblock->status_flags = 0;

    /* Reserve space for the userblock */
    if(H5FD_set_eoa(f->shared->lf, H5FD_MEM_SUPER, userblock_size) < 0)
        HGOTO_ERROR(H5E_FILE, H5E_CANTINIT, FAIL, "unable to set EOA value for userblock")

    /* Set the base address for the file in the VFD now, after allocating
     *  space for userblock.
     */
    if(H5FD_set_base_addr(f->shared->lf, sblock->base_addr) < 0)
        HGOTO_ERROR(H5E_FILE, H5E_CANTINIT, FAIL, "failed to set base address for file driver")

    /* Grab superblock version from property list */
    if(H5P_get(plist, H5F_CRT_SUPER_VERS_NAME, &super_vers) < 0)
        HGOTO_ERROR(H5E_PLIST, H5E_CANTGET, FAIL, "unable to get superblock version")
    
    /* Save a local copy of the superblock version number */
    sblock->super_vers = super_vers;

    /* Compute the size of the superblock */
    superblock_size = H5F_SUPERBLOCK_SIZE(super_vers, f);

    /* Compute the size of the driver information block */
    H5_ASSIGN_OVERFLOW(driver_size, H5FD_sb_size(f->shared->lf), hsize_t, size_t);
    if(driver_size > 0) {
        driver_size += H5F_DRVINFOBLOCK_HDR_SIZE;

        /*
         * The file driver information block begins immediately after the
         * superblock. (relative to base address in file)
         */
        sblock->driver_addr = superblock_size;
    } /* end if */

    /*
     * Allocate space for the userblock, superblock & driver info blocks.
     * We do it with one allocation request because the userblock and
     * superblock need to be at the beginning of the file and only the first
     * allocation request is required to return memory at format address zero.
     */
    if(super_vers < HDF5_SUPERBLOCK_VERSION_2)
        superblock_size += driver_size;
    super_addr = H5MF_alloc(f, H5FD_MEM_SUPER, dxpl_id, superblock_size);
    if(HADDR_UNDEF == super_addr)
        HGOTO_ERROR(H5E_FILE, H5E_CANTINIT, FAIL, "unable to allocate file space for userblock and/or superblock")
    if(0 != super_addr)
        HGOTO_ERROR(H5E_FILE, H5E_CANTINIT, FAIL, "file driver failed to allocate userblock and/or superblock at address zero")

    /*
     * Determine if we will need a superblock extension
     */

    /* Files with SOHM indices always need the superblock extension */
    if(f->shared->sohm_nindexes > 0) {
        HDassert(super_vers >= HDF5_SUPERBLOCK_VERSION_2);
        need_ext = TRUE;
    } /* end if */
    /* If we're going to use a version of the superblock format which allows
     *  for the superblock extension, check for non-default values to store
     *  in it.
     */
    else if(super_vers >= HDF5_SUPERBLOCK_VERSION_2) {
        /* Check for non-default v1 B-tree 'K' values to store */
        if(sblock->btree_k[H5B_SNODE_ID] != HDF5_BTREE_SNODE_IK_DEF ||
                sblock->btree_k[H5B_CHUNK_ID] != HDF5_BTREE_CHUNK_IK_DEF ||
                sblock->sym_leaf_k != H5F_CRT_SYM_LEAF_DEF)
            need_ext = TRUE;
        /* Check for driver info to store */
        else if(driver_size > 0)
            need_ext = TRUE;
        else
            need_ext = FALSE;
    } /* end if */
    else
        need_ext = FALSE;

    /* Create the superblock extension for "extra" superblock data, if necessary. */
    if(need_ext) {
        H5O_loc_t       ext_loc;            /* Superblock extension object location */

        /* The superblock extension isn't actually a group, but the
         * default group creation list should work fine.
         * If we don't supply a size for the object header, HDF5 will
         * allocate H5O_MIN_SIZE by default.  This is currently
         * big enough to hold the biggest possible extension, but should
         * be tuned if more information is added to the superblock
         * extension.
         */
        H5O_loc_reset(&ext_loc);
        if(H5O_create(f, dxpl_id, 0, H5P_GROUP_CREATE_DEFAULT, &ext_loc) < 0)
            HGOTO_ERROR(H5E_OHDR, H5E_CANTCREATE, FAIL, "unable to create superblock extension")

        /* Record the address of the superblock extension */
        sblock->extension_addr = ext_loc.addr;

        /* Create the Shared Object Header Message table and register it with
         *      the metadata cache, if this file supports shared messages.
         */
        if(f->shared->sohm_nindexes > 0) {
            /* Initialize the shared message code & write the SOHM message to the extension */
            if(H5SM_init(f, plist, &ext_loc, dxpl_id) < 0)
                HGOTO_ERROR(H5E_FILE, H5E_CANTINIT, FAIL, "unable to create SOHM table")
        } /* end if */

        /* Check for non-default v1 B-tree 'K' values to store */
        if(sblock->btree_k[H5B_SNODE_ID] != HDF5_BTREE_SNODE_IK_DEF ||
                sblock->btree_k[H5B_CHUNK_ID] != HDF5_BTREE_CHUNK_IK_DEF ||
                sblock->sym_leaf_k != H5F_CRT_SYM_LEAF_DEF) {
            H5O_btreek_t btreek;        /* v1 B-tree 'K' value message for superblock extension */

            /* Write v1 B-tree 'K' value information to the superblock extension */
            btreek.btree_k[H5B_CHUNK_ID] = sblock->btree_k[H5B_CHUNK_ID];
            btreek.btree_k[H5B_SNODE_ID] = sblock->btree_k[H5B_SNODE_ID];
            btreek.sym_leaf_k = sblock->sym_leaf_k;
            if(H5O_msg_create(&ext_loc, H5O_BTREEK_ID, H5O_MSG_FLAG_CONSTANT | H5O_MSG_FLAG_DONTSHARE, H5O_UPDATE_TIME, &btreek, dxpl_id) < 0)
                HGOTO_ERROR(H5E_FILE, H5E_CANTINIT, FAIL, "unable to update v1 B-tree 'K' value header message")
        } /* end if */

        /* Check for driver info to store */
        if(driver_size > 0) {
            H5O_drvinfo_t drvinfo;      /* Driver info */
            uint8_t dbuf[H5F_MAX_DRVINFOBLOCK_SIZE];  /* Driver info block encoding buffer */

            /* Sanity check */
            HDassert(driver_size <= H5F_MAX_DRVINFOBLOCK_SIZE);

            /* Encode driver-specific data */
            if(H5FD_sb_encode(f->shared->lf, drvinfo.name, dbuf) < 0)
                HGOTO_ERROR(H5E_FILE, H5E_CANTINIT, FAIL, "unable to encode driver information")

            /* Write driver info information to the superblock extension */
            drvinfo.len = driver_size;
            drvinfo.buf = dbuf;
            if(H5O_msg_create(&ext_loc, H5O_DRVINFO_ID, H5O_MSG_FLAG_DONTSHARE, H5O_UPDATE_TIME, &drvinfo, dxpl_id) < 0)
                HGOTO_ERROR(H5E_FILE, H5E_CANTINIT, FAIL, "unable to update driver info header message")
        } /* end if */

        /* Twiddle the number of open objects to avoid closing the file
         * (since this will be the only open object currently).
         */
        f->nopen_objs++;
        if(H5O_close(&ext_loc) < 0)
            HGOTO_ERROR(H5E_OHDR, H5E_CANTRELEASE, FAIL, "unable to close superblock extension")
        f->nopen_objs--;
    } /* end if */

    if(H5AC_set(f, dxpl_id, H5AC_SUPERBLOCK, super_addr, sblock, H5AC__PIN_ENTRY_FLAG) < 0)
        HGOTO_ERROR(H5E_CACHE, H5E_CANTINS, FAIL, "can't add superblock to cache")

    /* Keep a local copy of the superblock location */
    f->shared->super_addr = super_addr;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5F_super_init() */


/*-------------------------------------------------------------------------
 * Function:    H5F_super_ext_size
 *              Get storage size of the superblock extension
 *
 * Return:      Success:        non-negative on success
 *              Failure:        Negative
 *
 * Programmer:  Vailin Choi
 *              July 11, 2007
 *
 * Modifications:     
 *
 *              Mike McGreevy, June 2, 2009
 *              Added protect/unprotect calls in order to access the
 *              superblock, now stored in the cache.
 *              
 *-------------------------------------------------------------------------
 */
herr_t
H5F_super_ext_size(H5F_t *f, hid_t dxpl_id, hsize_t *super_ext_size)
{
    H5O_loc_t ext_loc;                  /* "Object location" for superblock extension */
    H5O_info_t oinfo;                   /* Object info for superblock extension */
    herr_t ret_value = SUCCEED;         /* Return value */
    H5F_super_t * sblock = NULL;        /* superblock object */

    FUNC_ENTER_NOAPI(H5F_super_ext_size, FAIL)

    /* Sanity check */
    HDassert(f);
    HDassert(super_ext_size);

    /* Look up the superblock */
    if(NULL == (sblock = (H5F_super_t *)H5AC_protect(f, H5AC_dxpl_id, H5AC_SUPERBLOCK, f->shared->super_addr, NULL, NULL, H5AC_READ)))
        HGOTO_ERROR(H5E_CACHE, H5E_CANTPROTECT, FAIL, "unable to load superblock")

    /* Set up "fake" object location for superblock extension */
    H5O_loc_reset(&ext_loc);
    ext_loc.file = f;
    ext_loc.addr = sblock->extension_addr;

    /* Release the superblock */
    if(sblock && H5AC_unprotect(f, H5AC_dxpl_id, H5AC_SUPERBLOCK, f->shared->super_addr, sblock, H5AC__NO_FLAGS_SET) <0)
        HDONE_ERROR(H5E_CACHE, H5E_CANTUNPROTECT, FAIL, "unable to close superblock")

    /* Get object header info for superblock extension */
    if(H5O_get_info(&ext_loc, dxpl_id, FALSE, &oinfo) < 0)
	HGOTO_ERROR(H5E_FILE, H5E_CANTGET, FAIL, "unable to retrieve superblock extension info")

    /* Set the superblock extension size */
    *super_ext_size = oinfo.hdr.space.total;

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* H5F_super_ext_size() */


/*-------------------------------------------------------------------------
 * Function:    H5F_super_unpin
 *
 * Purpose:     Unpin the superblock
 *
 * Return:      Success:        non-negative on success
 *              Failure:        Negative
 *
 * Programmer:  Quincey Koziol
 *              August 10, 2009
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5F_super_unpin(H5F_t *f, hid_t dxpl_id)
{
    H5F_super_t * sblock = NULL;        /* superblock object */
    herr_t ret_value = SUCCEED;         /* Return value */

    FUNC_ENTER_NOAPI(H5F_super_unpin, FAIL)

    /* Sanity check */
    HDassert(f);
    HDassert(H5F_addr_defined(f->shared->super_addr));

    /* Look up the superblock */
    if(NULL == (sblock = (H5F_super_t *)H5AC_protect(f, dxpl_id, H5AC_SUPERBLOCK, f->shared->super_addr, NULL, NULL, H5AC_READ)))
        HGOTO_ERROR(H5E_CACHE, H5E_CANTPROTECT, FAIL, "unable to load superblock")

    /* Unpin the superblock, since we're about to destroy the cache */
    if(H5AC_unpin_entry(f, sblock) < 0)
        HGOTO_ERROR(H5E_FSPACE, H5E_CANTUNPIN, FAIL, "unable to unpin superblock")

done:
    /* Release the superblock */
    if(sblock && H5AC_unprotect(f, dxpl_id, H5AC_SUPERBLOCK, f->shared->super_addr, sblock, H5AC__NO_FLAGS_SET) < 0)
        HDONE_ERROR(H5E_CACHE, H5E_CANTUNPROTECT, FAIL, "unable to close superblock")

    FUNC_LEAVE_NOAPI(ret_value)
} /* H5F_super_unpin() */

