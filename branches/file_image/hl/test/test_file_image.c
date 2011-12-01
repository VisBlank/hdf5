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

#define DSET_NAME "dset"

#define RANK 2
 
/*-------------------------------------------------------------------------
* test file image operations 
*-------------------------------------------------------------------------
*/


static int test_file_image(size_t open_images, unsigned *flags, size_t nflags)
{
    hid_t       *file_id, *dset_id, file_space; /* HDF5 ids */
    hsize_t     dims1[RANK] = {2,3}; /* dimension of dataset */
    int         data1[6] = {1,2,3,4,5,6}; /* contents of dataset */
    int         data2[6] = {7,8,9,10,11,12}; /* "wrong" contents of dataset */
    hsize_t     dims3[RANK]; /* array to read dataset dimensions */
    int         data3[6]; /* array to read dataset contents */
    ssize_t     *buf_size; /* pointer to array of buffer sizes */
    void        **buf_ptr; /* pointer to array of pointers to image buffers */
    unsigned    *input_flags; /* pointer to array of flag combinations */
    char        file_name[32]; /* array for file name */
    size_t      i, j, k, nrow, n_values;

    TESTING("file_image_operations");

    VERIFY(open_images > 1 , "The number of open images must be greater than 1");

    VERIFY(nflags > 0, "The number of flag combinations  must be greater than 0");

    /* allocate array of flags for open images */
    if ((input_flags = (unsigned*)malloc(sizeof(unsigned) * open_images)) == NULL)
        FAIL_PUTS_ERROR("malloc() failed");

    /* allocate array of pointers for each of the open images */
    if ((buf_ptr = (void**)malloc(sizeof(void*) * open_images)) == NULL)
        FAIL_PUTS_ERROR("malloc() failed");

    /* allocate array to store the size of each open images */
    if ((buf_size = (ssize_t*)malloc(sizeof(ssize_t) * open_images)) == NULL)
        FAIL_PUTS_ERROR("malloc() failed");

    /* allocate array for each of the file identifiers */
    if ((file_id = (hid_t*)malloc(sizeof(hid_t) * open_images)) == NULL)
        FAIL_PUTS_ERROR("malloc() failed");

    /* allocate array for each of the dataset identifiers */
    if ((dset_id = (hid_t*)malloc(sizeof(hid_t) * open_images)) == NULL)
        FAIL_PUTS_ERROR("malloc() failed");

    /* create several file images */
    for (i = 0; i < open_images; i++) {

        /* populate array for flags combinations */
        input_flags[i] = flags[(nflags+i) % nflags];

        /* create file name */
        sprintf(file_name, "image_file%d.h5", i);

        /* create file */
        if ((file_id[i] = H5Fcreate (file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))<0)
            FAIL_PUTS_ERROR("H5Fcreate() failed");

        /* define dataspace for the dataset */ 
        file_space = H5Screate_simple(RANK, dims1, NULL);

        /* create and write an integer type dataset named "dset" */
        if ((dset_id[i] = H5Dcreate(file_id[i], DSET_NAME, H5T_NATIVE_INT, file_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
            FAIL_PUTS_ERROR("H5Dcreate() failed");
     
        /* dataset in open image 1 is written with "wrong" data */        
        if (i == 1) {
            if (H5Dwrite(dset_id[i], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,  data2) < 0)
                FAIL_PUTS_ERROR("H5Dwrite() failed");
        } /* end if*/
        /* dataset in the rest of the open images is written with correct data */
        else {
            if (H5Dwrite(dset_id[i], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,  data1) < 0)
                FAIL_PUTS_ERROR("H5Dwrite() failed");
        } /* end else */
          
        /* flush into the file */
        if (H5Fflush(file_id[i], H5F_SCOPE_LOCAL) < 0)
            FAIL_PUTS_ERROR("H5Fflush() failed");

        /* close dataspace */
        if (H5Sclose (file_space) < 0)
            FAIL_PUTS_ERROR("H5Sclose() failed");

        /* close dataset */
        if (H5Dclose (dset_id[i]) < 0)
            FAIL_PUTS_ERROR("H5Dclose() failed");

        /* get size of the file image i */
        if ((buf_size[i] = H5Fget_file_image(file_id[i], NULL, 0)) < 0)
            FAIL_PUTS_ERROR("H5Fget_file_image() failed");

        /* allocate buffer for the file image i */
        if ((buf_ptr[i] = (void*)malloc(buf_size[i])) == NULL)
            FAIL_PUTS_ERROR("malloc() failed");

        /* buffer for file image 2 is filled with random data */
        if (i == 2) {
            for (j = 0; j < buf_size[i]; j++)
                ((char*)buf_ptr[i])[j] = rand(); 
        } /* end if */
        /* buffers for the rest of the file images are filled with data from the respective files */
        else {
            if ((buf_size[i] = H5Fget_file_image(file_id[i], buf_ptr[i], buf_size[i])) < 0)
                FAIL_PUTS_ERROR("H5Fget_file_image() failed");
        } /* end else */

        /* file close */
        if (H5Fclose (file_id[i]) < 0)
            FAIL_PUTS_ERROR("H5Fclose() failed");
    } 

    /* open the file images with the core driver for data access */ 
    for (i = 0; i < open_images; i++) {
        /* open file image 2 filled with random data */
        if (i == 2) {

            H5E_BEGIN_TRY {
                /* attempt to set file image in the core driver */ 
                file_id[i] = H5LTopen_file_image(buf_ptr[i], buf_size[i], input_flags[i]);
            } H5E_END_TRY

            VERIFY(file_id[i] < 0, "H5LTopen_file_image() should have failed");
        } /* end if */
        /* open rest of file images */
        else {
            /* open file image with rest of flag combinations */
            /* set file image in the core driver */ 
            if ((file_id[i] = H5LTopen_file_image(buf_ptr[i], buf_size[i], input_flags[i])) < 0)
                FAIL_PUTS_ERROR("H5LTopen_file_image() failed");
        }
    }

    /* access the open file images and verify data */
    for (i = 0; i < open_images; i++) {

        /* if opening the file image failed, continue next iteration */
        if (file_id[i] < 0)
            continue;

        /* open dataset in file image */ 
        if ((dset_id[i] = H5Dopen(file_id[i], DSET_NAME, H5P_DEFAULT)) < 0)
            FAIL_PUTS_ERROR("H5Dopen() failed");

        /* get dataspace for the dataset */
        if ((file_space = H5Dget_space(dset_id[i])) < 0)
            FAIL_PUTS_ERROR("H5Dget_space() failed");

        /* get dimensions for the dataset */
        if (H5Sget_simple_extent_dims(file_space, dims3, NULL) < 0)
            FAIL_PUTS_ERROR("H5Sget_simple_extent_dims() failed");

        /* read dataset */
        if (H5Dread(dset_id[i], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,  data3) < 0)
            FAIL_PUTS_ERROR("H5Dread() failed");
        
        /* compute number of elements in dataset */  
        n_values = (size_t)(dims3[0] * dims3[1]);

        /* determine the number of rows in dataset */
        nrow = (size_t)dims3[1];

        /* verify contents for file image 1 with "wrong" data */
        if (i == 1) {
            /* compare file image values with original data */
            for (j = 0; j < n_values / nrow; j++){ 
                for (k = 0; k < nrow;k++)
                    if (data3[j * nrow + k ] == data1[j * nrow + k ])
                        FAIL_PUTS_ERROR("comparison of image values with original data should have failed");
            } 
        } /* end if */
        /* verify contents for the rest of the file images */
        else {
            /* compare file image values with original data */
            for (j = 0; j < n_values / nrow; j++){ 
                for (k = 0; k < nrow;k++)
                    if (data3[j * nrow + k ] != data1[j * nrow + k ])
                        FAIL_PUTS_ERROR("comparison of image values with original data failed");

            } /* end for */
        } /* end else */
         
        /* close dataspace */ 
        if (H5Sclose (file_space) < 0)
            FAIL_PUTS_ERROR("H5Sclose() failed");

        /* close dataset */
        if (H5Dclose(dset_id[i]) < 0)
            FAIL_PUTS_ERROR("H5Dclose() failed");
    }

    /* close file images  and release shared buffer if appropriate */
    for (i = 0; i < open_images; i++) {

        /* if opening the file image failed, continue next iteration */
        if (file_id[i] < 0)
            continue;

        /* close file image */
        if (H5Fclose(file_id[i]) < 0)
            FAIL_PUTS_ERROR("H5Fclose() failed");

        /* free shared buffer if appropriate */
        if ((input_flags[i] & H5LT_FILE_IMAGE_DONT_COPY) && (input_flags[i] & H5LT_FILE_IMAGE_DONT_RELEASE)) {
            VERIFY(buf_ptr[i] != NULL, "image must be non NULL");
            free(buf_ptr[i]);
        }
       
    }

    /* release temporary working buffers */
    free(file_id);
    free(dset_id);
    free(buf_ptr);
    free(buf_size);
    free(input_flags);

    PASSED();

    return 0;

error:

    return -1;
}

/*-------------------------------------------------------------------------
* the main program
*-------------------------------------------------------------------------
*/
int main( void )
{
    int       nerrors = 0;
    size_t    open_images = 10; /* number of open file images */
    size_t    nflags = 7; /* number of flag combinations */
    unsigned  flags[7]; /* array with flag combinations */
   
    /* set flag combinations for testing */ 
    flags[0] = 0;
    flags[1] = H5LT_FILE_IMAGE_DONT_RELEASE;
    flags[2] = H5LT_FILE_IMAGE_DONT_COPY;
    flags[3] = H5LT_FILE_IMAGE_DONT_COPY | H5LT_FILE_IMAGE_DONT_RELEASE;
    flags[4] = H5LT_FILE_IMAGE_OPEN_RW;
    flags[5] = H5LT_FILE_IMAGE_OPEN_RW | H5LT_FILE_IMAGE_DONT_RELEASE;
    flags[6] = H5LT_FILE_IMAGE_OPEN_RW | H5LT_FILE_IMAGE_DONT_COPY;
    //flags[7] = H5LT_FILE_IMAGE_OPEN_RW | H5LT_FILE_IMAGE_DONT_COPY | H5LT_FILE_IMAGE_DONT_RELEASE;

    /* Test file image operations. The flag combinations are assigned to file images in round-robin fashion */
    nerrors += test_file_image(open_images, flags, nflags) < 0? 1 : 0;

    if (nerrors) goto error;
    printf("File image tests passed.\n");
    return 0;

error:
    printf("***** %d IMAGE TEST%s FAILED! *****\n",nerrors, 1 == nerrors ? "" : "S");
    return 1;
}
