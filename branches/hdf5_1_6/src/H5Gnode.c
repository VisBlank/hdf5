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
 * Created:		snode.c
 *			Jun 26 1997
 *			Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:		Functions for handling symbol table nodes.  A
 *			symbol table node is a small collection of symbol
 *			table entries.	A B-tree usually points to the
 *			symbol table nodes for any given symbol table.
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
#define H5G_PACKAGE /*suppress error message about including H5Gpkg.h */
#define H5F_PACKAGE		/*suppress error about including H5Fpkg	  */

/* Packages needed by this file... */
#include "H5private.h"		/* Generic Functions			*/
#include "H5ACprivate.h"	/* Metadata cache			*/
#include "H5Bprivate.h"		/* B-link trees				*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5Fpkg.h"		/* File access				*/
#include "H5FLprivate.h"	/* Free Lists                           */
#include "H5Gpkg.h"		/* Groups		  		*/
#include "H5HLprivate.h"	/* Local Heaps				*/
#include "H5Iprivate.h"		/* IDs			  		*/
#include "H5MFprivate.h"	/* File memory management		*/
#include "H5MMprivate.h"	/* Memory management			*/
#include "H5Oprivate.h"		/* Object headers		  	*/
#include "H5Pprivate.h"		/* Property lists			*/

/* Private typedefs */

/*
 * Each key field of the B-link tree that points to symbol table
 * nodes consists of this structure...
 */
typedef struct H5G_node_key_t {
    size_t      offset;                 /*offset into heap for name          */
} H5G_node_key_t;

/* Private macros */
#define H5G_NODE_VERS   1               /*symbol table node version number   */
#define H5G_NODE_SIZEOF_HDR(F) (H5G_NODE_SIZEOF_MAGIC + 4)

#define PABLO_MASK	H5G_node_mask

/* PRIVATE PROTOTYPES */
static herr_t H5G_node_serialize(H5F_t *f, H5G_node_t *sym, size_t size, uint8_t *buf);
static size_t H5G_node_size(H5F_t *f);
static herr_t H5G_node_shared_free(void *shared);

/* Metadata cache callbacks */
static H5G_node_t *H5G_node_load(H5F_t *f, hid_t dxpl_id, haddr_t addr, const void *_udata1,
				 void *_udata2);
static herr_t H5G_node_flush(H5F_t *f, hid_t dxpl_id, hbool_t destroy, haddr_t addr,
			     H5G_node_t *sym);
static herr_t H5G_node_dest(H5F_t *f, H5G_node_t *sym);
static herr_t H5G_node_clear(H5G_node_t *sym);

/* B-tree callbacks */
static size_t H5G_node_sizeof_rkey(const H5F_t *f, const void *_udata);
static H5RC_t *H5G_node_get_shared(H5F_t *f, const void *_udata);
static herr_t H5G_node_create(H5F_t *f, hid_t dxpl_id, H5B_ins_t op, void *_lt_key,
			      void *_udata, void *_rt_key,
			      haddr_t *addr_p/*out*/);
static int H5G_node_cmp2(H5F_t *f, hid_t dxpl_id, void *_lt_key, void *_udata,
			  void *_rt_key);
static int H5G_node_cmp3(H5F_t *f, hid_t dxpl_id, void *_lt_key, void *_udata,
			  void *_rt_key);
static herr_t H5G_node_found(H5F_t *f, hid_t dxpl_id, haddr_t addr, const void *_lt_key,
			     void *_udata, const void *_rt_key);
static H5B_ins_t H5G_node_insert(H5F_t *f, hid_t dxpl_id, haddr_t addr, void *_lt_key,
				 hbool_t *lt_key_changed, void *_md_key,
				 void *_udata, void *_rt_key,
				 hbool_t *rt_key_changed,
				 haddr_t *new_node_p/*out*/);
static H5B_ins_t H5G_node_remove(H5F_t *f, hid_t dxpl_id, haddr_t addr, void *lt_key,
				 hbool_t *lt_key_changed, void *udata,
				 void *rt_key, hbool_t *rt_key_changed);
static herr_t H5G_node_decode_key(H5F_t *f, H5B_t *bt, uint8_t *raw,
				  void *_key);
static herr_t H5G_node_encode_key(H5F_t *f, H5B_t *bt, uint8_t *raw,
				  void *_key);
static herr_t H5G_node_debug_key(FILE *stream, H5F_t *f, hid_t dxpl_id,
                                    int indent, int fwidth, const void *key,
                                    const void *udata);

/* H5G inherits cache-like properties from H5AC */
const H5AC_class_t H5AC_SNODE[1] = {{
    H5AC_SNODE_ID,
    (H5AC_load_func_t)H5G_node_load,
    (H5AC_flush_func_t)H5G_node_flush,
    (H5AC_dest_func_t)H5G_node_dest,
    (H5AC_clear_func_t)H5G_node_clear,
}};

/* H5G inherits B-tree like properties from H5B */
H5B_class_t H5B_SNODE[1] = {{
    H5B_SNODE_ID,		/*id			*/
    sizeof(H5G_node_key_t), 	/*sizeof_nkey		*/
    H5G_node_sizeof_rkey,	/*get_sizeof_rkey	*/
    H5G_node_get_shared,	/*get_shared		*/
    H5G_node_create,		/*new			*/
    H5G_node_cmp2,		/*cmp2			*/
    H5G_node_cmp3,		/*cmp3			*/
    H5G_node_found,		/*found			*/
    H5G_node_insert,		/*insert		*/
    TRUE,			/*follow min branch?	*/
    TRUE,			/*follow max branch?	*/
    H5G_node_remove,		/*remove		*/
    H5G_node_decode_key,	/*decode		*/
    H5G_node_encode_key,	/*encode		*/
    H5G_node_debug_key,		/*debug			*/
}};

/* Interface initialization */
static int interface_initialize_g = 0;
#define INTERFACE_INIT	NULL

/* Declare a free list to manage the H5B_shared_t struct */
H5FL_EXTERN(H5B_shared_t);

/* Declare a free list to manage the H5G_node_t struct */
H5FL_DEFINE_STATIC(H5G_node_t);

/* Declare a free list to manage sequences of H5G_entry_t's */
H5FL_SEQ_DEFINE_STATIC(H5G_entry_t);

/* Declare a free list to manage blocks of symbol node data */
H5FL_BLK_DEFINE_STATIC(symbol_node);

/* Declare a free list to manage the native key offset sequence information */
H5FL_SEQ_DEFINE_STATIC(size_t);

/* Declare a free list to manage the raw page information */
H5FL_BLK_DEFINE_STATIC(grp_page);


/*-------------------------------------------------------------------------
 * Function:	H5G_node_sizeof_rkey
 *
 * Purpose:	Returns the size of a raw B-link tree key for the specified
 *		file.
 *
 * Return:	Success:	Size of the key.
 *
 *		Failure:	never fails
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul 14 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5G_node_sizeof_rkey(const H5F_t *f, const void UNUSED * udata)
{
    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_node_sizeof_rkey);

    FUNC_LEAVE_NOAPI(H5F_SIZEOF_SIZE(f));	/*the name offset */
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_get_shared
 *
 * Purpose:	Returns the shared B-tree info for the specified UDATA.
 *
 * Return:	Success:	Pointer to the raw B-tree page for this
                                file's groups
 *
 *		Failure:	Can't fail
 *
 * Programmer:	Robb Matzke
 *		Wednesday, October  8, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5RC_t *
