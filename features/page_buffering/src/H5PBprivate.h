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
 * Created:		H5PBprivate.h
 *			June 2014
 *			Mohamad Chaarawi
 *
 *-------------------------------------------------------------------------
 */

#ifndef _H5PBprivate_H
#define _H5PBprivate_H

/* Include package's public header */
#include "H5PBpublic.h"

/* Private headers needed by this header */
#include "H5private.h"		/* Generic Functions			*/
#include "H5Fprivate.h"		/* File access				*/
#include "H5SLprivate.h"	/* Skip List				*/

/* Typedef for the main structure for the page buffer (defined in H5PBpkg.h) */
typedef struct H5PB_entry_t H5PB_entry_t;

#define H5PB_COLLECT_STATS 1

typedef struct H5PB_t
{
    size_t                   max_size;         /* The total page buffer size */
    size_t                   page_size;        /* size of a single page */
    unsigned                 min_meta_perc;    /* minimum ratio of metadata entries required before evicting meta entries */
    unsigned                 min_raw_perc;     /* minimum ratio of raw data entries required before evicting raw entries */
    unsigned                 meta_count;       /* Number of entries for metadata */
    unsigned                 raw_count;        /* Number of entries for raw data */
    unsigned                 min_meta_count;   /* Minimum Number of entries for metadata */
    unsigned                 min_raw_count;    /* Minimum Number of entries for raw data */

    H5SL_t                  *slist_ptr;        /* Skip list with all the active page entries */
    H5SL_t                  *mf_slist_ptr;     /* Skip list containing newly allocated page entries inserted from the MF layer */

    size_t                   LRU_list_len;     /* number of entries in the LRU (identical to slist_ptr count) */
    H5PB_entry_t            *LRU_head_ptr;     /* Head pointer of the LRU */
    H5PB_entry_t            *LRU_tail_ptr;     /* Tail pointer of the LRU */

#if H5PB_COLLECT_STATS
    int                      accesses[2];
    int                      hits[2];
    int                      misses[2];
    int                      evictions[2];
    int                      bypasses[2];
#endif
} H5PB_t;

H5_DLL herr_t H5PB_create(H5F_t *file, size_t page_buffer_size, unsigned page_buf_min_meta_perc, unsigned page_buf_min_raw_perc);
H5_DLL herr_t H5PB_flush(H5F_t *f, hid_t dxpl_id, hbool_t closing);
H5_DLL herr_t H5PB_dest(H5F_t *file, hid_t dxpl_id);
H5_DLL herr_t H5PB_add_new_page(H5F_t *f, H5FD_mem_t type, haddr_t page_addr);
#ifdef H5_HAVE_PARALLEL
H5_DLL herr_t H5PB_update_entry(H5PB_t *page_buf, haddr_t addr, size_t size, const void *buf);
#endif /* H5_HAVE_PARALLEL */
H5_DLL herr_t H5PB_read(const H5F_t *f, H5FD_mem_t type, haddr_t addr, size_t size,
                        hid_t dxpl_id, void *buf/*out*/);
H5_DLL herr_t H5PB_write(const H5F_t *f, H5FD_mem_t type, haddr_t addr, size_t size,
                         hid_t dxpl_id, const void *buf);
#if H5PB_COLLECT_STATS
H5_DLL herr_t H5PB_print_stats(const H5PB_t *page_buf);
H5_DLL herr_t H5PB_reset_stats(H5PB_t *page_buf);
#endif

#endif /* !_H5PBprivate_H */
