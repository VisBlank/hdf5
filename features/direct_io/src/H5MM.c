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
 * Created:		H5MM.c
 *			Jul 10 1997
 *			Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:		Memory management functions.
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */


#include "H5private.h"
#include "H5Eprivate.h"
#include "H5MMprivate.h"

#ifndef NDEBUG

/*-------------------------------------------------------------------------
 * Function:	H5MM_malloc
 *
 * Purpose:	Just like the POSIX version of malloc(3). This routine
 *		specifically checks for allocations of 0 bytes and fails
 *              in that case.  This routine is not called when NDEBUG is
 *		defined.
 *
 * Return:	Success:	Ptr to new memory
 *
 *		Failure:	NULL
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Nov  8 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void *
H5MM_malloc(size_t size)
{
    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5MM_malloc);

    assert(size);

    FUNC_LEAVE_NOAPI(HDmalloc(size));
} /* end H5MM_malloc() */


/*-------------------------------------------------------------------------
 * Function:	H5MM_calloc
 *
 * Purpose:	Similar to the POSIX version of calloc(3), except this routine
 *              just takes a 'size' parameter. This routine
 *		specifically checks for allocations of 0 bytes and fails
 *              in that case.  This routine is not called when NDEBUG is
 *		defined.
 *
 * Return:	Success:	Ptr to new memory
 *
 *		Failure:	NULL
 *
 * Programmer:	Quincey Koziol
 *		koziol@ncsa.uiuc.edu
 *		Nov  8 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void *
H5MM_calloc(size_t size)
{
    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5MM_calloc);

    assert(size);

    FUNC_LEAVE_NOAPI(HDcalloc(1,size));
} /* end H5MM_calloc() */
#endif /* NDEBUG */


/*-------------------------------------------------------------------------
 * Function:	H5MM_aligned_malloc
 *
 * Purpose:	Just like the POSIX version of malloc(3). This routine
 *		specifically checks for allocations of 0 bytes and fails
 *              in that case.  This routine is not called when NDEBUG is
 *		defined.
 *
 * Return:	Success:	The actual size of the allocated buffer 
 *
 *		Failure:	Negative
 *
 * Programmer:	Raymond Lu
 *		songyulu@hdfgroup.org
 *		4 August 2009
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
ssize_t
H5MM_aligned_malloc(void** buffer, size_t size, hbool_t initialize, 
                    H5FD_direct_fapl_t *align_info)
{
    size_t  alloc_size;
    size_t  fbsize, cbsize, boundary;
    char    error_msg[1024];
    ssize_t ret_value = SUCCEED;
 
    FUNC_ENTER_NOAPI(H5MM_aligned_malloc, FAIL);

    assert(size);

#ifdef H5_HAVE_DIRECT
    if(align_info && align_info->must_align) {
        fbsize = align_info->fbsize;
        cbsize = align_info->cbsize;
        boundary = align_info->mboundary;

        /* Make sure the size is a multiple of file block size.  It can be bigger than
         * the copy buffer size because the conversion may require a bigger size (see
         * the H5D_typeinfo_init in H5Dio.c) */
        if(size % fbsize > 0)
            alloc_size = ((size / fbsize) * fbsize) + fbsize;
        else
            alloc_size = size;

        if (HDposix_memalign(buffer, boundary, alloc_size) != 0)
            HGOTO_ERROR(H5E_RESOURCE, H5E_CANTALLOC, FAIL, "HDposix_memalign failed")
    
        if(initialize)
            HDmemset(*buffer, 0, alloc_size);
 
        ret_value = (ssize_t)alloc_size;
        HGOTO_DONE(ret_value);        
    }
#endif

    /* If Direct IO isn't used or it doesn't require alignment, 
     * simply allocate a buffer according to the actual size */
    if(initialize)
        *buffer = HDcalloc(1, size);
    else
        *buffer = HDmalloc(size); 

    ret_value = (ssize_t)size;

done:
    FUNC_LEAVE_NOAPI(ret_value);
} /* end H5MM_aligned_malloc() */


/*-------------------------------------------------------------------------
 * Function:	H5MM_realloc
 *
 * Purpose:	Just like the POSIX version of realloc(3). Specifically, the
 *		following calls are equivalent
 *
 *		H5MM_realloc (NULL, size) <==> H5MM_malloc (size)
 *		H5MM_realloc (ptr, 0)	  <==> H5MM_xfree (ptr)
 *		H5MM_realloc (NULL, 0)	  <==> NULL
 *
 * Return:	Success:	Ptr to new memory or NULL if the memory
 *				was freed.
 *
 *		Failure:	abort()
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul 10 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void *
H5MM_realloc(void *mem, size_t size)
{
    void *ret_value;

    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5MM_realloc);

    if (!mem) {
	if (0 == size)
            HGOTO_DONE(NULL);
	mem = H5MM_malloc(size);

    } else if (0 == size) {
	mem = H5MM_xfree(mem);

    } else {
	mem = HDrealloc(mem, size);
	assert(mem);
    }

    /* Set return value */
    ret_value=mem;

done:
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:	H5MM_xstrdup
 *
 * Purpose:	Duplicates a string.  If the string to be duplicated is the
 *		null pointer, then return null.	 If the string to be duplicated
 *		is the empty string then return a new empty string.
 *
 * Return:	Success:	Ptr to a new string (or null if no string).
 *
 *		Failure:	abort()
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul 10 1997
 *
 *-------------------------------------------------------------------------
 */
char *
H5MM_xstrdup(const char *s)
{
    char	*ret_value = NULL;

    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5MM_xstrdup)

    if(s) {
        ret_value = (char *)H5MM_malloc(HDstrlen(s) + 1);
        HDassert(ret_value);
        HDstrcpy(ret_value, s);
    } /* end if */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MM_xstrdup() */


/*-------------------------------------------------------------------------
 * Function:	H5MM_strdup
 *
 * Purpose:	Duplicates a string.  If the string to be duplicated is the
 *		null pointer, then return null.	 If the string to be duplicated
 *		is the empty string then return a new empty string.
 *
 * Return:	Success:	Ptr to a new string (or null if no string).
 *
 *		Failure:	abort()
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul 10 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
char *
H5MM_strdup(const char *s)
{
    char	*ret_value;

    FUNC_ENTER_NOAPI(H5MM_strdup, NULL)

    if(!s)
	HGOTO_ERROR(H5E_ARGS, H5E_BADVALUE, NULL, "null string")
    if(NULL == (ret_value = (char *)H5MM_malloc(HDstrlen(s) + 1)))
	HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")
    HDstrcpy(ret_value, s);

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5MM_strdup() */


/*-------------------------------------------------------------------------
 * Function:	H5MM_xfree
 *
 * Purpose:	Just like free(3) except null pointers are allowed as
 *		arguments, and the return value (always NULL) can be
 *		assigned to the pointer whose memory was just freed:
 *
 *			thing = H5MM_xfree (thing);
 *
 * Return:	Success:	NULL
 *
 *		Failure:	never fails
 *
 * Programmer:	Robb Matzke
 *		matzke@llnl.gov
 *		Jul 10 1997
 *
 *-------------------------------------------------------------------------
 */
void *
H5MM_xfree(void *mem)
{
    /* Use FUNC_ENTER_NOAPI_NOINIT_NOFUNC here to avoid performance issues */
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5MM_xfree);

    if(mem)
        HDfree(mem);

    FUNC_LEAVE_NOAPI(NULL);
} /* end H5MM_xfree() */