H5G_node_get_shared(H5F_t *f, const void UNUSED *_udata)
{
    H5RC_t *rc;

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_node_get_shared);

    assert(f);

    /* Increment reference count on shared B-tree node */
    rc=H5F_GRP_BTREE_SHARED(f);
    H5RC_INC(rc);

    /* Return the pointer to the ref-count object */
    FUNC_LEAVE_NOAPI(rc);
} /* end H5G_node_get_shared() */


/*-------------------------------------------------------------------------
 * Function:	H5G_node_decode_key
 *
 * Purpose:	Decodes a raw key into a native key.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul  8 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_decode_key(H5F_t *f, H5B_t UNUSED *bt, uint8_t *raw, void *_key)
{
    H5G_node_key_t	   *key = (H5G_node_key_t *) _key;

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_node_decode_key);

    assert(f);
    assert(raw);
    assert(key);

    H5F_DECODE_LENGTH(f, raw, key->offset);

    FUNC_LEAVE_NOAPI(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_encode_key
 *
 * Purpose:	Encodes a native key into a raw key.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul  8 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_encode_key(H5F_t *f, H5B_t UNUSED *bt, uint8_t *raw, void *_key)
{
    H5G_node_key_t	   *key = (H5G_node_key_t *) _key;

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_node_encode_key);

    assert(f);
    assert(raw);
    assert(key);

    H5F_ENCODE_LENGTH(f, raw, key->offset);

    FUNC_LEAVE_NOAPI(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_debug_key
 *
 * Purpose:	Prints a key.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Friday, February 28, 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_debug_key (FILE *stream, H5F_t *f, hid_t dxpl_id, int indent, int fwidth,
		      const void *_key, const void *_udata)
{
    const H5G_node_key_t   *key = (const H5G_node_key_t *) _key;
    const H5G_bt_ud1_t	   *udata = (const H5G_bt_ud1_t *) _udata;
    const H5HL_t           *heap = NULL;
    const char		   *s;
    herr_t      ret_value=SUCCEED;       /* Return value */
    
    FUNC_ENTER_NOAPI_NOINIT(H5G_node_debug_key);
    assert (key);

    HDfprintf(stream, "%*s%-*s %u\n", indent, "", fwidth, "Heap offset:",
        (unsigned)key->offset);

    HDfprintf(stream, "%*s%-*s ", indent, "", fwidth, "Name:");

    if (NULL == (heap = H5HL_protect(f, dxpl_id, udata->heap_addr)))
	HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "unable to protect symbol name");

    s = H5HL_offset_into(f, heap, key->offset);
    HDfprintf (stream, "%s\n", s);

    if (H5HL_unprotect(f, dxpl_id, heap, udata->heap_addr) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_PROTECT, FAIL, "unable to unprotect symbol name");

done:
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_size
 *
 * Purpose:	Returns the total size of a symbol table node.
 *
 * Return:	Success:	Total size of the node in bytes.
 *
 *		Failure:	Never fails.
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 23 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5G_node_size(H5F_t *f)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_node_size);

    FUNC_LEAVE_NOAPI(H5G_NODE_SIZEOF_HDR(f) +
                     (2 * H5F_SYM_LEAF_K(f)) * H5G_SIZEOF_ENTRY(f));
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_load
 *
 * Purpose:	Loads a symbol table node from the file.
 *
 * Return:	Success:	Ptr to the new table.
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 23 1997
 *
 * Modifications:
 *		Robb Matzke, 1999-07-28
 *		The ADDR argument is passed by value.
 *
 *	Quincey Koziol, 2002-7-180
 *	Added dxpl parameter to allow more control over I/O from metadata
 *      cache.
 *-------------------------------------------------------------------------
 */
static H5G_node_t *
H5G_node_load(H5F_t *f, hid_t dxpl_id, haddr_t addr, const void UNUSED  *_udata1,
	      void UNUSED * _udata2)
{
    H5G_node_t		   *sym = NULL;
    size_t		    size = 0;
    uint8_t		   *buf = NULL;
    const uint8_t	   *p = NULL;
    H5G_node_t		   *ret_value;	/*for error handling */

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_load);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(!_udata1);
    assert(NULL == _udata2);

    /*
     * Initialize variables.
     */
    size = H5G_node_size(f);
    if ((buf=H5FL_BLK_MALLOC(symbol_node,size))==NULL)
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed for symbol table node");
    p=buf;
    if (NULL==(sym = H5FL_CALLOC(H5G_node_t)) ||
            NULL==(sym->entry=H5FL_SEQ_CALLOC(H5G_entry_t,(2*H5F_SYM_LEAF_K(f)))))
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed");
    if (H5F_block_read(f, H5FD_MEM_BTREE, addr, size, dxpl_id, buf) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_READERROR, NULL, "unable to read symbol table node");
    /* magic */
    if (HDmemcmp(p, H5G_NODE_MAGIC, H5G_NODE_SIZEOF_MAGIC))
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, NULL, "bad symbol table node signature");
    p += 4;

    /* version */
    if (H5G_NODE_VERS != *p++)
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, NULL, "bad symbol table node version");
    /* reserved */
    p++;

    /* number of symbols */
    UINT16DECODE(p, sym->nsyms);

    /* entries */
    if (H5G_ent_decode_vec(f, &p, sym->entry, sym->nsyms) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, NULL, "unable to decode symbol table entries");

    /* Set return value */
    ret_value = sym;

