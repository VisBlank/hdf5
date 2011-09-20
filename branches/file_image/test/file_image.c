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

/***********************************************************
*
* Test program:  file_image
*
* kajkfjsd,lfkjskdlfjds//
*
*************************************************************/

#include "h5test.h"
#include "H5srcdir.h"
#include "H5Fprivate.h" /* required to test property removals */
#define VERIFY(condition, string) do { if (!(condition)) FAIL_PUTS_ERROR(string) } while(0)

/* Values for callback bit field */
#define MALLOC      0x01
#define MEMCPY      0x02
#define REALLOC     0x04
#define FREE        0x08
#define UDATA_COPY  0x10
#define UDATA_FREE  0x20

#define RANK 2
#define DIM0 1024
#define DIM1 32
#define DSET_NAME "test_dset"

const char *FILENAME[] = {
    "file_image_core_test",
    NULL
};


typedef struct {
    unsigned char used_callbacks;   /* Bitfield for tracking callbacks */
    H5_file_image_op_t malloc_src;  /* Source of file image callbacks */
    H5_file_image_op_t memcpy_src;
    H5_file_image_op_t realloc_src;
    H5_file_image_op_t free_src;
} udata_t;


/******************************************************************************
 * Function:    test_properties
 *
 * Purpose:     Tests that the file image properties (buffer pointer and length)
 *              are set properly. Image callbacks are not set in this test.
 *
 * Returns:     Success: 0
 *              Failure: 1
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static int
test_properties(void)
{
    hid_t   fapl_1;
    hid_t   fapl_2;
    hbool_t verbose = FALSE;
    char    *buffer;
    int     count = 10; 
    void    *temp;
    char    *temp2;
    int     i;   
    size_t  size;
    size_t  temp_size;

    if(verbose) HDfprintf(stdout, "entering test_properties()\n");
    
    /* Initialize file image buffer
     *
     * Note: this image will not contain a valid HDF5 file, as it complicates testing
     * property list functions. In the file driver tests further down, this will
     * not be the case.
     */
    size = (size_t)count * sizeof(char);
    buffer = (char *)malloc(size);
    for (i = 0; i < count-1; i++) buffer[i] = (char)(65+i);  buffer[count] = '\0';

    /* Create fapl */
    fapl_1 = H5Pcreate(H5P_FILE_ACCESS);

    /* Get file image stuff */
    H5Pget_file_image(fapl_1, (void **) &temp, &temp_size);

    /* Check default values */
    VERIFY(temp == NULL, "Default pointer is wrong");
    VERIFY(temp_size == 0, "Default size is wrong");

    /* Set file image stuff */
    H5Pset_file_image(fapl_1, (void *)buffer, size);
    
    /* Get the same */
    H5Pget_file_image(fapl_1, (void **) &temp, &temp_size);

    /* Check that sizes are the same, and that the buffers are identical but separate */
    VERIFY(temp != NULL,"temp is null!");
    VERIFY(temp_size == size,"Sizes of buffers don't match");
    VERIFY(temp != buffer, "Retrieved buffer is the same as original");
    VERIFY(0 == memcmp(temp, buffer, size),"Buffers contain different data");

    /* Copy the fapl */
    fapl_2 = H5Pcopy(fapl_1);

    /* Get values from the new fapl */
    H5Pget_file_image(fapl_2, (void **) &temp2, &temp_size);
    
    /* Check that sizes are the same, and that the buffers are identical but separate */
    VERIFY(temp_size == size,"Sizes of buffers don't match"); 
    VERIFY(temp2 != NULL,"Recieved buffer not set");
    VERIFY(temp2 != buffer, "Retrieved buffer is the same as original");
    VERIFY(temp2 != temp, "Retrieved buffer is the same as previously retrieved buffer");
    VERIFY(0 == memcmp(temp2, buffer, size),"Buffers contain different data");

    /* Close everything */
    H5Pclose(fapl_1);
    H5Pclose(fapl_2);
    free(buffer);
    free(temp);
    free(temp2);

    if(verbose) HDfprintf(stdout, "exiting test_properties()\n");

    return 0;

error:
    return 1; 
}

