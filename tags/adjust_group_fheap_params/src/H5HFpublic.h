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
 * Created:             H5HFpublic.h
 *                      Feb 24 2006
 *                      Quincey Koziol <koziol@ncsa.uiuc.edu>
 *
 * Purpose:             Public declarations for the fractal heap package.
 *
 *-------------------------------------------------------------------------
 */

#ifndef _H5HFpublic_H
#define _H5HFpublic_H

/* Public headers needed by this file */
#include "H5public.h"

/*****************/
/* Public Macros */
/*****************/
#define H5HF_CPARAM_VERSION_1 1
#define H5HF_CPARAM_VERSION_LATEST H5HF_CPARAM_VERSION_1

/*******************/
/* Public Typedefs */
/*******************/

/* Types of objects that are represented by fractal heaps */
typedef enum H5HF_type_t {
    H5HF_TYPE_UNKNOWN = -1,	/* Unknown object type */
    H5HF_TYPE_GROUP_LINK,	/* Heap contains the links for a group */
    H5HF_TYPE_NTYPES            /* Number of different object types (must be last!) */
} H5HF_type_t;

/* Creation parameters for fractal heaps */
typedef struct H5HF_cparam_t {
    unsigned    version;        /* Version of this struct */
    unsigned    width;          /* Number of columns in the table (must be power of 2) */
    size_t      start_block_size; /* Starting block size for table (must be power of 2) */
    size_t      max_direct_size; /* Maximum size of a direct block (must be power of 2) */
    unsigned    max_index;      /* Maximum ID/offset for table (integer log2 of actual value, ie. the # of bits required) */
    unsigned    start_root_rows; /* Starting number of rows for root indirect block */
                                /* 0 indicates to create the full indirect block for the root,
                                 * right from the start.  Doesn't have to be power of 2
                                 */
    hbool_t checksum_dblocks;   /* Whether the direct blocks should be checksummed */
    uint32_t max_man_size;      /* Max. size of object to manage in doubling table */
                                /* (i.e.  min. size of object to store standalone) */
    uint16_t id_len;            /* Length of IDs to use for heap objects */
                                /* (0 - make ID just large enough to hold length & offset of object in the heap) */
                                /* (1 - make ID just large enough to allow 'huge' objects to be accessed directly) */
                                /* (n - make ID 'n' bytes in size) */
} H5HF_cparam_t;

/**********************************/
/* Public API Function Prototypes */
/**********************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* _H5HFpublic_H */