done:
    if (buf)
        H5FL_BLK_FREE(symbol_node,buf);
    if (!ret_value) {
        if (sym)
            if(H5G_node_dest(f, sym)<0)
                HGOTO_ERROR(H5E_SYM, H5E_CANTFREE, NULL, "unable to destroy symbol table node");
    }

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_flush
 *
 * Purpose:	Flush a symbol table node to disk.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 23 1997
 *
 * Modifications:
 *              rky, 1998-08-28
 *		Only p0 writes metadata to disk.
 *
 * 		Robb Matzke, 1999-07-28
 *		The ADDR argument is passed by value.
 *
 *	Quincey Koziol, 2002-7-180
 *	Added dxpl parameter to allow more control over I/O from metadata
 *      cache.
 *
 *      Pedro Vicente, <pvn@ncsa.uiuc.edu> 18 Sep 2002
 *      Added `id to name' support.
 * 
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_flush(H5F_t *f, hid_t dxpl_id, hbool_t destroy, haddr_t addr, H5G_node_t *sym)
{
    uint8_t	*buf = NULL;
    size_t	size;
    int		i;
    herr_t      ret_value=SUCCEED;       /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_flush);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(sym);

    /*
     * Look for dirty entries and set the node dirty flag.
     */
    for (i=0; i<sym->nsyms; i++) {
	if (sym->entry[i].dirty) {
            /* Set the node's dirty flag */
            sym->cache_info.is_dirty = TRUE;

            /* Reset the entry's dirty flag */
            sym->entry[i].dirty=FALSE;
        } /* end if */
    }

    /*
     * Write the symbol node to disk.
     */
    if (sym->cache_info.is_dirty) {
        size = H5G_node_size(f);

        /* Allocate temporary buffer */
        if ((buf=H5FL_BLK_MALLOC(symbol_node,size))==NULL)
            HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed");

        if (H5G_node_serialize(f, sym, size, buf) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_WRITEERROR, FAIL, "node serialization failed");

        if (H5F_block_write(f, H5FD_MEM_BTREE, addr, size, dxpl_id, buf) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_WRITEERROR, FAIL, "unable to write symbol table node to the file");
        H5FL_BLK_FREE(symbol_node,buf);

        /* Reset the node's dirty flag */
        sym->cache_info.is_dirty = FALSE;
    }

    /*
     * Destroy the symbol node?	 This might happen if the node is being
     * preempted from the cache.
     */
    if (destroy) {
        if(H5G_node_dest(f, sym)<0)
	    HGOTO_ERROR(H5E_SYM, H5E_CANTFREE, FAIL, "unable to destroy symbol table node");
    }

done:
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:    H5G_node_serialize
 *
 * Purpose:     Serialize the symbol table node
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Bill Wendling
 *              wendling@ncsa.uiuc.edu
 *              Sept. 16, 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_serialize(H5F_t *f, H5G_node_t *sym, size_t size, uint8_t *buf)
{
    uint8_t    *p;
    herr_t      ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_serialize);

    /* check args */
    assert(f);
    assert(sym);
    assert(buf);

    p = buf;

    /* magic number */
    HDmemcpy(p, H5G_NODE_MAGIC, H5G_NODE_SIZEOF_MAGIC);
    p += 4;

    /* version number */
    *p++ = H5G_NODE_VERS;

    /* reserved */
    *p++ = 0;

    /* number of symbols */
    UINT16ENCODE(p, sym->nsyms);

    /* entries */
    if (H5G_ent_encode_vec(f, &p, sym->entry, sym->nsyms) < 0)
        HGOTO_ERROR(H5E_SYM, H5E_CANTENCODE, FAIL, "can't serialize")
    HDmemset(p, 0, size - (p - buf));

done:
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_dest
 *
 * Purpose:	Destroy a symbol table node in memory.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Jan 15 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_dest(H5F_t UNUSED *f, H5G_node_t *sym)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_node_dest);

    /*
     * Check arguments.
     */
    assert(sym);

    /* Verify that node is clean */
    assert (sym->cache_info.is_dirty==FALSE);

    if(sym->entry)
        sym->entry = H5FL_SEQ_FREE(H5G_entry_t,sym->entry);
    H5FL_FREE(H5G_node_t,sym);

    FUNC_LEAVE_NOAPI(SUCCEED);
} /* end H5G_node_dest() */


/*-------------------------------------------------------------------------
 * Function:	H5G_node_clear
 *
 * Purpose:	Mark a symbol table node in memory as non-dirty.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Mar 20 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_clear(H5G_node_t *sym)
{
    int i;              /* Local index variable */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_node_clear);

    /*
     * Check arguments.
     */
    assert(sym);


    /* Look for dirty entries and reset the dirty flag.  */
    for (i=0; i<sym->nsyms; i++)
        sym->entry[i].dirty=FALSE;
    sym->cache_info.is_dirty = FALSE;

    FUNC_LEAVE_NOAPI(SUCCEED);
} /* end H5G_node_clear() */


/*-------------------------------------------------------------------------
 * Function:	H5G_node_create
 *
 * Purpose:	Creates a new empty symbol table node.	This function is
 *		called by the B-tree insert function for an empty tree.	 It
 *		is also called internally to split a symbol node with LT_KEY
 *		and RT_KEY null pointers.
 *
 * Return:	Success:	Non-negative.  The address of symbol table
 *				node is returned through the ADDR_P argument.
 *
 *		Failure:	Negative
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 23 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_create(H5F_t *f, hid_t dxpl_id, H5B_ins_t UNUSED op, void *_lt_key,
		void UNUSED *_udata, void *_rt_key, haddr_t *addr_p/*out*/)
{
    H5G_node_key_t	   *lt_key = (H5G_node_key_t *) _lt_key;
    H5G_node_key_t	   *rt_key = (H5G_node_key_t *) _rt_key;
    H5G_node_t		   *sym = NULL;
    hsize_t		    size = 0;
    herr_t      ret_value=SUCCEED;       /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_create);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5B_INS_FIRST == op);

    if (NULL==(sym = H5FL_CALLOC(H5G_node_t)))
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed");
    size = H5G_node_size(f);
    if (HADDR_UNDEF==(*addr_p=H5MF_alloc(f, H5FD_MEM_BTREE, dxpl_id, size)))
	HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to allocate file space");
    sym->cache_info.is_dirty = TRUE;
    sym->entry = H5FL_SEQ_CALLOC(H5G_entry_t,(2*H5F_SYM_LEAF_K(f)));
    if (NULL==sym->entry)
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed");
    if (H5AC_set(f, dxpl_id, H5AC_SNODE, *addr_p, sym) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, FAIL, "unable to cache symbol table leaf node");
    /*
     * The left and right symbols in an empty tree are both the
     * empty string stored at offset zero by the H5G functions. This
     * allows the comparison functions to work correctly without knowing
     * that there are no symbols.
     */
    if (lt_key)
        lt_key->offset = 0;
    if (rt_key)
        rt_key->offset = 0;

done:
    if(ret_value<0) {
        if(sym!=NULL) {
            if(sym->entry!=NULL)
                H5FL_SEQ_FREE(H5G_entry_t,sym->entry);
            H5FL_FREE(H5G_node_t,sym);
        } /* end if */
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_cmp2
 *
 * Purpose:	Compares two keys from a B-tree node (LT_KEY and RT_KEY).
 *		The UDATA pointer supplies extra data not contained in the
 *		keys (in this case, the heap address).
 *
 * Return:	Success:	negative if LT_KEY is less than RT_KEY.
 *
 *				positive if LT_KEY is greater than RT_KEY.
 *
 *				zero if LT_KEY and RT_KEY are equal.
 *
 *		Failure:	FAIL (same as LT_KEY<RT_KEY)
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 23 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
H5G_node_cmp2(H5F_t *f, hid_t dxpl_id, void *_lt_key, void *_udata, void *_rt_key)
{
    H5G_bt_ud1_t	   *udata = (H5G_bt_ud1_t *) _udata;
    H5G_node_key_t	   *lt_key = (H5G_node_key_t *) _lt_key;
    H5G_node_key_t	   *rt_key = (H5G_node_key_t *) _rt_key;
    const H5HL_t           *heap = NULL;
    const char		   *s1, *s2;
    const char		   *base;           /* Base of heap */
    int		    ret_value;

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_cmp2);

    assert(udata);
    assert(lt_key);
    assert(rt_key);

    /* Get base address of heap */
    if (NULL == (heap = H5HL_protect(f, dxpl_id, udata->heap_addr)))
	HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "unable to protect symbol name");

    base = H5HL_offset_into(f, heap, 0);

    /* Get pointers to string names */
    s1=base+lt_key->offset;
    s2=base+rt_key->offset;

    /* Set return value */
    ret_value = HDstrcmp(s1, s2);

done:
    if (heap && H5HL_unprotect(f, dxpl_id, heap, udata->heap_addr) < 0)
	HDONE_ERROR(H5E_SYM, H5E_PROTECT, FAIL, "unable to unprotect symbol name");

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_cmp3
 *
 * Purpose:	Compares two keys from a B-tree node (LT_KEY and RT_KEY)
 *		against another key (not necessarily the same type)
 *		pointed to by UDATA.
 *
 * Return:	Success:	negative if the UDATA key is less than
 *				or equal to the LT_KEY
 *
 *				positive if the UDATA key is greater
 *				than the RT_KEY.
 *
 *				zero if the UDATA key falls between
 *				the LT_KEY (exclusive) and the
 *				RT_KEY (inclusive).
 *
 *		Failure:	FAIL (same as UDATA < LT_KEY)
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 23 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static int
H5G_node_cmp3(H5F_t *f, hid_t dxpl_id, void *_lt_key, void *_udata, void *_rt_key)
{
    H5G_bt_ud1_t	*udata = (H5G_bt_ud1_t *) _udata;
    H5G_node_key_t	*lt_key = (H5G_node_key_t *) _lt_key;
    H5G_node_key_t	*rt_key = (H5G_node_key_t *) _rt_key;
    const H5HL_t        *heap = NULL;
    const char		*s;
    const char          *base;              /* Base of heap */
    int                  ret_value=0;       /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_cmp3);

    /* Get base address of heap */
    if (NULL == (heap = H5HL_protect(f, dxpl_id, udata->heap_addr)))
	HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "unable to protect symbol name");

    base = H5HL_offset_into(f, heap, 0);

    /* left side */
    s=base+lt_key->offset;
    if (HDstrcmp(udata->name, s) <= 0)
	HGOTO_DONE(-1);

    /* right side */
    s=base+rt_key->offset;
    if (HDstrcmp(udata->name, s) > 0)
	HGOTO_DONE(1);

done:
    if (heap && H5HL_unprotect(f, dxpl_id, heap, udata->heap_addr) < 0)
	HDONE_ERROR(H5E_SYM, H5E_PROTECT, FAIL, "unable to unprotect symbol name");

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_found
 *
 * Purpose:	The B-tree search engine has found the symbol table node
 *		which contains the requested symbol if the symbol exists.
 *		This function should examine that node for the symbol and
 *		return information about the symbol through the UDATA
 *		structure which contains the symbol name on function
 *		entry.
 *
 *		If the operation flag in UDATA is H5G_OPER_FIND, then
 *		the entry is copied from the symbol table to the UDATA
 *		entry field.  Otherwise the entry is copied from the
 *		UDATA entry field to the symbol table.
 *
 * Return:	Success:	Non-negative if found and data returned through
 *				the UDATA pointer.
 *
 *		Failure:	Negative if not found.
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 23 1997
 *
 * Modifications:
 *		Robb Matzke, 1999-07-28
 *		The ADDR argument is passed by value.
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_found(H5F_t *f, hid_t dxpl_id, haddr_t addr, const void UNUSED *_lt_key,
	       void *_udata, const void UNUSED *_rt_key)
{
    H5G_bt_ud1_t	*bt_udata = (H5G_bt_ud1_t *) _udata;
    H5G_node_t		*sn = NULL;
    const H5HL_t        *heap = NULL;
    int		lt = 0, idx = 0, rt, cmp = 1;
    const char		*s;
    const char          *base;           /* Base of heap */
    herr_t      ret_value=SUCCEED;       /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_found);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(bt_udata);

    /*
     * Load the symbol table node for exclusive access.
     */
    if (NULL == (sn = H5AC_protect(f, dxpl_id, H5AC_SNODE, addr, NULL, NULL, H5AC_READ)))
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, FAIL, "unable to protect symbol table node");

    /* Get base address of heap */
    if (NULL == (heap = H5HL_protect(f, dxpl_id, bt_udata->heap_addr)))
	HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "unable to protect symbol name");

    base = H5HL_offset_into(f, heap, 0);

    /*
     * Binary search.
     */
    rt = sn->nsyms;
    while (lt < rt && cmp) {
	idx = (lt + rt) / 2;
        s=base+sn->entry[idx].name_off;
	cmp = HDstrcmp(bt_udata->name, s);

	if (cmp < 0) {
	    rt = idx;
	} else {
	    lt = idx + 1;
	}
    }

    if (H5HL_unprotect(f, dxpl_id, heap, bt_udata->heap_addr) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_PROTECT, FAIL, "unable to unprotect symbol name");
    heap=NULL; base=NULL;

    if (cmp)
        HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "not found");

    if (bt_udata->operation==H5G_OPER_FIND)
        /*
         * The caller is querying the symbol entry, copy it into the UDATA
         * entry field. (Hmm... should this use H5G_ent_copy()? - QAK)
         */
        bt_udata->ent = sn->entry[idx];
    else
        HGOTO_ERROR(H5E_SYM, H5E_UNSUPPORTED, FAIL, "internal erorr (unknown symbol find operation)");

done:
    if (sn && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE) < 0)
	HDONE_ERROR(H5E_SYM, H5E_PROTECT, FAIL, "unable to release symbol table node");

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_insert
 *
 * Purpose:	The B-tree insertion engine has found the symbol table node
 *		which should receive the new symbol/address pair.  This
 *		function adds it to that node unless it already existed.
 *
 *		If the node has no room for the symbol then the node is
 *		split into two nodes.  The original node contains the
 *		low values and the new node contains the high values.
 *		The new symbol table entry is added to either node as
 *		appropriate.  When a split occurs, this function will
 *		write the maximum key of the low node to the MID buffer
 *		and return the address of the new node.
 *
 *		If the new key is larger than RIGHT then update RIGHT
 *		with the new key.
 *
 * Return:	Success:	An insertion command for the caller, one of
 *				the H5B_INS_* constants.  The address of the
 *				new node, if any, is returned through the
 *				NEW_NODE_P argument.  NEW_NODE_P might not be
 *				initialized if the return value is
 *				H5B_INS_NOOP.
 *
 *		Failure:	H5B_INS_ERROR, NEW_NODE_P might not be
 *				initialized.
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 24 1997
 *
 * Modifications:
 *		Robb Matzke, 1999-07-28
 *		The ADDR argument is passed by value.
 *-------------------------------------------------------------------------
 */
static H5B_ins_t
H5G_node_insert(H5F_t *f, hid_t dxpl_id, haddr_t addr, void UNUSED *_lt_key,
		hbool_t UNUSED *lt_key_changed, void *_md_key,
		void *_udata, void *_rt_key, hbool_t UNUSED *rt_key_changed,
		haddr_t *new_node_p)
{
    H5G_node_key_t	*md_key = (H5G_node_key_t *) _md_key;
    H5G_node_key_t	*rt_key = (H5G_node_key_t *) _rt_key;
    H5G_bt_ud1_t	*bt_udata = (H5G_bt_ud1_t *) _udata;

    H5G_node_t		*sn = NULL, *snrt = NULL;
    const H5HL_t        *heap = NULL;
    size_t		offset;			/*offset of name in heap */
    const char		*s;
    const char          *base;           /* Base of heap */
    int		idx = -1, cmp = 1;
    int		lt = 0, rt;		/*binary search cntrs	*/
    H5G_node_t		*insert_into = NULL;	/*node that gets new entry*/
    H5B_ins_t		ret_value = H5B_INS_ERROR;

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_insert);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(md_key);
    assert(rt_key);
    assert(bt_udata);
    assert(new_node_p);

    /*
     * Load the symbol node.
     */
    if (NULL == (sn = H5AC_protect(f, dxpl_id, H5AC_SNODE, addr, NULL, NULL, H5AC_WRITE)))
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, H5B_INS_ERROR, "unable to protect symbol table node");

    /* Get base address of heap */
    if (NULL == (heap = H5HL_protect(f, dxpl_id, bt_udata->heap_addr)))
	HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, H5B_INS_ERROR, "unable to protect symbol name");

    base = H5HL_offset_into(f, heap, 0);

    /*
     * Where does the new symbol get inserted?	We use a binary search.
     */
    rt = sn->nsyms;
    while (lt < rt) {
	idx = (lt + rt) / 2;
        s=base+sn->entry[idx].name_off;

	if (0 == (cmp = HDstrcmp(bt_udata->name, s))) /*already present */ {
            HCOMMON_ERROR(H5E_SYM, H5E_CANTINSERT, "symbol is already present in symbol table");

            if (H5HL_unprotect(f, dxpl_id, heap, bt_udata->heap_addr) < 0)
                HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to unprotect symbol name");
            heap=NULL; base=NULL;

	    HGOTO_DONE(H5B_INS_ERROR);
        }

	if (cmp < 0) {
	    rt = idx;
	} else {
	    lt = idx + 1;
	}
    }
    idx += cmp > 0 ? 1 : 0;

    if (H5HL_unprotect(f, dxpl_id, heap, bt_udata->heap_addr) < 0)
	HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to unprotect symbol name");
    heap=NULL; base=NULL;

    /*
     * Add the new name to the heap.
     */
    offset = H5HL_insert(f, dxpl_id, bt_udata->heap_addr, HDstrlen(bt_udata->name)+1,
			bt_udata->name);
    bt_udata->ent.name_off = offset;
    if (0==offset || (size_t)(-1)==offset)
	HGOTO_ERROR(H5E_SYM, H5E_CANTINSERT, H5B_INS_ERROR, "unable to insert symbol name into heap");
    if ((size_t)(sn->nsyms) >= 2*H5F_SYM_LEAF_K(f)) {
	/*
	 * The node is full.  Split it into a left and right
	 * node and return the address of the new right node (the
	 * left node is at the same address as the original node).
	 */
	ret_value = H5B_INS_RIGHT;

	/* The right node */
	if (H5G_node_create(f, dxpl_id, H5B_INS_FIRST, NULL, NULL, NULL,
			    new_node_p/*out*/)<0)
	    HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, H5B_INS_ERROR, "unable to split symbol table node");

	if (NULL == (snrt = H5AC_protect(f, dxpl_id, H5AC_SNODE, *new_node_p, NULL, NULL, H5AC_WRITE)))
	    HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, H5B_INS_ERROR, "unable to split symbol table node");

	HDmemcpy(snrt->entry, sn->entry + H5F_SYM_LEAF_K(f),
		 H5F_SYM_LEAF_K(f) * sizeof(H5G_entry_t));
	snrt->nsyms = H5F_SYM_LEAF_K(f);
	snrt->cache_info.is_dirty = TRUE;

	/* The left node */
	HDmemset(sn->entry + H5F_SYM_LEAF_K(f), 0,
		 H5F_SYM_LEAF_K(f) * sizeof(H5G_entry_t));
	sn->nsyms = H5F_SYM_LEAF_K(f);
	sn->cache_info.is_dirty = TRUE;

	/* The middle key */
	md_key->offset = sn->entry[sn->nsyms - 1].name_off;

	/* Where to insert the new entry? */
	if (idx <= (int)H5F_SYM_LEAF_K(f)) {
	    insert_into = sn;
	    if (idx == (int)H5F_SYM_LEAF_K(f))
		md_key->offset = offset;
	} else {
	    idx -= H5F_SYM_LEAF_K(f);
	    insert_into = snrt;
	    if (idx == (int)H5F_SYM_LEAF_K (f)) {
		rt_key->offset = offset;
		*rt_key_changed = TRUE;
	    }
	}
    } else {
	/* Where to insert the new entry? */
	ret_value = H5B_INS_NOOP;
	sn->cache_info.is_dirty = TRUE;
	insert_into = sn;
	if (idx == sn->nsyms) {
	    rt_key->offset = offset;
	    *rt_key_changed = TRUE;
	}
    }

    /* Move entries */
    HDmemmove(insert_into->entry + idx + 1,
	      insert_into->entry + idx,
	      (insert_into->nsyms - idx) * sizeof(H5G_entry_t));
    H5G_ent_copy(&(insert_into->entry[idx]), &(bt_udata->ent),H5G_COPY_NULL);
    insert_into->entry[idx].dirty = TRUE;
    insert_into->nsyms += 1;

done:
    if (snrt && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, *new_node_p, snrt, FALSE) < 0)
	HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to release symbol table node");
    if (sn && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE) < 0)
	HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to release symbol table node");

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_remove
 *
 * Purpose:	The B-tree removal engine has found the symbol table node
 *		which should contain the name which is being removed.  This
 *		function removes the name from the symbol table and
 *		decrements the link count on the object to which the name
 *		points.
 *
 *              If the udata->name parameter is set to NULL, then remove
 *              all entries in this symbol table node.  This only occurs
 *              during the deletion of the entire group, so don't bother
 *              freeing individual name entries in the local heap, the group's
 *              symbol table removal code will just free the entire local
 *              heap eventually.  Do reduce the link counts for each object
 *              however.
 *
 * Return:	Success:	If all names are removed from the symbol
 *				table node then H5B_INS_REMOVE is returned;
 *				otherwise H5B_INS_NOOP is returned.
 *
 *		Failure:	H5B_INS_ERROR
 *
 * Programmer:	Robb Matzke
 *              Thursday, September 24, 1998
 *
 * Modifications:
 *	Robb Matzke, 1999-07-28
 *	The ADDR argument is passed by value.
 *
 *      Pedro Vicente, <pvn@ncsa.uiuc.edu> 18 Sep 2002
 *      Added `id to name' support.
 *
 *	Quincey Koziol, 2003-03-22
 *	Added support for deleting all the entries at once.
 *
 *-------------------------------------------------------------------------
 */