/******************************************************************************
 * Function:    malloc_cb
 *
 * Purpose:     This function allows calls to the malloc callback to be tracked.
 *
 * Returns:     The result of a standard malloc
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static void *
malloc_cb(size_t size, H5_file_image_op_t op, void *udata)
{
    udata_t *u = (udata_t *)udata;
    u->used_callbacks |= MALLOC;
    u->malloc_src = op;
    return malloc(size);
}

/******************************************************************************
 * Function:    memcpy_cb
 *
 * Purpose:     This function allows calls to the memcpy callback to be tracked.
 *
 * Returns:     The result of a standard memcpy
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static void *
memcpy_cb(void *dest, const void *src, size_t size, H5_file_image_op_t op, void *udata)
{
    udata_t *u = (udata_t *)udata;
    u->used_callbacks |= MEMCPY;
    u->memcpy_src = op;
    return memcpy(dest, src, size);
}

/******************************************************************************
 * Function:    realloc_cb
 *
 * Purpose:     This function allows calls to the realloc callback to be tracked.
 *
 * Returns:     The result of a standard realloc
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static void *
realloc_cb(void *ptr, size_t size, H5_file_image_op_t op, void *udata)
{
    udata_t *u = (udata_t *)udata;
    u->used_callbacks |= REALLOC;
    u->realloc_src = op;
    return realloc(ptr,size);
}

/******************************************************************************
 * Function:    free_cb
 *
 * Purpose:     This function allows calls to the free callback to be tracked.
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static void
free_cb(void *ptr, H5_file_image_op_t op, void *udata)
{
    udata_t *u = (udata_t *)udata;
    u->used_callbacks |= FREE;
    u->free_src = op;
    free(ptr);
    return;
}

/******************************************************************************
 * Function:    udata_copy_cb
 *
 * Purpose:     This function allows calls to the udata_copy callback to be tracked.
 *              No copying actualy takes place; it is easier to deal with only one
 *              instance of the udata.
 *
 * Returns:     A pointer to the same udata that was passed in.
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static void *
udata_copy_cb(void *udata)
{
    udata_t *u = (udata_t *)udata;
    u->used_callbacks |= UDATA_COPY;
    return udata;
}

/******************************************************************************
 * Function:    udata_free_cb
 *
 * Purpose:     This function allows calls to the udata_free callback to be tracked.
 *
 *              Note: this callback doesn't actually do anything. Since the
 *              udata_copy callback doesn't copy, only one instance of the udata
 *              is kept alive and such it must be freed explicitly at the end of the tests.
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static void
udata_free_cb(void *udata)
{
    udata_t *u = (udata_t *)udata;
    u->used_callbacks |= UDATA_FREE;
    return;
}

/******************************************************************************
 * Function:    reset_udata
 *
 * Purpose:     Resets the udata to default values. This facilitates storing only
 *              the results of a single operation in the udata.
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static void
reset_udata(udata_t *u)
{
    u->used_callbacks = 0;
    u->malloc_src = u->memcpy_src = u->realloc_src = u->free_src = H5_FILE_IMAGE_OP_NO_OP;
}

/******************************************************************************
 * Function:    test_callbacks
 *
 * Purpose:     Tests that callbacks are called properly in property list functions.
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static int
test_callbacks(void)
{
    hid_t fapl_1;
    hid_t fapl_2;
    hbool_t verbose = FALSE;
    void *(*image_malloc)(size_t, H5_file_image_op_t, void *);
    void *(*image_memcpy)(void *, const void *, size_t, H5_file_image_op_t, void *);
    void *(*image_realloc)(void *, size_t, H5_file_image_op_t, void *);
    void  (*image_free)(void *, H5_file_image_op_t, void *);
    void *(*udata_copy)(void *);
    void  (*udata_free)(void *);
    udata_t *udata;
    void *temp_udata;
    char *file_image;
    char *temp_file_image;
    int   count = 10;
    int   i;
    size_t size;
    size_t temp_size;

    if(verbose) HDfprintf(stdout, "entering test_callbacks()\n");

    /* Allocate and initialize udata */
    udata = (udata_t *)malloc(sizeof(udata_t));
    reset_udata(udata);

    /* Allocate and initialize file image buffer */
    size = (size_t)count * sizeof(char);
    file_image = (char *)malloc(size);
    for (i = 0; i < count-1; i++) file_image[i] = (char)(65+i);
    file_image[count] = '\0';

    /* Create fapl */
    fapl_1 = H5Pcreate(H5P_FILE_ACCESS);

    /* Get file image stuff */
    H5Pget_file_image_callbacks(fapl_1, &image_malloc, &image_memcpy, &image_realloc, &image_free, &udata_copy, &udata_free, (void **)&temp_udata);

    /* Check default values */
    VERIFY(image_malloc == NULL, "Default malloc callback is wrong");
    VERIFY(image_memcpy == NULL, "Default memcpy callback is wrong");
    VERIFY(image_realloc == NULL, "Default realloc callback is wrong");
    VERIFY(image_free == NULL, "Default free callback is wrong");
    VERIFY(udata_copy == NULL, "Default udata copy callback is wrong");
    VERIFY(udata_free == NULL, "Default udata free callback is wrong");
    VERIFY(temp_udata == NULL, "Default udata is wrong");

    /* Set file image callbacks */
    H5Pset_file_image_callbacks(fapl_1, &malloc_cb, &memcpy_cb, &realloc_cb, &free_cb, &udata_copy_cb, &udata_free_cb, (void *)udata);

    /* Get file image callbacks */
    H5Pget_file_image_callbacks(fapl_1, &image_malloc, &image_memcpy, &image_realloc, &image_free, &udata_copy, &udata_free, &temp_udata);
    
    /* Verify values */
    VERIFY(image_malloc == &malloc_cb, "malloc callback was not set or retrieved properly");   
    VERIFY(image_memcpy == &memcpy_cb, "memcpy callback was not set or retrieved properly");
    VERIFY(image_realloc == &realloc_cb, "realloc callback was not set or retrieved properly");
    VERIFY(image_free == &free_cb, "free callback was not set or retrieved properly");
    VERIFY(udata_copy == &udata_copy_cb, "udata copy callback was not set or retrieved properly");
    VERIFY(udata_free == &udata_free_cb, "udata free callback was not set or retrieved properly");
    VERIFY(temp_udata == udata, "udata was not set or retrieved properly");
    
    /*
     * Check callbacks in internal function without a previously set file image
     */

    /* Copy fapl */
    reset_udata(udata);
    fapl_2 = H5Pcopy(fapl_1);
    
    /* Verify that the property's copy callback used the correct image callbacks */
    VERIFY(udata->used_callbacks == (UDATA_COPY), "Copying a fapl with no image used incorrect callbacks");

    /* Close fapl */
    reset_udata(udata);
    H5Pclose(fapl_2);

    /* Verify that the udata free callback was used */
    VERIFY(udata->used_callbacks == (UDATA_FREE), "Closing a fapl with no image used incorrect callbacks");

    /* Copy again */
    fapl_2 = H5Pcopy(fapl_1);
    
    /* Remove property from fapl */
    reset_udata(udata);
    H5Premove(fapl_2, H5F_ACS_FILE_IMAGE_INFO_NAME); 

    /* Verify that the property's delete callback was called using the correct image callbacks */
    VERIFY(udata->used_callbacks == (UDATA_FREE), "Removing a property from a fapl with no image used incorrect callbacks");
    
    /* Close it again */
    H5Pclose(fapl_2);

    /* Get file image */ //unsafe looking. probably should fix.
    reset_udata(udata);
    H5Pget_file_image(fapl_1, (void **)&temp_file_image, &temp_size);

    /* Verify that the correct callbacks were used */
    VERIFY(udata->used_callbacks == 0, "attempting to retrieve the image from a fapl without an image has an unexpected callback");

    /* Set file image */
    reset_udata(udata);
    H5Pset_file_image(fapl_1, (void *)file_image, size);

    VERIFY(udata->used_callbacks == (MALLOC | MEMCPY), "Setting a file image (first time) used incorrect callbacks");
    
    /*
     * Check callbacks in internal functions with a previously set file image
     */
    
    /* Copy fapl */
    reset_udata(udata);
    fapl_2 = H5Pcopy(fapl_1);
    
    /* Verify that the property's copy callback used the correct image callbacks */
    VERIFY(udata->used_callbacks == (MALLOC | MEMCPY | UDATA_COPY), "Copying a fapl with an image used incorrect callbacks");
    VERIFY(udata->malloc_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_COPY, "malloc callback has wrong source");
    VERIFY(udata->memcpy_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_COPY, "memcpy callback has wrong source");

    /* Close fapl */
    reset_udata(udata);
    H5Pclose(fapl_2);

    /* Verify that the udata free callback was used */
    VERIFY(udata->used_callbacks == (FREE|UDATA_FREE), "Closing a fapl with an image used incorrect callbacks");
    VERIFY(udata->free_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_CLOSE, "free callback has wrong source");

    /* Copy again */
    fapl_2 = H5Pcopy(fapl_1);
    
    /* Remove property from fapl */
    reset_udata(udata);
    H5Premove(fapl_2, H5F_ACS_FILE_IMAGE_INFO_NAME); 

    /* Verify that the property's delete callback was called using the correct image callbacks */
    VERIFY(udata->used_callbacks == (FREE|UDATA_FREE), "Removing a property from a fapl with an image used incorrect callbacks");
    VERIFY(udata->free_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_CLOSE, "free callback has wrong source");
    
    /* Close it again */
    H5Pclose(fapl_2);

    /* Get file image */ 
    reset_udata(udata);
    H5Pget_file_image(fapl_1, (void **)&temp_file_image, &temp_size);

    /* Verify that the correct callbacks were used */
    VERIFY(udata->used_callbacks == (MALLOC|MEMCPY), "attempting to retrieve the image from a fapl with an image has an unexpected callback");
    VERIFY(udata->malloc_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_GET, "malloc callback has wrong source");
    VERIFY(udata->memcpy_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_GET, "memcpy callback has wrong source");

    /* Set file image */
    reset_udata(udata);
    H5Pset_file_image(fapl_1, (void *)file_image, size);

    VERIFY(udata->used_callbacks == (FREE | MALLOC | MEMCPY), "Setting a file image (second time) used incorrect callbacks");
    VERIFY(udata->malloc_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_SET, "malloc callback has wrong source");
    VERIFY(udata->memcpy_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_SET, "memcpy callback has wrong source");
    VERIFY(udata->free_src == H5_FILE_IMAGE_OP_PROPERTY_LIST_SET, "freec callback has wrong source");

    /* Close stuff */
    H5Pclose(fapl_1);
    free(file_image);
    free(temp_file_image);
    free(udata);

    if(verbose) HDfprintf(stdout, "regular exit test_callbacks()\n");

    return 0;

