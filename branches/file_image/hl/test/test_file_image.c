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

#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "h5hltest.h"
#include "H5srcdir.h"
#include "H5LTpublic.h"

#define FILE_NAME1 "file_img1.h5"
#define FILE_NAME2 "file_img2.h5"
#define DSET_NAME "dset"

#define RANK 2
 
/*-------------------------------------------------------------------------
* test file image operations 
*-------------------------------------------------------------------------
*/


/* load file content and size into buffer and variable using standard UNIX I/O */
static int test_load_image(char *filename, void **buffer_ptr, hsize_t *size)
{
    int fd;
    struct stat sb;

    if ((fd = open(filename, O_RDONLY)) < 0)
        goto out;

    if (fstat(fd, &sb) < 0)
        goto out;

    *size = (size_t)sb.st_size;

    if ((*buffer_ptr = malloc(*size)) == NULL)
        goto out;

    if (read(fd, *buffer_ptr, *size) < 0)
        goto out;

    if (close(fd) < 0)
        goto out;
   
    return 0;

out:

    return -1;
} 


static int test_file_image( void )
{
    hid_t       file_id1, fapl1, file_id2, fapl2, file_id3, file_id4;
    hsize_t     dims1[RANK]={2,3};
    hsize_t     dims2[RANK]={2,2};
    int         data1[6]={1,2,3,4,5,6};
    int         data2[4]={7,8,9,10};
    hsize_t     dims3[RANK];
    hsize_t     dims4[RANK];
    int         data3[6];
    int         data4[4];
    herr_t      status;
    hsize_t     size1, size2;
    void        *buffer_ptr1;
    void        *buffer_ptr2;
    unsigned    flags;
    size_t      i, j, nrow, n_values;

    TESTING("file_image_operations");

    /* create a HDF5 file */
    if ((file_id1 = H5Fcreate (FILE_NAME1, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))<0)
        goto out;

    /* create a HDF5 file */
    if ((file_id2 = H5Fcreate (FILE_NAME2, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))<0)
        goto out;

    /* create and write an integer type dataset named "dset" */
    if (H5LTmake_dataset(file_id1, DSET_NAME,RANK, dims1, H5T_NATIVE_INT, data1) < 0)
        goto out;
    if (H5LTmake_dataset(file_id2, DSET_NAME,RANK, dims2, H5T_NATIVE_INT, data2) < 0)
        goto out;

    /* flush into the file */
    if (H5Fflush(file_id1, H5F_SCOPE_LOCAL) < 0)
        goto out;

    if (H5Fclose (file_id1) < 0)
        goto out;

    /* load file contents and size into buffer and size */
    if (test_load_image(FILE_NAME1, &buffer_ptr1, &size1) < 0)
        goto out;

    /* flush into the file */
    if (H5Fflush(file_id2, H5F_SCOPE_LOCAL) < 0)
        goto out;

    if (H5Fclose (file_id2) < 0)
        goto out;

    /* load file contents and size into buffer and size */
    if (test_load_image(FILE_NAME2, &buffer_ptr2, &size2) < 0)
        goto out;

    /* set file image in the core driver */ 
    if ((file_id3 = H5LTopen_file_image(buffer_ptr1, size1, H5LT_FILE_IMAGE_DONT_COPY|H5LT_FILE_IMAGE_DONT_RELEASE)) < 0)
        goto out;

    /* set file image in the core driver */ 
    if ((file_id4 = H5LTopen_file_image(buffer_ptr2, size2, H5LT_FILE_IMAGE_DONT_COPY)) < 0)
        goto out;

    if (H5LTread_dataset_int(file_id3, DSET_NAME, data3) < 0)
        goto out;

    if (H5LTget_dataset_info(file_id3, DSET_NAME, dims3, NULL, NULL) < 0)
        goto out;

    if (H5LTread_dataset_int(file_id4, DSET_NAME, data4) < 0)
        goto out;

    if (H5LTget_dataset_info(file_id4, DSET_NAME, dims4, NULL, NULL) < 0)
        goto out;

    n_values = (size_t)(dims3[0] * dims3[1]);
    nrow = (size_t)dims3[1];

    /* compare file image values with original data */
    for (i = 0; i < n_values / nrow; i++){ 
        for (j = 0; j < nrow; j++)
            if (data3[i * nrow + j] != data1[i * nrow + j])
                goto out; 
    } /* end for */

    n_values = (size_t)(dims4[0] * dims4[1]);
    nrow = (size_t)dims4[1];

    /* compare file image values with original data */
    for (i = 0; i < n_values / nrow; i++){ 
        for (j = 0; j < nrow; j++)
            if (data4[i * nrow + j] != data2[i * nrow + j])
                goto out; 
    } /* end for */

    H5Fflush(file_id3, H5F_SCOPE_LOCAL);
    H5Fflush(file_id4, H5F_SCOPE_LOCAL);

    H5Fclose(file_id3);
    H5Fclose(file_id4);

    /* only release buffer that was not release by library */
    free(buffer_ptr1);

    PASSED();

    return 0;

out:

    H5_FAILED();
    return -1;
}

/*-------------------------------------------------------------------------
* the main program
*-------------------------------------------------------------------------
*/
int main( void )
{
    int  nerrors=0;

    /* test image */
    nerrors += test_file_image();

    /* check for errors */
    if (nerrors)
        goto error;

    return 0;

error:
    return 1;


}