static H5B_ins_t
H5G_node_remove(H5F_t *f, hid_t dxpl_id, haddr_t addr, void *_lt_key/*in,out*/,
		hbool_t UNUSED *lt_key_changed/*out*/,
		void *_udata/*in,out*/, void *_rt_key/*in,out*/,
		hbool_t *rt_key_changed/*out*/)
{
    H5G_node_key_t	*lt_key = (H5G_node_key_t*)_lt_key;
    H5G_node_key_t	*rt_key = (H5G_node_key_t*)_rt_key;
    H5G_bt_ud1_t	*bt_udata = (H5G_bt_ud1_t*)_udata;
    H5G_node_t		*sn = NULL;
    const H5HL_t        *heap = NULL;
    int		lt=0, rt, idx=0, cmp=1;
    const char		*s = NULL;
    const char          *base;              /* Base of heap */
    H5B_ins_t		ret_value = H5B_INS_ERROR;

    FUNC_ENTER_NOAPI_NOINIT(H5G_node_remove);

    /* Check arguments */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(lt_key);
    assert(rt_key);
    assert(bt_udata);

    /* Load the symbol table */
    if (NULL==(sn=H5AC_protect(f, dxpl_id, H5AC_SNODE, addr, NULL, NULL, H5AC_WRITE)))
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, H5B_INS_ERROR, "unable to protect symbol table node");

    /* "Normal" removal of a single entry from the symbol table node */
    if(bt_udata->name!=NULL) {
        size_t len=0;
        hbool_t found;     /* Indicate that the string was found */

        /* Get base address of heap */
        if (NULL == (heap = H5HL_protect(f, dxpl_id, bt_udata->heap_addr)))
            HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, H5B_INS_ERROR, "unable to protect symbol name");

        base = H5HL_offset_into(f, heap, 0);

        /* Find the name with a binary search */
        rt = sn->nsyms;
        while (lt<rt && cmp) {
            idx = (lt+rt)/2;
            s=base+sn->entry[idx].name_off;
            cmp = HDstrcmp(bt_udata->name, s);
            if (cmp<0) {
                rt = idx;
            } else {
                lt = idx+1;
            }
        }

        if (H5HL_unprotect(f, dxpl_id, heap, bt_udata->heap_addr) < 0)
            HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to unprotect symbol name");
        heap=NULL; base=NULL;

        if (cmp)
            HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, H5B_INS_ERROR, "not found");

        if (H5G_CACHED_SLINK==sn->entry[idx].type) {
            /* Remove the symbolic link value */
            if (NULL == (heap = H5HL_protect(f, dxpl_id, bt_udata->heap_addr)))
                HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, H5B_INS_ERROR, "unable to protect symbol name");

            s = H5HL_offset_into(f, heap, sn->entry[idx].cache.slink.lval_offset);
            if (s) {
                len=HDstrlen(s)+1;
                found=1;
            } /* end if */
            else
                found=0;

            if (H5HL_unprotect(f, dxpl_id, heap, bt_udata->heap_addr) < 0)
                HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to unprotect symbol name");
            heap=NULL; s=NULL;

            if (found)
                H5HL_remove(f, dxpl_id, bt_udata->heap_addr, sn->entry[idx].cache.slink.lval_offset, len);

            H5E_clear(); /* no big deal */
        } else {
            /* Decrement the reference count */
            assert(H5F_addr_defined(sn->entry[idx].header));
            if (H5O_link(sn->entry+idx, -1, dxpl_id)<0)
                HGOTO_ERROR(H5E_SYM, H5E_CANTINIT, H5B_INS_ERROR, "unable to decrement object link count");
        }
        
        /* Remove the name from the local heap */
        if (NULL == (heap = H5HL_protect(f, dxpl_id, bt_udata->heap_addr)))
            HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, H5B_INS_ERROR, "unable to protect symbol name");

        s = H5HL_offset_into(f, heap, sn->entry[idx].name_off);

        if (s) {
            len=HDstrlen(s)+1;
            found=1;
        } /* end if */
        else
            found=0;

        if (H5HL_unprotect(f, dxpl_id, heap, bt_udata->heap_addr) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to unprotect symbol name");
        heap=NULL; s=NULL;

        if (found)
            H5HL_remove(f, dxpl_id, bt_udata->heap_addr, sn->entry[idx].name_off, len);

        H5E_clear(); /* no big deal */

        /* Remove the entry from the symbol table node */
        if (1==sn->nsyms) {
            /*
             * We are about to remove the only symbol in this node. Copy the left
             * key to the right key and mark the right key as dirty.  Free this
             * node and indicate that the pointer to this node in the B-tree
             * should be removed also.
             */
            assert(0==idx);
            *rt_key = *lt_key;
            *rt_key_changed = TRUE;
            sn->nsyms = 0;
            sn->cache_info.is_dirty = TRUE;
            if (H5MF_xfree(f, H5FD_MEM_BTREE, dxpl_id, addr, (hsize_t)H5G_node_size(f))<0
                    || H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, TRUE)<0) {
                sn = NULL;
                HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to free symbol table node");
            }
            sn = NULL;
            ret_value = H5B_INS_REMOVE;
            
        } else if (0==idx) {
            /*
             * We are about to remove the left-most entry from the symbol table
             * node but there are other entries to the right.  No key values
             * change.
             */
            sn->nsyms -= 1;
            sn->cache_info.is_dirty = TRUE;
            HDmemmove(sn->entry+idx, sn->entry+idx+1,
                      (sn->nsyms-idx)*sizeof(H5G_entry_t));
            ret_value = H5B_INS_NOOP;
            
        } else if (idx+1==sn->nsyms) {
            /*
             * We are about to remove the right-most entry from the symbol table
             * node but there are other entries to the left.  The right key
             * should be changed to reflect the new right-most entry.
             */
            sn->nsyms -= 1;
            sn->cache_info.is_dirty = TRUE;
            rt_key->offset = sn->entry[sn->nsyms-1].name_off;
            *rt_key_changed = TRUE;
            ret_value = H5B_INS_NOOP;
            
        } else {
            /*
             * We are about to remove an entry from the middle of a symbol table
             * node.
             */
            sn->nsyms -= 1;
            sn->cache_info.is_dirty = TRUE;
            HDmemmove(sn->entry+idx, sn->entry+idx+1,
                      (sn->nsyms-idx)*sizeof(H5G_entry_t));
            ret_value = H5B_INS_NOOP;
        }
    } /* end if */
    /* Remove all entries from node, during B-tree deletion */
    else {
        /* Reduce the link count for all entries in this node */
        for(idx=0; idx<sn->nsyms; idx++) {
            if (H5G_CACHED_SLINK!=sn->entry[idx].type) {
                /* Decrement the reference count */
                assert(H5F_addr_defined(sn->entry[idx].header));
                if (H5O_link(sn->entry+idx, -1, dxpl_id)<0)
                    HGOTO_ERROR(H5E_SYM, H5E_CANTDELETE, H5B_INS_ERROR, "unable to decrement object link count");
            } /* end if */
        } /* end for */

        /*
         * We are about to remove all the symbols in this node. Copy the left
         * key to the right key and mark the right key as dirty.  Free this
         * node and indicate that the pointer to this node in the B-tree
         * should be removed also.
         */
        *rt_key = *lt_key;
        *rt_key_changed = TRUE;
        sn->nsyms = 0;
        sn->cache_info.is_dirty = TRUE;
        if (H5MF_xfree(f, H5FD_MEM_BTREE, dxpl_id, addr, (hsize_t)H5G_node_size(f))<0
                || H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, TRUE)<0) {
            sn = NULL;
            HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to free symbol table node");
        }
        sn = NULL;
        ret_value = H5B_INS_REMOVE;
    } /* end else */
    