error:
    if(verbose) HDfprintf(stdout, "error exit test_callbacks()\n");
    return 1;
}

/******************************************************************************
 * Function:    test_core
 *
 * Purpose:     Tests that callbacks are called properly in the core VFD and
 *              that the initial file image works properly.
 *
 * Programmer:  Jacob Gruber
 *              Monday, August 22, 2011
 *
 ******************************************************************************
 */
static int
test_core(void)
{
    hid_t   fapl;
    hid_t   file;
    hid_t   dset;
    hid_t   space;
    hbool_t verbose = FALSE;
    udata_t *udata;
    unsigned char *file_image;
    char    filename[1024];
    char    src_dir_filename[1024];
    const char *tmp = NULL;
    size_t  size;
    hsize_t dims[2];
    int     fd;
    struct stat  sb;
    herr_t ret;

    if(verbose) HDfprintf(stdout, "entering test_core()\n");

    /* Create fapl */
    fapl = h5_fileaccess();
    VERIFY(fapl >= 0, "fapl creation failed");

    /* Set up the core VFD */
    ret = H5Pset_fapl_core(fapl, 0, 0);
    VERIFY(ret >= 0, "setting core driver in fapl failed");

    if ( verbose )
        HDfprintf(stdout, "FILENAME[0] = \"%s\".\n", FILENAME[0]);
    tmp = h5_fixname(FILENAME[0], fapl, filename, sizeof(filename));
    VERIFY(tmp != NULL, "h5_fixname failed");
    if ( verbose ) 
	 HDfprintf(stdout, "filename = \"%s\".\n", tmp);

    /* convert file name to srcdir file name.  Make a copy as 
     * H5_get_srcdir_filename() simply sets up the file name in its
     * own buffer each time it is called -- overwriting the previous
     * value.
     */
    tmp = H5_get_srcdir_filename(filename);
    VERIFY(tmp != NULL, "H5_get_srcdir_filename failed");
    VERIFY(strlen(tmp) < 1023, "srcdir file name too long.");
    HDstrncpy(src_dir_filename, tmp, 1023);
    src_dir_filename[1023] = '\0';
    if ( verbose )
        HDfprintf(stdout, "src_dir_filename = \"%s\".\n", src_dir_filename);

    /* Allocate and initialize udata */
    udata = (udata_t *)malloc(sizeof(udata_t));
    VERIFY(udata != NULL, "udata malloc failed");

    /* Set file image callbacks */
    ret = H5Pset_file_image_callbacks(fapl, &malloc_cb, &memcpy_cb, &realloc_cb, &free_cb, &udata_copy_cb, &udata_free_cb, (void *)udata);
    VERIFY(ret >= 0, "set image callbacks failed");

    /* Test open (no file image) */
    reset_udata(udata);
    file = H5Fopen(src_dir_filename, H5F_ACC_RDWR, fapl);
    VERIFY(file >= 0, "H5Fopen failed");
    VERIFY(udata->used_callbacks == MALLOC, "opening a core file used the wrong callbacks");
    VERIFY(udata->malloc_src == H5_FILE_IMAGE_OP_FILE_OPEN, "Malloc callback came from wrong sourc in core open");

    /* Close file
     * Note: closing the file without writing to it generates a realloc callback (through truncate)
     *       while closing after writing does not. This is because a write forces the eof
     *       to be a multiple of the core increment, while an open does not. H5FD_core_truncate,
     *       forces the eof to be a multiple of the increment, and thus must realloc if no write
     *       was performed (assuming the file size is not a multiple of the increment).
     */
    reset_udata(udata);
    ret = H5Fclose(file);
    VERIFY(ret >= 0, "H5Fclose failed");
    VERIFY(udata->used_callbacks == (FREE|REALLOC), "Closing a core file used the wrong callbacks");
    VERIFY(udata->free_src == H5_FILE_IMAGE_OP_FILE_CLOSE, "Free callback came from wrong sourc in core close");
    VERIFY(udata->realloc_src == H5_FILE_IMAGE_OP_FILE_RESIZE, "Realloc callback came from wrong sourc in core close");

    /* Reopen file */
    file = H5Fopen(src_dir_filename, H5F_ACC_RDWR, fapl);
    VERIFY(file >= 0, "H5Fopen failed");

    /* Set up a new dset */
    dims[0] = DIM0;
    dims[1] = DIM1;
    space = H5Screate_simple(RANK, dims, dims);
    VERIFY(space >= 0, "H5Screate failed");
    
    /* Create new dset, invoking H5FD_core_write */
    reset_udata(udata);
    dset = H5Dcreate(file, DSET_NAME, H5T_NATIVE_INT, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    VERIFY(dset >=0, "H5Dcreate failed");
    
    /* Flush the write and check the realloc callback */
    ret = H5Fflush(file, H5F_SCOPE_LOCAL);
    VERIFY(ret >= 0, "H5Fflush failed");
    VERIFY(udata->used_callbacks == (REALLOC), "core write used the wrong callbacks");
    VERIFY(udata->realloc_src == H5_FILE_IMAGE_OP_FILE_RESIZE, "Realloc callback came from wrong source in core write");
    
    /* Close dset and space */
    ret = H5Dclose(dset);
    VERIFY(ret >= 0, "H5Dclose failed");
    ret = H5Sclose(space);
    VERIFY(ret >= 0, "H5Sclose failed");
    
    /* Test file close */
    reset_udata(udata);
    ret = H5Fclose(file);
    VERIFY(ret >= 0, "H5Fclose failed");
    VERIFY(udata->used_callbacks == (FREE), "Closing a core file used the wrong callbacks");
    VERIFY(udata->free_src == H5_FILE_IMAGE_OP_FILE_CLOSE, "Free callback came from wrong sourc in core close");

    /* Create file image buffer */
    fd = open(src_dir_filename,O_RDONLY);
    VERIFY(fd > 0, "open failed");
    ret = fstat(fd,&sb);
    VERIFY(ret == 0, "fstat failed");
    size = (size_t)sb.st_size;
    file_image = (unsigned char *)malloc(size);
    read(fd, file_image, size);

    /* Set file image in plist */
    H5Pset_file_image(fapl, file_image, size);

    /* Test open with file image */
    file = H5Fopen("dne.h5", H5F_ACC_RDWR, fapl);
    H5Fclose(file);

    /* Release resources */
    h5_cleanup(FILENAME, fapl); 
    free(udata);
    free(file_image);
    
    PASSED();

    if(verbose) HDfprintf(stdout, "regular exit test_core()\n");

    return 0;

error:

    if(verbose) HDfprintf(stdout, "error exit test_core()\n");

    return 1;
}

int
main(void)
{
    int errors = 0;

    h5_reset();

    printf("Testing File Image Functionality.\n");

    errors += test_properties();
    errors += test_callbacks();
    errors += test_core();

    if(errors) { 
        printf("***** %d File Image TEST%s FAILED! *****\n", 
            errors, errors > 1 ? "S" : ""); 
        return 1; 
    }

    printf("All File Image tests passed.\n");
    return 0;
}
