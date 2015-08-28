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

#ifndef H5PB_PACKAGE
#error "Do not include this file outside the H5PB package!"
#endif

#ifndef _H5PBpkg_H
#define _H5PBpkg_H


/* Get package's private header */
#include "H5PBprivate.h"

struct H5PB_entry_t
{
    void                    *page_buf_ptr;          /* pointer to the buffer containing the data */
    H5PB_t                  *page_buf;              /* pointer to the page buffer struct */
    haddr_t		     addr;                  /* address of the page in the file */
    H5F_mem_page_t           type;                  /* type of the page entry (H5F_MEM_PAGE_RAW/META) */
    hbool_t		     is_dirty;              /* bool indicating whether the page has dirty data or not */

    /* fields supporting replacement policies: */
    struct H5PB_entry_t     *next;                  /* next pointer in the LRU list */
    struct H5PB_entry_t     *prev;                  /* previous pointer in the LRU list */
};

#endif /* _H5PBpkg_H */