done:
    if (sn && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE)<0)
	HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_INS_ERROR, "unable to release symbol table node");

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_iterate
 *
 * Purpose:	This function gets called during a group iterate operation.
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jun 24 1997
 *
 * Modifications:
 *		Robb Matzke, 1999-07-28
 *		The ADDR argument is passed by value.
 *
 *		Quincey Koziol, 2002-04-22
 *		Changed to callback from H5B_iterate
 *-------------------------------------------------------------------------
 */
int
H5G_node_iterate (H5F_t *f, hid_t dxpl_id, void UNUSED *_lt_key, haddr_t addr,
		  void UNUSED *_rt_key, void *_udata)
{
    H5G_bt_ud2_t	*bt_udata = (H5G_bt_ud2_t *)_udata;
    H5G_node_t		*sn = NULL;
    const H5HL_t        *heap = NULL;
    int		i, nsyms;
    size_t		n, *name_off=NULL;
    const char		*name;
    char		buf[1024], *s;
    int	                ret_value;

    FUNC_ENTER_NOAPI(H5G_node_iterate, H5B_ITER_ERROR);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(bt_udata);

    /*
     * Save information about the symbol table node since we can't lock it
     * because we're about to call an application function.
     */
    if (NULL == (sn = H5AC_protect(f, dxpl_id, H5AC_SNODE, addr, NULL, NULL, H5AC_READ)))
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, H5B_ITER_ERROR, "unable to load symbol table node");
    nsyms = sn->nsyms;
    if (NULL==(name_off = H5MM_malloc (nsyms*sizeof(name_off[0]))))
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, H5B_ITER_ERROR, "memory allocation failed");
    for (i=0; i<nsyms; i++)
        name_off[i] = sn->entry[i].name_off;

    if (H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE) != SUCCEED) {
        sn = NULL;
        HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_ITER_ERROR, "unable to release object header");
    }

    sn=NULL;    /* Make certain future references will be caught */

    /*
     * Iterate over the symbol table node entries.
     */
    for (i=0, ret_value=H5B_ITER_CONT; i<nsyms && !ret_value; i++) {
        if (bt_udata->skip>0) {
            --bt_udata->skip;
        } else {
            if (NULL == (heap = H5HL_protect(f, dxpl_id, bt_udata->ent->cache.stab.heap_addr)))
                HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, H5B_ITER_ERROR, "unable to protect symbol name");

            name = H5HL_offset_into(f, heap, name_off[i]);
            assert (name);
            n = HDstrlen (name);

            if (n+1>sizeof(buf)) {
                if (NULL==(s = H5MM_malloc (n+1)))
                    HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, H5B_ITER_ERROR, "memory allocation failed");
            } else {
                s = buf;
            }
            HDstrcpy (s, name);

            if (H5HL_unprotect(f, dxpl_id, heap, bt_udata->ent->cache.stab.heap_addr) < 0)
                HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_ITER_ERROR, "unable to unprotect symbol name");
            heap=NULL; name=NULL;

            ret_value = (bt_udata->op)(bt_udata->group_id, s, bt_udata->op_data);
            if (s!=buf)
                H5MM_xfree (s);
        }

        /* Increment the number of entries passed through */
        /* (whether we skipped them or not) */
        bt_udata->final_ent++;
    }
    if (ret_value<0)
        HERROR (H5E_SYM, H5E_CANTNEXT, "iteration operator failed");

