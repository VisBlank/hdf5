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

/*
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Thursday, March 12, 1998
 */

#include "hdf5.h"
#include "H5private.h"


#define RAW_FILE_NAME   "iopipe.raw"
#define HDF5_FILE_NAME  "iopipe.h5"

#define REQUEST_SIZE_X  4096
#define REQUEST_SIZE_Y  4096
#define NREAD_REQUESTS  45
#define NWRITE_REQUESTS 45



/*-------------------------------------------------------------------------
 * Function:    print_stats
 *
 * Purpose:     Prints statistics
 *
 * Return:      void
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 12, 1998
 *
 *-------------------------------------------------------------------------
 */
static void
print_stats(const char *prefix, H5_timer_t timer, size_t n_io_bytes)
{
    HDfprintf(stderr, "%-16s user: %T\tsystem: %T\telapsed: %T\t@ %B\n",
        prefix,
        timer.total.user,
        timer.total.system,
        timer.total.elapsed,
        n_io_bytes / timer.total.elapsed);

    return;
}


/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:
 *
 * Return:  Success:    0
 *
 *          Failure:    Abort
 *
 * Programmer:  Robb Matzke
 *              Thursday, March 12, 1998
 *
 *-------------------------------------------------------------------------
 */
int
main (void)
{
    static hsize_t  size[2] = {REQUEST_SIZE_X, REQUEST_SIZE_Y};
    static unsigned nread = NREAD_REQUESTS, nwrite = NWRITE_REQUESTS;

    unsigned char   *the_data = NULL;
    hid_t           file, dset, file_space = -1;
    herr_t          status;
    int             fd;
    unsigned        u;
    hssize_t        n;
    HDoff_t         offset;
    hsize_t         start[2];
    hsize_t         count[2];
    H5_timer_t      timer;

    HDfprintf(stderr, "I/O request size is %1.2f MB\n",
      size[0] * size[1] / (double)(1024 * 1024));

    /* Open the files */
    file = H5Fcreate(HDF5_FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    HDassert(file >= 0);
    fd = HDopen(RAW_FILE_NAME, O_RDWR|O_CREAT|O_TRUNC, 0666);
    HDassert(fd >= 0);

    /* Create the dataset */
    file_space = H5Screate_simple(2, size, size);
    HDassert(file_space >= 0);
    dset = H5Dcreate2(file, "dset", H5T_NATIVE_UCHAR, file_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    HDassert(dset >= 0);

    /* initial fill for lazy malloc */
    the_data = (unsigned char *)HDmalloc((size_t)(size[0] * size[1]));
    HDmemset(the_data, 0xAA, (size_t)(size[0] * size[1]));

    /************
     * Fill raw *
     ************/
    H5_timer_init(&timer);
    H5_timer_start(&timer);
    for(u = 0; u < nwrite; u++) {
        HDmemset(the_data, 0xAA, (size_t)(size[0]*size[1]));
    }
    H5_timer_stop(&timer);
    print_stats("fill raw", timer, (size_t)(nread * size[0] * size[1]));

    /*************
     * Fill hdf5 *
     *************/
    H5_timer_init(&timer);
    H5_timer_start(&timer);
    for(u = 0; u < nread; u++) {
        status = H5Dread(dset, H5T_NATIVE_UCHAR, file_space, file_space,
            H5P_DEFAULT, the_data);
        HDassert(status >= 0);
    }
    H5_timer_stop(&timer);
    print_stats("fill hdf5", timer, (size_t)(nread * size[0] * size[1]));

    /*************************
     * Write the raw dataset *
     *************************/
    H5_timer_init(&timer);
    H5_timer_start(&timer);
    for(u = 0; u < nwrite; u++) {
        offset = HDlseek(fd, (off_t)0, SEEK_SET);
        HDassert(0 == offset);
        n = HDwrite(fd, the_data, (h5_posix_io_t)(size[0] * size[1]));
        HDassert (n >= 0 && (size_t)n == size[0] * size[1]);
    }
    H5_timer_stop(&timer);
    print_stats("out raw", timer, (size_t)(nread * size[0] * size[1]));

    /**************************
     * Write the hdf5 dataset *
     **************************/
    H5_timer_init(&timer);
    H5_timer_start(&timer);
    for(u = 0; u < nwrite; u++) {
        status = H5Dwrite(dset, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, the_data);
        HDassert(status >= 0);
    }
    H5_timer_stop(&timer);
    print_stats("out hdf5", timer, (size_t)(nread * size[0] * size[1]));

    /************************
     * Read the raw dataset *
     ************************/
    H5_timer_init(&timer);
    H5_timer_start(&timer);
    for(u = 0; u < nread; u++) {
        offset = HDlseek(fd, (off_t)0, SEEK_SET);
        HDassert(0 == offset);
        n = HDread(fd, the_data, (h5_posix_io_t)(size[0] * size[1]));
        HDassert(n >= 0 && (size_t)n == size[0] * size[1]);
    }
    H5_timer_stop(&timer);
    print_stats("in raw", timer, (size_t)(nread * size[0] * size[1]));

    /*************************
     * Read the hdf5 dataset *
     *************************/
    H5_timer_init(&timer);
    H5_timer_start(&timer);
    for(u = 0; u < nread; u++) {
        status = H5Dread(dset, H5T_NATIVE_UCHAR, file_space, file_space,
            H5P_DEFAULT, the_data);
        HDassert(status >= 0);
    }
    H5_timer_stop(&timer);
    print_stats("in hdf5", timer, (size_t)(nread * size[0] * size[1]));

    /******************
     * Read hyperslab *
     ******************/
    HDassert(size[0] > 20 && size[1] > 20);
    start[0] = start[1] = 10;
    count[0] = count[1] = size[0] - 20;
    status = H5Sselect_hyperslab(file_space, H5S_SELECT_SET, start, NULL, count, NULL);
    HDassert(status >= 0);

    H5_timer_init(&timer);
    H5_timer_start(&timer);
    for(u = 0; u < nread; u++) {
        status = H5Dread(dset, H5T_NATIVE_UCHAR, file_space, file_space,
            H5P_DEFAULT, the_data);
        HDassert(status >= 0);
    }
    H5_timer_stop(&timer);
    print_stats("in hdf5 partial", timer, (size_t)(nread * count[0] * count[1]));

    /* Close everything */
    HDclose(fd);
    H5Dclose(dset);
    H5Sclose(file_space);
    H5Fclose(file);
    HDfree(the_data);

    return 0;
}