done:
    if (heap && H5HL_unprotect(f, dxpl_id, heap, bt_udata->ent->cache.stab.heap_addr) < 0)
        HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_ITER_ERROR, "unable to unprotect symbol name");

    if (sn && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE) != SUCCEED)
        HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_ITER_ERROR, "unable to release object header");

    name_off = H5MM_xfree (name_off);
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_sumup
 *
 * Purpose:	This function gets called during a group iterate operation
 *              to return total number of members in the group. 
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:  Raymond Lu
 *              Nov 20, 2002
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
H5G_node_sumup(H5F_t *f, hid_t dxpl_id, void UNUSED *_lt_key, haddr_t addr,
		  void UNUSED *_rt_key, void *_udata)
{
    hsize_t	        *num_objs = (hsize_t *)_udata;
    H5G_node_t		*sn = NULL;
    int                  ret_value = H5B_ITER_CONT;

    FUNC_ENTER_NOAPI(H5G_node_sumup, H5B_ITER_ERROR);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(num_objs);

    /* Find the object node and add the number of symbol entries. */
    if (NULL == (sn = H5AC_protect(f, dxpl_id, H5AC_SNODE, addr, NULL, NULL, H5AC_READ)))
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, H5B_ITER_ERROR, "unable to load symbol table node");

    *num_objs += sn->nsyms;

done:
    if (sn && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE) != SUCCEED)
        HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_ITER_ERROR, "unable to release object header");

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_name
 *
 * Purpose:	This function gets called during a group iterate operation
 *              to return object name by giving idx.
 *
 * Return:	0 if object isn't found in this node; 1 if object is found; 
 *              Negative on failure
 *
 * Programmer:  Raymond Lu	
 *              Nov 20, 2002
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
H5G_node_name(H5F_t *f, hid_t dxpl_id, void UNUSED *_lt_key, haddr_t addr,
		  void UNUSED *_rt_key, void *_udata)
{
    H5G_bt_ud3_t	*bt_udata = (H5G_bt_ud3_t *)_udata;
    const H5HL_t        *heap = NULL;
    size_t		name_off;
    hsize_t             loc_idx;
    const char		*name;   
    H5G_node_t		*sn = NULL;
    int                 ret_value = H5B_ITER_CONT;

    FUNC_ENTER_NOAPI(H5G_node_name, H5B_ITER_ERROR);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(bt_udata);

    if (NULL == (sn = H5AC_protect(f, dxpl_id, H5AC_SNODE, addr, NULL, NULL, H5AC_READ)))
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, H5B_ITER_ERROR, "unable to load symbol table node");
    
    /* Find the node, locate the object symbol table entry and retrieve the name */ 
    if(bt_udata->idx >= bt_udata->num_objs && bt_udata->idx < (bt_udata->num_objs+sn->nsyms)) {
        loc_idx = bt_udata->idx - bt_udata->num_objs; 
        name_off = sn->entry[loc_idx].name_off;  

        if (NULL == (heap = H5HL_protect(f, dxpl_id, bt_udata->ent->cache.stab.heap_addr)))
            HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, H5B_ITER_ERROR, "unable to protect symbol name");

        name = H5HL_offset_into(f, heap, name_off);
        assert (name);
        bt_udata->name = H5MM_strdup (name);
        assert(bt_udata->name);

        if (H5HL_unprotect(f, dxpl_id, heap, bt_udata->ent->cache.stab.heap_addr) < 0)
            HGOTO_ERROR(H5E_SYM, H5E_PROTECT, H5B_ITER_ERROR, "unable to unprotect symbol name");
        heap=NULL; name=NULL;

        ret_value = H5B_ITER_STOP;
    } else {
        bt_udata->num_objs += sn->nsyms;
    }

done:
    if (sn && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE) != SUCCEED)
        HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_ITER_ERROR, "unable to release object header");

    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_type
 *
 * Purpose:	This function gets called during a group iterate operation
 *              to return object type by given idx.
 *
 * Return:	0 if object isn't found in this node; 1 if found;
 *              Negative on failure
 *
 * Programmer:  Raymond Lu	
 *              Nov 20, 2002
 *
 *
 *-------------------------------------------------------------------------
 */
int
H5G_node_type(H5F_t *f, hid_t dxpl_id, void UNUSED *_lt_key, haddr_t addr,
		  void UNUSED *_rt_key, void *_udata)
{
    H5G_bt_ud3_t	*bt_udata = (H5G_bt_ud3_t*)_udata;
    hsize_t             loc_idx;
    H5G_node_t		*sn = NULL;
    int                 ret_value = H5B_ITER_CONT;

    FUNC_ENTER_NOAPI(H5G_node_type, H5B_ITER_ERROR);

    /* Check arguments. */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(bt_udata);

    /* Find the node, locate the object symbol table entry and retrieve the type */ 
    if (NULL == (sn = H5AC_protect(f, dxpl_id, H5AC_SNODE, addr, NULL, NULL, H5AC_READ)))
	HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, H5B_ITER_ERROR, "unable to load symbol table node");

    if(bt_udata->idx >= bt_udata->num_objs && bt_udata->idx < (bt_udata->num_objs+sn->nsyms)) {
        loc_idx = bt_udata->idx - bt_udata->num_objs; 
        bt_udata->type = H5G_get_type(&(sn->entry[loc_idx]), dxpl_id); 
        ret_value = H5B_ITER_STOP;
    } else {
        bt_udata->num_objs += sn->nsyms;
    }

done:
    if (sn && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE) != SUCCEED)
        HDONE_ERROR(H5E_SYM, H5E_PROTECT, H5B_ITER_ERROR, "unable to release object header");
    
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5G_node_init
 *
 * Purpose:	This function gets called during a file opening to initialize
 *              global information about group B-tree nodes for file.
 *
 * Return:	Non-negative on success
 *              Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              Jul  5, 2004
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_node_init(H5F_t *f)
{
    H5B_shared_t *shared;               /* Shared B-tree node info */
    size_t	u;                      /* Local index variable */
    herr_t      ret_value = SUCCEED;    /* Return value */

    FUNC_ENTER_NOAPI(H5G_node_init, FAIL);

    /* Check arguments. */
    assert(f);

    /* Allocate space for the shared structure */
    if(NULL==(shared=H5FL_MALLOC(H5B_shared_t)))
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed for shared B-tree info")

    /* Set up the "global" information for this file's groups */
    shared->type= H5B_SNODE;
    shared->two_k=2*H5F_KVALUE(f,H5B_SNODE);
    shared->sizeof_rkey = H5G_node_sizeof_rkey(f, NULL);
    assert(shared->sizeof_rkey);
    shared->sizeof_rnode = H5B_nodesize(f, shared, &shared->sizeof_keys);
    assert(shared->sizeof_rnode);
    if(NULL==(shared->page=H5FL_BLK_MALLOC(grp_page,shared->sizeof_rnode)))
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed for B-tree page")
#ifdef H5_USING_PURIFY
HDmemset(shared->page,0,shared->sizeof_rnode);
#endif /* H5_USING_PURIFY */
    if(NULL==(shared->nkey=H5FL_SEQ_MALLOC(size_t,(size_t)(2*H5F_KVALUE(f,H5B_SNODE)+1))))
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, FAIL, "memory allocation failed for B-tree page")

    /* Initialize the offsets into the native key buffer */
    for(u=0; u<(2*H5F_KVALUE(f,H5B_SNODE)+1); u++)
        shared->nkey[u]=u*H5B_SNODE->sizeof_nkey;

    /* Make shared B-tree info reference counted */
    if(NULL==(f->shared->grp_btree_shared=H5RC_create(shared,H5G_node_shared_free)))
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, FAIL, "can't create ref-count wrapper for shared B-tree info")

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5G_node_init() */


/*-------------------------------------------------------------------------
 * Function:	H5G_node_close
 *
 * Purpose:	This function gets called during a file close to shutdown
 *              global information about group B-tree nodes for file.
 *
 * Return:	Non-negative on success
 *              Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              Jul  5, 2004
 *
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5G_node_close(const H5F_t *f)
{
    herr_t ret_value=SUCCEED;

    FUNC_ENTER_NOAPI(H5G_node_close,FAIL)

    /* Check arguments. */
    assert(f);

    /* Free the raw B-tree node buffer */
    H5RC_DEC(H5F_GRP_BTREE_SHARED(f));

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5G_node_close */


/*-------------------------------------------------------------------------
 * Function:	H5G_node_shared_free
 *
 * Purpose:	Free B-tree shared info
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Thursday, July  8, 2004
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5G_node_shared_free (void *_shared)
{
    H5B_shared_t *shared = (H5B_shared_t *)_shared;

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5G_node_shared_free)

    /* Free the raw B-tree node buffer */
    H5FL_BLK_FREE(grp_page,shared->page);

    /* Free the B-tree native key offsets buffer */
    H5FL_SEQ_FREE(size_t,shared->nkey);

    /* Free the shared B-tree info */
    H5FL_FREE(H5B_shared_t,shared);

    FUNC_LEAVE_NOAPI(SUCCEED)
} /* end H5G_node_shared_free() */


/*-------------------------------------------------------------------------
 * Function:	H5G_node_debug
 *
 * Purpose:	Prints debugging information about a symbol table node
 *		or a B-tree node for a symbol table B-tree.
 *
 * Return:	0(zero) on success/Negative on failure
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Aug  4 1997
 *
 * Modifications:
 *		Robb Matzke, 1999-07-28
 *		The ADDR and HEAP arguments are passed by value.
 *-------------------------------------------------------------------------
 */
herr_t
H5G_node_debug(H5F_t *f, hid_t dxpl_id, haddr_t addr, FILE * stream, int indent,
	       int fwidth, haddr_t heap)
{
    int			    i;
    H5G_node_t		   *sn = NULL;
    const char		   *s;
    const H5HL_t           *heap_ptr = NULL;
    herr_t      ret_value=SUCCEED;       /* Return value */

    FUNC_ENTER_NOAPI(H5G_node_debug, FAIL);

    /*
     * Check arguments.
     */
    assert(f);
    assert(H5F_addr_defined(addr));
    assert(stream);
    assert(indent >= 0);
    assert(fwidth >= 0);

    /*
     * If we couldn't load the symbol table node, then try loading the
     * B-tree node.
     */
    if (NULL == (sn = H5AC_protect(f, dxpl_id, H5AC_SNODE, addr, NULL, NULL, H5AC_READ))) {
        H5G_bt_ud1_t	udata;		/*data to pass through B-tree	*/

	H5E_clear(); /*discard that error */
        udata.heap_addr = heap;
	if ( H5B_debug(f, dxpl_id, addr, stream, indent, fwidth, H5B_SNODE, &udata) < 0)
	    HGOTO_ERROR(H5E_SYM, H5E_CANTLOAD, FAIL, "unable to debug B-tree node");
	HGOTO_DONE(SUCCEED);
    }
    fprintf(stream, "%*sSymbol Table Node...\n", indent, "");
    fprintf(stream, "%*s%-*s %s\n", indent, "", fwidth,
	    "Dirty:",
	    sn->cache_info.is_dirty ? "Yes" : "No");
    fprintf(stream, "%*s%-*s %u\n", indent, "", fwidth,
	    "Size of Node (in bytes):", (unsigned)H5G_node_size(f));
    fprintf(stream, "%*s%-*s %d of %d\n", indent, "", fwidth,
	    "Number of Symbols:",
	    sn->nsyms, 2 * H5F_SYM_LEAF_K(f));

    indent += 3;
    fwidth = MAX(0, fwidth - 3);
    for (i = 0; i < sn->nsyms; i++) {
	fprintf(stream, "%*sSymbol %d:\n", indent - 3, "", i);

	if (heap>0 && H5F_addr_defined(heap)) {
            if (NULL == (heap_ptr = H5HL_protect(f, dxpl_id, heap)))
                HGOTO_ERROR(H5E_SYM, H5E_NOTFOUND, FAIL, "unable to protect symbol name");

            s = H5HL_offset_into(f, heap_ptr, sn->entry[i].name_off);

            if (s)
                fprintf(stream, "%*s%-*s `%s'\n", indent, "", fwidth, "Name:", s);

            if (H5HL_unprotect(f, dxpl_id, heap_ptr, heap) < 0)
                HGOTO_ERROR(H5E_SYM, H5E_PROTECT, FAIL, "unable to unprotect symbol name");
            heap_ptr=NULL; s=NULL;
	}
        else
            fprintf(stream, "%*s%-*s\n", indent, "", fwidth, "Warning: Invalid heap address given, name not displayed!");

	H5G_ent_debug(f, dxpl_id, sn->entry + i, stream, indent, fwidth, heap);
    }

done:
    if (sn && H5AC_unprotect(f, dxpl_id, H5AC_SNODE, addr, sn, FALSE) < 0)
	HDONE_ERROR(H5E_SYM, H5E_PROTECT, FAIL, "unable to release symbol table node");

    FUNC_LEAVE_NOAPI(ret_value);
}
