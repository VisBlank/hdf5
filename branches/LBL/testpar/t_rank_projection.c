
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
   This program will test independant and collective reads and writes between
   selections of different rank that non-the-less are deemed as having the 
   same shape by H5Sselect_shape_same().
 */

#define H5S_PACKAGE             /*suppress error about including H5Spkg   */


#include "hdf5.h"
#include "H5private.h"
#include "testphdf5.h"
#include "H5Spkg.h"             /* Dataspaces                           */


/*-------------------------------------------------------------------------
 * Function:	contig_hyperslab_dr_pio_test__run_test()
 *
 * Purpose:	Test I/O to/from hyperslab selections of different rank in
 *		the parallel.
 *
 * Return:	void
 *
 * Programmer:	JRM -- 9/18/09
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

#define PAR_SS_DR_MAX_RANK	5
#define CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG 0

void
contig_hyperslab_dr_pio_test__run_test(const int test_num,
                                       const int edge_size,
                                       const int chunk_edge_size,
                                       const int small_rank,
                                       const int large_rank,
                                       const hbool_t use_collective_io,
                                       const hid_t dset_type)
{
    const char *fcnName = "contig_hyperslab_dr_pio_test()";
    const char *filename;
    hbool_t	use_gpfs = FALSE;   /* Use GPFS hints */
    hbool_t	mis_match = FALSE;
    int		i, j, k, l, m, n;
    int		mpi_size = -1;
    int         mpi_rank = -1;
    int         start_index;
    int         stop_index;
    const int   test_max_rank = 5;  /* must update code if this changes */
    uint32_t	expected_value;
    uint32_t  * small_ds_buf_0;
    uint32_t  * small_ds_buf_1;
    uint32_t  * small_ds_buf_2;
    uint32_t  * small_ds_slice_buf;
    uint32_t  * large_ds_buf_0;
    uint32_t  * large_ds_buf_1;
    uint32_t  * large_ds_buf_2;
    uint32_t  * large_ds_slice_buf;
    uint32_t  * ptr_0;
    uint32_t  * ptr_1;
    uint32_t  * ptr_2;
    MPI_Comm    mpi_comm = MPI_COMM_NULL;
    MPI_Info	mpi_info = MPI_INFO_NULL;
    hid_t       fid;			/* HDF5 file ID */
    hid_t	acc_tpl;		/* File access templates */
    hid_t	xfer_plist = H5P_DEFAULT;
    hid_t       full_mem_small_ds_sid;
    hid_t       full_file_small_ds_sid;
    hid_t       mem_small_ds_sid;
    hid_t       file_small_ds_sid;
    hid_t	small_ds_slice_sid;
    hid_t       full_mem_large_ds_sid;
    hid_t       full_file_large_ds_sid;
    hid_t       mem_large_ds_sid;
    hid_t       file_large_ds_sid;
    hid_t       file_large_ds_process_slice_sid;
    hid_t       mem_large_ds_process_slice_sid;
    hid_t	large_ds_slice_sid;
    hid_t       small_ds_dcpl_id = H5P_DEFAULT;
    hid_t       large_ds_dcpl_id = H5P_DEFAULT;
    hid_t       small_dataset;     /* Dataset ID                   */
    hid_t       large_dataset;     /* Dataset ID                   */
    size_t      small_ds_size = 1;
    size_t      small_ds_slice_size = 1;
    size_t      large_ds_size = 1;
    size_t      large_ds_slice_size = 1;
    hsize_t     dims[PAR_SS_DR_MAX_RANK];
    hsize_t     chunk_dims[PAR_SS_DR_MAX_RANK];
    hsize_t     start[PAR_SS_DR_MAX_RANK];
    hsize_t     stride[PAR_SS_DR_MAX_RANK];
    hsize_t     count[PAR_SS_DR_MAX_RANK];
    hsize_t     block[PAR_SS_DR_MAX_RANK];
    hsize_t   * start_ptr = NULL;
    hsize_t   * stride_ptr = NULL;
    hsize_t   * count_ptr = NULL;
    hsize_t   * block_ptr = NULL;
    htri_t      check;          /* Shape comparison return value */
    herr_t	ret;		/* Generic return value */

    HDassert( edge_size >= 6 );
    HDassert( edge_size >= chunk_edge_size );
    HDassert( ( chunk_edge_size == 0 ) || ( chunk_edge_size >= 3 ) );
    HDassert( 1 < small_rank );
    HDassert( small_rank < large_rank );
    HDassert( large_rank <= test_max_rank );
    HDassert( test_max_rank <= PAR_SS_DR_MAX_RANK );

    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    HDassert( mpi_size >= 1 );

    mpi_comm = MPI_COMM_WORLD;
    mpi_info = MPI_INFO_NULL;

    for ( i = 0; i < small_rank - 1; i++ )
    {
        small_ds_size *= (size_t)edge_size;
        small_ds_slice_size *= (size_t)edge_size;
    }
    small_ds_size *= (size_t)(mpi_size + 1);


    for ( i = 0; i < large_rank - 1; i++ ) {

        large_ds_size *= (size_t)edge_size;
        large_ds_slice_size *= (size_t)edge_size;
    }
    large_ds_size *= (size_t)(mpi_size + 1);


    /* set up the start, stride, count, and block pointers */
    start_ptr  = &(start[PAR_SS_DR_MAX_RANK - large_rank]);
    stride_ptr = &(stride[PAR_SS_DR_MAX_RANK - large_rank]);
    count_ptr  = &(count[PAR_SS_DR_MAX_RANK - large_rank]);
    block_ptr  = &(block[PAR_SS_DR_MAX_RANK - large_rank]);


    /* Allocate buffers */
    small_ds_buf_0 = (uint32_t *)HDmalloc(sizeof(uint32_t) * small_ds_size);
    VRFY((small_ds_buf_0 != NULL), "malloc of small_ds_buf_0 succeeded");

    small_ds_buf_1 = (uint32_t *)HDmalloc(sizeof(uint32_t) * small_ds_size);
    VRFY((small_ds_buf_1 != NULL), "malloc of small_ds_buf_1 succeeded");

    small_ds_buf_2 = (uint32_t *)HDmalloc(sizeof(uint32_t) * small_ds_size);
    VRFY((small_ds_buf_2 != NULL), "malloc of small_ds_buf_2 succeeded");

    small_ds_slice_buf = 
	(uint32_t *)HDmalloc(sizeof(uint32_t) * small_ds_slice_size);
    VRFY((small_ds_slice_buf != NULL), "malloc of small_ds_slice_buf succeeded");

    large_ds_buf_0 = (uint32_t *)HDmalloc(sizeof(uint32_t) * large_ds_size);
    VRFY((large_ds_buf_0 != NULL), "malloc of large_ds_buf_0 succeeded");

    large_ds_buf_1 = (uint32_t *)HDmalloc(sizeof(uint32_t) * large_ds_size);
    VRFY((large_ds_buf_1 != NULL), "malloc of large_ds_buf_1 succeeded");

    large_ds_buf_2 = (uint32_t *)HDmalloc(sizeof(uint32_t) * large_ds_size);
    VRFY((large_ds_buf_2 != NULL), "malloc of large_ds_buf_2 succeeded");

    large_ds_slice_buf = 
	(uint32_t *)HDmalloc(sizeof(uint32_t) * large_ds_slice_size);
    VRFY((large_ds_slice_buf != NULL), "malloc of large_ds_slice_buf succeeded");

    /* initialize the buffers */

    ptr_0 = small_ds_buf_0;
    ptr_1 = small_ds_buf_1;
    ptr_2 = small_ds_buf_2;

    for ( i = 0; i < (int)small_ds_size; i++ ) {

        *ptr_0 = (uint32_t)i;
        *ptr_1 = 0;
        *ptr_2 = 0;

        ptr_0++;
        ptr_1++;
        ptr_2++;
    }

    ptr_0 = small_ds_slice_buf;

    for ( i = 0; i < (int)small_ds_slice_size; i++ ) {

	*ptr_0 = (uint32_t)i;
        ptr_0++;
    }

    ptr_0 = large_ds_buf_0;
    ptr_1 = large_ds_buf_1;
    ptr_2 = large_ds_buf_2;

    for ( i = 0; i < (int)large_ds_size; i++ ) {

        *ptr_0 = (uint32_t)i;
        *ptr_1 = 0;
        *ptr_2 = 0;

        ptr_0++;
        ptr_1++;
        ptr_2++;
    }

    ptr_0 = large_ds_slice_buf;

    for ( i = 0; i < (int)large_ds_slice_size; i++ ) {

	*ptr_0 = (uint32_t)0;
        ptr_0++;
    }

    filename = (const char *)GetTestParameters();
    HDassert( filename != NULL );
#if 0 
    if ( MAINPROCESS ) {

        HDfprintf(stdout, "%d: test num = %d.\n", mpi_rank, test_num);
        HDfprintf(stdout, "%d: mpi_size = %d.\n", mpi_rank, mpi_size);
        HDfprintf(stdout, 
                  "%d: small/large rank = %d/%d, use_collective_io = %d.\n",
                  mpi_rank, small_rank, large_rank, (int)use_collective_io);
        HDfprintf(stdout, "%d: edge_size = %d, chunk_edge_size = %d.\n",
                  mpi_rank, edge_size, chunk_edge_size);
        HDfprintf(stdout, "%d: small_ds_size = %d, large_ds_size = %d.\n",
                  mpi_rank, (int)small_ds_size, (int)large_ds_size);
        HDfprintf(stdout, "%d: filename = %s.\n", mpi_rank, filename);
    }
#endif
    /* ----------------------------------------
     * CREATE AN HDF5 FILE WITH PARALLEL ACCESS
     * ---------------------------------------*/
    /* setup file access template */
    acc_tpl = create_faccess_plist(mpi_comm, mpi_info, facc_type, use_gpfs);
    VRFY((acc_tpl >= 0), "create_faccess_plist() succeeded");

    /* create the file collectively */
    fid = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, acc_tpl);
    VRFY((fid >= 0), "H5Fcreate succeeded");

    MESG("File opened.");

    /* Release file-access template */
    ret = H5Pclose(acc_tpl);
    VRFY((ret >= 0), "H5Pclose(acc_tpl) succeeded");


    /* setup dims: */
    dims[0] = (int)(mpi_size + 1);
    dims[1] = dims[2] = dims[3] = dims[4] = edge_size;


    /* Create small ds dataspaces */
    full_mem_small_ds_sid = H5Screate_simple(small_rank, dims, NULL);
    VRFY((full_mem_small_ds_sid != 0), 
         "H5Screate_simple() full_mem_small_ds_sid succeeded");

    full_file_small_ds_sid = H5Screate_simple(small_rank, dims, NULL);
    VRFY((full_file_small_ds_sid != 0), 
         "H5Screate_simple() full_file_small_ds_sid succeeded");

    mem_small_ds_sid = H5Screate_simple(small_rank, dims, NULL);
    VRFY((mem_small_ds_sid != 0), 
	 "H5Screate_simple() mem_small_ds_sid succeeded");

    file_small_ds_sid = H5Screate_simple(small_rank, dims, NULL);
    VRFY((file_small_ds_sid != 0), 
         "H5Screate_simple() file_small_ds_sid succeeded");

    small_ds_slice_sid = H5Screate_simple(small_rank - 1, &(dims[1]), NULL);
    VRFY((small_ds_slice_sid != 0), 
         "H5Screate_simple() small_ds_slice_sid succeeded");


    /* Create large ds dataspaces */
    full_mem_large_ds_sid = H5Screate_simple(large_rank, dims, NULL);
    VRFY((full_mem_large_ds_sid != 0), 
         "H5Screate_simple() full_mem_large_ds_sid succeeded");

    full_file_large_ds_sid = H5Screate_simple(large_rank, dims, NULL);
    VRFY((full_file_large_ds_sid != FAIL), 
         "H5Screate_simple() full_file_large_ds_sid succeeded");

    mem_large_ds_sid = H5Screate_simple(large_rank, dims, NULL);
    VRFY((mem_large_ds_sid != FAIL), 
         "H5Screate_simple() mem_large_ds_sid succeeded");

    file_large_ds_sid = H5Screate_simple(large_rank, dims, NULL);
    VRFY((file_large_ds_sid != FAIL), 
         "H5Screate_simple() file_large_ds_sid succeeded");

    mem_large_ds_process_slice_sid = H5Screate_simple(large_rank, dims, NULL);
    VRFY((mem_large_ds_process_slice_sid != FAIL), 
         "H5Screate_simple() mem_large_ds_process_slice_sid succeeded");

    file_large_ds_process_slice_sid = H5Screate_simple(large_rank, dims, NULL);
    VRFY((file_large_ds_process_slice_sid != FAIL), 
         "H5Screate_simple() file_large_ds_process_slice_sid succeeded");


    large_ds_slice_sid = H5Screate_simple(large_rank - 1, &(dims[1]), NULL);
    VRFY((large_ds_slice_sid != 0), 
         "H5Screate_simple() large_ds_slice_sid succeeded");


    /* Select the entire extent of the full small ds, and ds slice dataspaces */
    ret = H5Sselect_all(full_mem_small_ds_sid);
    VRFY((ret != FAIL), "H5Sselect_all(full_mem_small_ds_sid) succeeded");

    ret = H5Sselect_all(full_file_small_ds_sid);
    VRFY((ret != FAIL), "H5Sselect_all(full_file_small_ds_sid) succeeded");

    ret = H5Sselect_all(small_ds_slice_sid);
    VRFY((ret != FAIL), "H5Sselect_all(small_ds_slice_sid) succeeded");


    /* Select the entire extent of the full large ds, and ds slice dataspaces */
    ret = H5Sselect_all(full_mem_large_ds_sid);
    VRFY((ret != FAIL), "H5Sselect_all(full_mem_large_ds_sid) succeeded");

    ret = H5Sselect_all(full_file_large_ds_sid);
    VRFY((ret != FAIL), "H5Sselect_all(full_file_large_ds_sid) succeeded");

    ret = H5Sselect_all(large_ds_slice_sid);
    VRFY((ret != FAIL), "H5Sselect_all(large_ds_slice_sid) succeeded");


    /* if chunk edge size is greater than zero, set up the small and
     * large data set creation property lists to specify chunked
     * datasets.
     */
    if ( chunk_edge_size > 0 ) {

        chunk_dims[0] = mpi_size + 1;
        chunk_dims[1] = chunk_dims[2] = 
                        chunk_dims[3] = chunk_dims[4] = chunk_edge_size;

        small_ds_dcpl_id = H5Pcreate(H5P_DATASET_CREATE);
        VRFY((ret != FAIL), "H5Pcreate() small_ds_dcpl_id succeeded");

        ret = H5Pset_layout(small_ds_dcpl_id, H5D_CHUNKED);
        VRFY((ret != FAIL), "H5Pset_layout() small_ds_dcpl_id succeeded");

        ret = H5Pset_chunk(small_ds_dcpl_id, small_rank, chunk_dims);
        VRFY((ret != FAIL), "H5Pset_chunk() small_ds_dcpl_id succeeded");


        large_ds_dcpl_id = H5Pcreate(H5P_DATASET_CREATE);
        VRFY((ret != FAIL), "H5Pcreate() large_ds_dcpl_id succeeded");

        ret = H5Pset_layout(large_ds_dcpl_id, H5D_CHUNKED);
        VRFY((ret != FAIL), "H5Pset_layout() large_ds_dcpl_id succeeded");

        ret = H5Pset_chunk(large_ds_dcpl_id, large_rank, chunk_dims);
        VRFY((ret != FAIL), "H5Pset_chunk() large_ds_dcpl_id succeeded");
    }

    /* create the small dataset */
    small_dataset = H5Dcreate2(fid, "small_dataset", dset_type,
                               file_small_ds_sid, H5P_DEFAULT,
                               small_ds_dcpl_id, H5P_DEFAULT);
    VRFY((ret != FAIL), "H5Dcreate2() small_dataset succeeded");

    /* create the large dataset */
    large_dataset = H5Dcreate2(fid, "large_dataset", dset_type,
                               file_large_ds_sid, H5P_DEFAULT,
                               large_ds_dcpl_id, H5P_DEFAULT);
    VRFY((ret != FAIL), "H5Dcreate2() large_dataset succeeded");



    /* setup xfer property list */
    xfer_plist = H5Pcreate(H5P_DATASET_XFER);
    VRFY((xfer_plist >= 0), "H5Pcreate(H5P_DATASET_XFER) succeeded");

    ret = H5Pset_dxpl_mpio(xfer_plist, H5FD_MPIO_COLLECTIVE);
    VRFY((ret >= 0), "H5Pset_dxpl_mpio succeeded");

    if ( ! use_collective_io ) {

        ret = H5Pset_dxpl_mpio_collective_opt(xfer_plist,
                                              H5FD_MPIO_INDIVIDUAL_IO);
        VRFY((ret>= 0), "H5Pset_dxpl_mpio_collective_opt() suceeded");
    }

    /* setup selection to write initial data to the small and large data sets */
    start[0] = mpi_rank;
    stride[0] = 2 * (mpi_size + 1);
    count[0] = 1;
    block[0] = 1;

    for ( i = 1; i < large_rank; i++ ) {

        start[i] = 0;
        stride[i] = 2 * edge_size;
        count[i] = 1;
        block[i] = edge_size;
    }

    /* setup selections for writing initial data to the small data set */
    ret = H5Sselect_hyperslab(mem_small_ds_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), "H5Sselect_hyperslab(mem_small_ds_sid, set) suceeded");

    ret = H5Sselect_hyperslab(file_small_ds_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), "H5Sselect_hyperslab(file_small_ds_sid, set) suceeded");

    if ( MAINPROCESS ) { /* add an additional slice to the selections */

        start[0] = mpi_size;

        ret = H5Sselect_hyperslab(mem_small_ds_sid,
                                  H5S_SELECT_OR,
                                  start,
                                  stride,
                                  count,
                                  block);
        VRFY((ret>= 0), "H5Sselect_hyperslab(mem_small_ds_sid, or) suceeded");

        ret = H5Sselect_hyperslab(file_small_ds_sid,
                                  H5S_SELECT_OR,
                                  start,
                                  stride,
                                  count,
                                  block);
        VRFY((ret>= 0), "H5Sselect_hyperslab(file_small_ds_sid, or) suceeded");
    }


    /* write the initial value of the small data set to file */
    ret = H5Dwrite(small_dataset, dset_type, mem_small_ds_sid, file_small_ds_sid,
                   xfer_plist, small_ds_buf_0);
#if 0 /* JRM */
    if ( ret < 0 ) H5Eprint(H5E_DEFAULT, stderr);
#endif /* JRM */
    VRFY((ret >= 0), "H5Dwrite() small_dataset initial write succeeded");


    /* read the small data set back to verify that it contains the 
     * expected data.  Note that each process reads in the entire 
     * data set.
     */
    ret = H5Dread(small_dataset,
                  H5T_NATIVE_UINT32,
                  full_mem_small_ds_sid,
                  full_file_small_ds_sid,
                  xfer_plist,
                  small_ds_buf_1);
    VRFY((ret >= 0), "H5Dread() small_dataset initial read succeeded");


    /* verify that the correct data was written to the small data set */
    expected_value = 0;
    mis_match = FALSE;
    ptr_1 = small_ds_buf_1;

    i = 0;
    for ( i = 0; i < (int)small_ds_size; i++ ) {

        if ( *ptr_1 != expected_value ) {

            mis_match = TRUE;
        }
        ptr_1++;
        expected_value++;
    }
    VRFY( (mis_match == FALSE), "small ds init data good.");



    /* setup selections for writing initial data to the large data set */

    start[0] = mpi_rank;

    ret = H5Sselect_hyperslab(mem_large_ds_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), "H5Sselect_hyperslab(mem_large_ds_sid, set) suceeded");

    ret = H5Sselect_hyperslab(file_large_ds_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), "H5Sselect_hyperslab(file_large_ds_sid, set) suceeded");
 
    /* In passing, setup the process slice data spaces as well */

    ret = H5Sselect_hyperslab(mem_large_ds_process_slice_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), 
         "H5Sselect_hyperslab(mem_large_ds_process_slice_sid, set) suceeded");

    ret = H5Sselect_hyperslab(file_large_ds_process_slice_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), 
         "H5Sselect_hyperslab(file_large_ds_process_slice_sid, set) suceeded");

    if ( MAINPROCESS ) { /* add an additional slice to the selections */

        start[0] = mpi_size;

        ret = H5Sselect_hyperslab(mem_large_ds_sid,
                                  H5S_SELECT_OR,
                                  start,
                                  stride,
                                  count,
                                  block);
        VRFY((ret>= 0), "H5Sselect_hyperslab(mem_large_ds_sid, or) suceeded");

        ret = H5Sselect_hyperslab(file_large_ds_sid,
                                  H5S_SELECT_OR,
                                  start,
                                  stride,
                                  count,
                                  block);
        VRFY((ret>= 0), "H5Sselect_hyperslab(file_large_ds_sid, or) suceeded");
    }


    /* write the initial value of the large data set to file */
    ret = H5Dwrite(large_dataset, dset_type, mem_large_ds_sid, file_large_ds_sid,
                   xfer_plist, large_ds_buf_0);
    if ( ret < 0 ) H5Eprint(H5E_DEFAULT, stderr);
    VRFY((ret >= 0), "H5Dwrite() large_dataset initial write succeeded");


    /* read the small data set back to verify that it contains the 
     * expected data.  Note that each process reads in the entire 
     * data set.
     */
    ret = H5Dread(large_dataset,
                  H5T_NATIVE_UINT32,
                  full_mem_large_ds_sid,
                  full_file_large_ds_sid,
                  xfer_plist,
                  large_ds_buf_1);
    VRFY((ret >= 0), "H5Dread() large_dataset initial read succeeded");


    /* verify that the correct data was written to the small data set */
    expected_value = 0;
    mis_match = FALSE;
    ptr_1 = large_ds_buf_1;

    i = 0;
    for ( i = 0; i < (int)large_ds_size; i++ ) {

        if ( *ptr_1 != expected_value ) {

            mis_match = TRUE;
        }
        ptr_1++;
        expected_value++;
    }
    VRFY( (mis_match == FALSE), "large ds init data good.");


    /* first, verify that we can read from disk correctly using selections
     * of different rank that H5S_select_shape_same() views as being of the
     * same shape.
     *
     * Start by reading small_rank-D - 1 slice from the on disk large cube, 
     * and verifying that the data read is correct.  Verify that 
     * H5S_select_shape_same() returns true on the memory and file selections.
     */

    /* We have already done a H5Sselect_all() on the data space 
     * small_ds_slice_sid, so no need to call H5Sselect_all() again.
     */

    /* set up start, stride, count, and block -- note that we will
     * change start[] so as to read slices of the large cube.
     */
    for ( i = 0; i < PAR_SS_DR_MAX_RANK; i++ ) {

        start[i] = 0;
        stride[i] = 2 * edge_size;
        count[i] = 1;
        if ( (PAR_SS_DR_MAX_RANK - i) > (small_rank - 1) ) {

            block[i] = 1;

        } else {

            block[i] = edge_size;
        }
    }

#if CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG /* JRM */
    HDfprintf(stdout, 
              "%s reading slices from big cube on disk into small cube slice.\n",
              fcnName);
#endif /* JRM */
    /* in serial versions of this test, we loop through all the dimensions
     * of the large data set.  However, in the parallel version, each 
     * process only works with that slice of the large cube indicated
     * by its rank -- hence we set the most slowly changing index to 
     * mpi_rank, and don't itterate over it.
     */

    if ( PAR_SS_DR_MAX_RANK - large_rank == 0 ) {

        i = mpi_rank;

    } else {

        i = 0;
    }

    /* since large_rank is at most PAR_SS_DR_MAX_RANK, no need to 
     * loop over it -- either we are setting i to mpi_rank, or
     * we are setting it to zero.  It will not change during the 
     * test.
     */

    if ( PAR_SS_DR_MAX_RANK - large_rank == 1 ) {

        j = mpi_rank;

    } else {

        j = 0;
    }

    do {
        if ( PAR_SS_DR_MAX_RANK - large_rank == 2 ) {

            k = mpi_rank;

        } else {

            k = 0;
        }

        do {
            /* since small rank >= 2 and large_rank > small_rank, we 
             * have large_rank >= 3.  Since PAR_SS_DR_MAX_RANK == 5
             * (baring major re-orgaization), this gives us:
             *
             *     (PAR_SS_DR_MAX_RANK - large_rank) <= 2
             *
             * so no need to repeat the test in the outer loops --
             * just set l = 0.
             */

            l = 0;
            do {
                /* we know that small_rank - 1 >= 1 and that 
                 * large_rank > small_rank by the assertions at the head 
                 * of this function.  Thus no need for another inner loop.
                 */
                start[0] = i;
                start[1] = j;
                start[2] = k;
                start[3] = l;
                start[4] = 0;

                ret = H5Sselect_hyperslab(file_large_ds_sid,
                                          H5S_SELECT_SET,
                                          start_ptr,
                                          stride_ptr,
                                          count_ptr,
                                          block_ptr);
                VRFY((ret != FAIL), 
                     "H5Sselect_hyperslab(file_large_cube_sid) succeeded");


                /* verify that H5S_select_shape_same() reports the two
                 * selections as having the same shape.
                 */
                check = H5S_select_shape_same_test(small_ds_slice_sid,
                                                   file_large_ds_sid);
                VRFY((check == TRUE), "H5S_select_shape_same_test passed");


                /* Read selection from disk */
#if CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG /* JRM */
                HDfprintf(stdout, "%s:%d: start = %d %d %d %d %d.\n", fcnName,
                          mpi_rank, start[0], start[1], start[2], start[3], 
                          start[4]);
                HDfprintf(stdout, "%s slice/file extent dims = %d/%d.\n",
                          fcnName,
                          H5Sget_simple_extent_ndims(small_ds_slice_sid),
                          H5Sget_simple_extent_ndims(file_large_ds_sid));
#endif /* JRM */
                ret = H5Dread(large_dataset,
                              H5T_NATIVE_UINT32,
                              small_ds_slice_sid,
                              file_large_ds_sid,
                              xfer_plist,
                              small_ds_slice_buf);
                VRFY((ret >= 0), "H5Sread() slice from large ds succeeded.");


                /* verify that expected data is retrieved */

                mis_match = FALSE;
                ptr_1 = small_ds_slice_buf;
                expected_value = 
			(i * edge_size * edge_size * edge_size * edge_size) +
                        (j * edge_size * edge_size * edge_size) +
                        (k * edge_size * edge_size) +
                        (l * edge_size);

                for ( n = 0; n < (int)small_ds_slice_size; n++ ) {

                    if ( *ptr_1 != expected_value ) {

                        mis_match = TRUE;
                    }

                    *ptr_1 = 0; /* zero data for next use */

                    ptr_1++;
                    expected_value++;
                }

                VRFY((mis_match == FALSE), 
                     "small slice read from large ds data good.");
                l++;

            } while ( ( large_rank > 2 ) &&
                      ( (small_rank - 1) <= 1 ) &&
                      ( l < edge_size ) );
            k++;
        } while ( ( large_rank > 3 ) &&
                  ( (small_rank - 1) <= 2 ) &&
                  ( k < edge_size ) );
        j++;
    } while ( ( large_rank > 4 ) &&
              ( (small_rank - 1) <= 3 ) &&
              ( j < edge_size ) );


    /* similarly, read slices of the on disk small data set into slices 
     * through the in memory large data set, and verify that the correct 
     * data (and only the correct data) is read.
     */

    start[0] = mpi_rank;
    stride[0] = 2 * (mpi_size + 1);
    count[0] = 1;
    block[0] = 1;

    for ( i = 1; i < large_rank; i++ ) {

        start[i] = 0;
        stride[i] = 2 * edge_size;
        count[i] = 1;
        block[i] = edge_size;
    }

    ret = H5Sselect_hyperslab(file_small_ds_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), "H5Sselect_hyperslab(file_small_ds_sid, set) suceeded");


#if CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG /* JRM */
    HDfprintf(stdout, 
      "%s reading slices of on disk small data set into slices of big data set.\n",
              fcnName);
#endif /* JRM */

    /* zero out the in memory large ds */
    ptr_1 = large_ds_buf_1;
    for ( n = 0; n < (int)large_ds_size; n++ ) {

        *ptr_1 = 0;
        ptr_1++;
    }

    /* set up start, stride, count, and block -- note that we will
     * change start[] so as to read slices of the large cube.
     */
    for ( i = 0; i < PAR_SS_DR_MAX_RANK; i++ ) {

        start[i] = 0;
        stride[i] = 2 * edge_size;
        count[i] = 1;
        if ( (PAR_SS_DR_MAX_RANK - i) > (small_rank - 1) ) {

            block[i] = 1;

        } else {

            block[i] = edge_size;
        }
    }


    /* in serial versions of this test, we loop through all the dimensions
     * of the large data set that don't appear in the small data set.  
     *
     * However, in the parallel version, each process only works with that 
     * slice of the large (and small) data set indicated by its rank -- hence 
     * we set the most slowly changing index to mpi_rank, and don't itterate 
     * over it.
     */


    if ( PAR_SS_DR_MAX_RANK - large_rank == 0 ) {

        i = mpi_rank;

    } else {

        i = 0;
    }

    /* since large_rank is at most PAR_SS_DR_MAX_RANK, no need to 
     * loop over it -- either we are setting i to mpi_rank, or
     * we are setting it to zero.  It will not change during the 
     * test.
     */

    if ( PAR_SS_DR_MAX_RANK - large_rank == 1 ) {

        j = mpi_rank;

    } else {

        j = 0;
    }

    do {
        if ( PAR_SS_DR_MAX_RANK - large_rank == 2 ) {

            k = mpi_rank;

        } else {

            k = 0;
        }

        do {
            /* since small rank >= 2 and large_rank > small_rank, we 
             * have large_rank >= 3.  Since PAR_SS_DR_MAX_RANK == 5
             * (baring major re-orgaization), this gives us:
             *
             *     (PAR_SS_DR_MAX_RANK - large_rank) <= 2
             *
             * so no need to repeat the test in the outer loops --
             * just set l = 0.
             */

            l = 0;
            do {
                /* we know that small_rank >= 1 and that large_rank > small_rank
                 * by the assertions at the head of this function.  Thus no
                 * need for another inner loop.
                 */
                start[0] = i;
                start[1] = j;
                start[2] = k;
                start[3] = l;
                start[4] = 0;

                ret = H5Sselect_hyperslab(mem_large_ds_sid,
                                          H5S_SELECT_SET,
                                          start_ptr,
                                          stride_ptr,
                                          count_ptr,
                                          block_ptr);
                VRFY((ret != FAIL), 
                     "H5Sselect_hyperslab(mem_large_ds_sid) succeeded");


                /* verify that H5S_select_shape_same() reports the two
                 * selections as having the same shape.
                 */
                check = H5S_select_shape_same_test(file_small_ds_sid,
                                                   mem_large_ds_sid);
                VRFY((check == TRUE), "H5S_select_shape_same_test passed");


                /* Read selection from disk */
#if CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG /* JRM */
                HDfprintf(stdout, "%s:%d: start = %d %d %d %d %d.\n", 
                          fcnName, mpi_rank, 
                          start[0], start[1], start[2], start[3], start[4]);
                HDfprintf(stdout, "%s:%d: mem/file extent dims = %d/%d.\n",
                          fcnName, mpi_rank,
                          H5Sget_simple_extent_ndims(mem_large_ds_sid),
                          H5Sget_simple_extent_ndims(file_small_ds_sid));
#endif /* JRM */
                ret = H5Dread(small_dataset,
                              H5T_NATIVE_UINT32,
                              mem_large_ds_sid,
                              file_small_ds_sid,
                              xfer_plist,
                              large_ds_buf_1);
                VRFY((ret >= 0), "H5Sread() slice from small ds succeeded.");

                /* verify that the expected data and only the
                 * expected data was read.
                 */
                ptr_1 = large_ds_buf_1;
                expected_value = mpi_rank * small_ds_slice_size;
                start_index = 
                        (i * edge_size * edge_size * edge_size * edge_size) +
                        (j * edge_size * edge_size * edge_size) +
                        (k * edge_size * edge_size) +
                        (l * edge_size);
                stop_index = start_index + (int)small_ds_slice_size - 1;

                HDassert( 0 <= start_index );
                HDassert( start_index < stop_index );
                HDassert( stop_index <= (int)large_ds_size );

                for ( n = 0; n < (int)large_ds_size; n++ ) {

                    if ( ( n >= start_index ) && ( n <= stop_index ) ) {

                        if ( *ptr_1 != expected_value ) {

                            mis_match = TRUE;
                        }
                        expected_value++;

                    } else {

                        if ( *ptr_1 != 0 ) {

                            mis_match = TRUE;
                        }
                    }
                    /* zero out the value for the next pass */
                    *ptr_1 = 0;

                    ptr_1++;
                }

                VRFY((mis_match == FALSE), 
                     "small slice read from large ds data good.");

                l++;

            } while ( ( large_rank > 2 ) &&
                      ( (small_rank - 1) <= 1 ) &&
                      ( l < edge_size ) );
            k++;
        } while ( ( large_rank > 3 ) &&
                  ( (small_rank - 1) <= 2 ) &&
                  ( k < edge_size ) );
        j++;
    } while ( ( large_rank > 4 ) &&
              ( (small_rank - 1) <= 3 ) &&
              ( j < edge_size ) );


    /* now we go in the opposite direction, verifying that we can write
     * from memory to file using selections of different rank that
     * H5S_select_shape_same() views as being of the same shape.
     *
     * Start by writing small_rank - 1 D slices from the in memory large data
     * set to the on disk small cube dataset.  After each write, read the 
     * slice of the small dataset back from disk, and verify that it contains 
     * the expected data. Verify that H5S_select_shape_same() returns true on 
     * the memory and file selections.
     */

    start[0] = mpi_rank;
    stride[0] = 2 * (mpi_size + 1);
    count[0] = 1;
    block[0] = 1;

    for ( i = 1; i < large_rank; i++ ) {

        start[i] = 0;
        stride[i] = 2 * edge_size;
        count[i] = 1;
        block[i] = edge_size;
    }

    ret = H5Sselect_hyperslab(file_small_ds_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), "H5Sselect_hyperslab(file_small_ds_sid, set) suceeded");

    ret = H5Sselect_hyperslab(mem_small_ds_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), "H5Sselect_hyperslab(mem_small_ds_sid, set) suceeded");


    /* set up start, stride, count, and block -- note that we will
     * change start[] so as to read slices of the large cube.
     */
    for ( i = 0; i < PAR_SS_DR_MAX_RANK; i++ ) {

        start[i] = 0;
        stride[i] = 2 * edge_size;
        count[i] = 1;
        if ( (PAR_SS_DR_MAX_RANK - i) > (small_rank - 1) ) {

            block[i] = 1;

        } else {

            block[i] = edge_size;
        }
    }

    /* zero out the in memory small ds */
    ptr_1 = small_ds_buf_1;
    for ( n = 0; n < (int)small_ds_size; n++ ) {

        *ptr_1 = 0;
        ptr_1++;
    }


#if CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG /* JRM */
    HDfprintf(stdout, 
              "%s writing slices from big ds to slices of small ds on disk.\n",
              fcnName);
#endif /* JRM */

    /* in serial versions of this test, we loop through all the dimensions
     * of the large data set that don't appear in the small data set.  
     *
     * However, in the parallel version, each process only works with that 
     * slice of the large (and small) data set indicated by its rank -- hence 
     * we set the most slowly changing index to mpi_rank, and don't itterate 
     * over it.
     */


    if ( PAR_SS_DR_MAX_RANK - large_rank == 0 ) {

        i = mpi_rank;

    } else {

        i = 0;
    }

    /* since large_rank is at most PAR_SS_DR_MAX_RANK, no need to 
     * loop over it -- either we are setting i to mpi_rank, or
     * we are setting it to zero.  It will not change during the 
     * test.
     */

    if ( PAR_SS_DR_MAX_RANK - large_rank == 1 ) {

        j = mpi_rank;

    } else {

        j = 0;
    }

    j = 0;
    do {
        if ( PAR_SS_DR_MAX_RANK - large_rank == 2 ) {

            k = mpi_rank;

        } else {

            k = 0;
        }

        do {
            /* since small rank >= 2 and large_rank > small_rank, we 
             * have large_rank >= 3.  Since PAR_SS_DR_MAX_RANK == 5
             * (baring major re-orgaization), this gives us:
             *
             *     (PAR_SS_DR_MAX_RANK - large_rank) <= 2
             *
             * so no need to repeat the test in the outer loops --
             * just set l = 0.
             */

            l = 0;
            do {
                /* we know that small_rank >= 1 and that large_rank > small_rank
                 * by the assertions at the head of this function.  Thus no
                 * need for another inner loop.
                 */

                /* zero out this rank's slice of the on disk small data set */
                ret = H5Dwrite(small_dataset,
                               H5T_NATIVE_UINT32,
                               mem_small_ds_sid,
                               file_small_ds_sid,
                               xfer_plist,
                               small_ds_buf_2);
                VRFY((ret >= 0), "H5Dwrite() zero slice to small ds succeeded.");

                /* select the portion of the in memory large cube from which we
                 * are going to write data.
                 */
                start[0] = i;
                start[1] = j;
                start[2] = k;
                start[3] = l;
                start[4] = 0;

                ret = H5Sselect_hyperslab(mem_large_ds_sid,
                                          H5S_SELECT_SET,
                                          start_ptr,
                                          stride_ptr,
                                          count_ptr,
                                          block_ptr);
                VRFY((ret >= 0), 
                     "H5Sselect_hyperslab() mem_large_ds_sid succeeded.");


                /* verify that H5S_select_shape_same() reports the in
                 * memory slice through the cube selection and the
                 * on disk full square selections as having the same shape.
                 */
                check = H5S_select_shape_same_test(file_small_ds_sid,
                                                   mem_large_ds_sid);
                VRFY((check == TRUE), "H5S_select_shape_same_test passed.");


                /* write the slice from the in memory large data set to the 
                 * slice of the on disk small dataset. */
#if CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG /* JRM */
                HDfprintf(stdout, "%s:%d: start = %d %d %d %d %d.\n", 
                          fcnName, mpi_rank,
                          start[0], start[1], start[2], start[3], start[4]);
                HDfprintf(stdout, "%s:%d: mem/file extent dims = %d/%d.\n",
                          fcnName, mpi_rank,
                          H5Sget_simple_extent_ndims(mem_large_ds_sid),
                          H5Sget_simple_extent_ndims(file_small_ds_sid));
#endif /* JRM */
                ret = H5Dwrite(small_dataset,
                               H5T_NATIVE_UINT32,
                               mem_large_ds_sid,
                               file_small_ds_sid,
                               xfer_plist,
                               large_ds_buf_0);
                VRFY((ret >= 0), "H5Dwrite() slice to large ds succeeded.");


                /* read the on disk square into memory */
                ret = H5Dread(small_dataset,
                              H5T_NATIVE_UINT32,
                              mem_small_ds_sid,
                              file_small_ds_sid,
                              xfer_plist,
                              small_ds_buf_1);
                VRFY((ret >= 0), "H5Dread() slice from small ds succeeded.");


                /* verify that expected data is retrieved */

                mis_match = FALSE;
                ptr_1 = small_ds_buf_1;

                expected_value = 
			(i * edge_size * edge_size * edge_size * edge_size) +
                        (j * edge_size * edge_size * edge_size) +
                        (k * edge_size * edge_size) +
                        (l * edge_size);

                start_index = mpi_rank * small_ds_slice_size;
                stop_index = start_index + small_ds_slice_size - 1;

                HDassert( 0 <= start_index );
                HDassert( start_index < stop_index );
                HDassert( stop_index <= (int)small_ds_size );

                for ( n = 0; n < (int)small_ds_size; n++ ) {

                    if ( ( n >= start_index ) && ( n <= stop_index ) ) {

                        if ( *ptr_1 != expected_value ) {

                            mis_match = TRUE;
                        }
                        expected_value++;

                    } else {

                        if ( *ptr_1 != 0 ) {

                            mis_match = TRUE;
                        }
                    }
                    /* zero out the value for the next pass */
                    *ptr_1 = 0;

                    ptr_1++;
                }

                VRFY((mis_match == FALSE), 
                     "small slice write from large ds data good.");

                l++;

            } while ( ( large_rank > 2 ) &&
                      ( (small_rank - 1) <= 1 ) &&
                      ( l < edge_size ) );
            k++;
        } while ( ( large_rank > 3 ) &&
                  ( (small_rank - 1) <= 2 ) &&
                  ( k < edge_size ) );
        j++;
    } while ( ( large_rank > 4 ) &&
              ( (small_rank - 1) <= 3 ) &&
              ( j < edge_size ) );


    /* Now write the contents of the process's slice of the in memory 
     * small data set to slices of the on disk large data set.  After 
     * each write, read the process's slice of the large data set back
     * into memory, and verify that it contains the expected data. 
     * Verify that H5S_select_shape_same() returns true on the memory 
     * and file selections.
     */

    /* select the slice of the in memory small data set associated with 
     * the process's mpi rank.
     */
    start[0] = mpi_rank;
    stride[0] = 2 * (mpi_size + 1);
    count[0] = 1;
    block[0] = 1;

    for ( i = 1; i < large_rank; i++ ) {

        start[i] = 0;
        stride[i] = 2 * edge_size;
        count[i] = 1;
        block[i] = edge_size;
    }

    ret = H5Sselect_hyperslab(mem_small_ds_sid,
                              H5S_SELECT_SET,
                              start,
                              stride,
                              count,
                              block);
    VRFY((ret >= 0), "H5Sselect_hyperslab(mem_small_ds_sid, set) suceeded");


    /* set up start, stride, count, and block -- note that we will
     * change start[] so as to write slices of the small data set to
     * slices of the large data set.
     */
    for ( i = 0; i < PAR_SS_DR_MAX_RANK; i++ ) {

        start[i] = 0;
        stride[i] = 2 * edge_size;
        count[i] = 1;
        if ( (PAR_SS_DR_MAX_RANK - i) > (small_rank - 1) ) {

            block[i] = 1;

        } else {

            block[i] = edge_size;
        }
    }

    /* zero out the in memory large ds */
    ptr_1 = large_ds_buf_1;
    for ( n = 0; n < (int)large_ds_size; n++ ) {

        *ptr_1 = 0;
        ptr_1++;
    }

#if CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG /* JRM */
    HDfprintf(stdout, 
         "%s writing process slices of small ds to slices of large ds on disk.\n",
         fcnName);
#endif /* JRM */

    if ( PAR_SS_DR_MAX_RANK - large_rank == 0 ) {

        i = mpi_rank;

    } else {

        i = 0;
    }

    /* since large_rank is at most PAR_SS_DR_MAX_RANK, no need to 
     * loop over it -- either we are setting i to mpi_rank, or
     * we are setting it to zero.  It will not change during the 
     * test.
     */

    if ( PAR_SS_DR_MAX_RANK - large_rank == 1 ) {

        j = mpi_rank;

    } else {

        j = 0;
    }

    do {
        if ( PAR_SS_DR_MAX_RANK - large_rank == 2 ) {

            k = mpi_rank;

        } else {

            k = 0;
        }

        do {
            /* since small rank >= 2 and large_rank > small_rank, we 
             * have large_rank >= 3.  Since PAR_SS_DR_MAX_RANK == 5
             * (baring major re-orgaization), this gives us:
             *
             *     (PAR_SS_DR_MAX_RANK - large_rank) <= 2
             *
             * so no need to repeat the test in the outer loops --
             * just set l = 0.
             */

            l = 0;
            do {
                /* we know that small_rank >= 1 and that large_rank > small_rank
                 * by the assertions at the head of this function.  Thus no
                 * need for another inner loop.
                 */

                /* Zero out this processes slice of the on disk large data set.
                 * Note that this will leave one slice with its original data
                 * as there is one more slice than processes.
                  */
                ret = H5Dwrite(large_dataset,
                               H5T_NATIVE_UINT32,
                               large_ds_slice_sid,
                               file_large_ds_process_slice_sid,
                               xfer_plist,
                               large_ds_buf_2);
		VRFY((ret != FAIL), "H5Dwrite() to zero large ds suceeded");


                /* select the portion of the in memory large cube to which we
                 * are going to write data.
                 */
                start[0] = i;
                start[1] = j;
                start[2] = k;
                start[3] = l;
                start[4] = 0;

                ret = H5Sselect_hyperslab(file_large_ds_sid,
                                          H5S_SELECT_SET,
                                          start_ptr,
                                          stride_ptr,
                                          count_ptr,
                                          block_ptr);
		VRFY((ret != FAIL), 
                     "H5Sselect_hyperslab() target large ds slice succeeded");


                /* verify that H5S_select_shape_same() reports the in
                 * memory small data set slice selection and the
                 * on disk slice through the large data set selection
                 * as having the same shape.
                 */
                check = H5S_select_shape_same_test(mem_small_ds_sid,
                                                   file_large_ds_sid);
		VRFY((check == TRUE), "H5S_select_shape_same_test passed");


                /* write the small data set slice from memory to the 
                 * target slice of the disk data set 
                 */
#if CONTIG_HYPERSLAB_DR_PIO_TEST__RUN_TEST__DEBUG /* JRM */
                HDfprintf(stdout, "%s:%d: start = %d %d %d %d %d.\n", 
                          fcnName, mpi_rank,
                          start[0], start[1], start[2], start[3], start[4]);
                HDfprintf(stdout, "%s:%d: mem/file extent dims = %d/%d.\n",
                          fcnName, mpi_rank,
                          H5Sget_simple_extent_ndims(mem_small_ds_sid),
                          H5Sget_simple_extent_ndims(file_large_ds_sid));
#endif /* JRM */
                ret = H5Dwrite(large_dataset,
                               H5T_NATIVE_UINT32,
                               mem_small_ds_sid,
                               file_large_ds_sid,
                               xfer_plist,
                               small_ds_buf_0);
		VRFY((ret != FAIL), 
                      "H5Dwrite of small ds slice to large ds succeeded");


                /* read this processes slice on the on disk large 
                 * data set into memory.
                 */

                ret = H5Dread(large_dataset,
                              H5T_NATIVE_UINT32,
                              mem_large_ds_process_slice_sid,
                              file_large_ds_process_slice_sid,
                              xfer_plist,
                              large_ds_buf_1);
                VRFY((ret != FAIL), 
                     "H5Dread() of process slice of large ds succeeded");


                /* verify that the expected data and only the
                 * expected data was read.
                 */
                ptr_1 = large_ds_buf_1;
                expected_value = (uint32_t)(mpi_rank) * small_ds_slice_size;


                start_index = (i * edge_size * edge_size * edge_size * edge_size) +
                              (j * edge_size * edge_size * edge_size) +
                              (k * edge_size * edge_size) +
                              (l * edge_size);
                stop_index = start_index + (int)small_ds_slice_size - 1;

                HDassert( 0 <= start_index );
                HDassert( start_index < stop_index );
                HDassert( stop_index < (int)large_ds_size );

                for ( n = 0; n < (int)large_ds_size; n++ ) {

                    if ( ( n >= start_index ) && ( n <= stop_index ) ) {

                        if ( *ptr_1 != expected_value ) {

                            mis_match = TRUE;
                        }

                        expected_value++;

                    } else {

                        if ( *ptr_1 != 0 ) {

                            mis_match = TRUE;
                        }
                    }
                    /* zero out buffer for next test */
                    *ptr_1 = 0;
                    ptr_1++;
                }

                VRFY((mis_match == FALSE), 
                     "small ds slice write to large ds slice data good.");

                l++;

            } while ( ( large_rank > 2 ) &&
                      ( (small_rank - 1) <= 1 ) &&
                      ( l < edge_size ) );
            k++;
        } while ( ( large_rank > 3 ) &&
                  ( (small_rank - 1) <= 2 ) &&
                  ( k < edge_size ) );
        j++;
    } while ( ( large_rank > 4 ) &&
              ( (small_rank - 1) <= 3 ) &&
              ( j < edge_size ) );


    /* Close dataspaces */
    ret = H5Sclose(full_mem_small_ds_sid);
    VRFY((ret != FAIL), "H5Sclose(full_mem_small_ds_sid) succeeded");

    ret = H5Sclose(full_file_small_ds_sid);
    VRFY((ret != FAIL), "H5Sclose(full_file_small_ds_sid) succeeded");

    ret = H5Sclose(mem_small_ds_sid);
    VRFY((ret != FAIL), "H5Sclose(mem_small_ds_sid) succeeded");

    ret = H5Sclose(file_small_ds_sid);
    VRFY((ret != FAIL), "H5Sclose(file_small_ds_sid) succeeded");

    ret = H5Sclose(small_ds_slice_sid);
    VRFY((ret != FAIL), "H5Sclose(small_ds_slice_sid) succeeded");

    ret = H5Sclose(full_mem_large_ds_sid);
    VRFY((ret != FAIL), "H5Sclose(full_mem_large_ds_sid) succeeded");

    ret = H5Sclose(full_file_large_ds_sid);
    VRFY((ret != FAIL), "H5Sclose(full_file_large_ds_sid) succeeded");

    ret = H5Sclose(mem_large_ds_sid);
    VRFY((ret != FAIL), "H5Sclose(mem_large_ds_sid) succeeded");

    ret = H5Sclose(file_large_ds_sid);
    VRFY((ret != FAIL), "H5Sclose(mem_large_ds_sid) succeeded");

    ret = H5Sclose(mem_large_ds_process_slice_sid);
    VRFY((ret != FAIL), "H5Sclose(mem_large_ds_process_slice_sid) succeeded");

    ret = H5Sclose(file_large_ds_process_slice_sid);
    VRFY((ret != FAIL), "H5Sclose(file_large_ds_process_slice_sid) succeeded");

    ret = H5Sclose(large_ds_slice_sid);
    VRFY((ret != FAIL), "H5Sclose(large_ds_slice_sid) succeeded");


    /* Close Datasets */
    ret = H5Dclose(small_dataset);
    VRFY((ret != FAIL), "H5Dclose(small_dataset) succeeded");

    ret = H5Dclose(large_dataset);
    VRFY((ret != FAIL), "H5Dclose(large_dataset) succeeded");


    /* close the file collectively */
    MESG("about to close file.");
    ret = H5Fclose(fid);
    VRFY((ret != FAIL), "file close succeeded");

    /* Free memory buffers */
    if ( small_ds_buf_0 != NULL ) HDfree(small_ds_buf_0);
    if ( small_ds_buf_1 != NULL ) HDfree(small_ds_buf_1);
    if ( small_ds_buf_2 != NULL ) HDfree(small_ds_buf_2);
    if ( small_ds_slice_buf != NULL ) HDfree(small_ds_slice_buf);

    if ( large_ds_buf_0 != NULL ) HDfree(large_ds_buf_0);
    if ( large_ds_buf_1 != NULL ) HDfree(large_ds_buf_1);
    if ( large_ds_buf_2 != NULL ) HDfree(large_ds_buf_2);
    if ( large_ds_slice_buf != NULL ) HDfree(large_ds_slice_buf);

    return;

} /* contig_hyperslab_dr_pio_test__run_test() */


/*-------------------------------------------------------------------------
 * Function:	contig_hyperslab_dr_pio_test()
 *
 * Purpose:	Test I/O to/from hyperslab selections of different rank in
 *		the parallel case.
 *
 * Return:	void
 *
 * Programmer:	JRM -- 9/18/09
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */

void
contig_hyperslab_dr_pio_test(void)
{
    const char *fcnName = "contig_hyperslab_dr_pio_test()";
    int	        test_num = 0;
    int		edge_size = 10;
    int		chunk_edge_size = 0;
    int	        small_rank;
    int	        large_rank;
    int  	use_collective_io;
    hid_t	dset_type = H5T_STD_U32LE;

    for ( large_rank = 3; large_rank <= PAR_SS_DR_MAX_RANK; large_rank++ ) {

        for ( small_rank = 2; small_rank < large_rank; small_rank++ ) {

            for ( use_collective_io = 0; 
                  use_collective_io <= 1; 
                  use_collective_io++ ) {

                chunk_edge_size = 0;
                contig_hyperslab_dr_pio_test__run_test(test_num,
                                                       edge_size,
                                                       chunk_edge_size,
                                                       small_rank,
                                                       large_rank,
                                                       (hbool_t)use_collective_io,
                                                       dset_type);
                test_num++;

                chunk_edge_size = 5;
                contig_hyperslab_dr_pio_test__run_test(test_num,
                                                       edge_size,
                                                       chunk_edge_size,
                                                       small_rank,
                                                       large_rank,
                                                       (hbool_t)use_collective_io,
                                                       dset_type);
                test_num++;
            }
        }
    }

    return;

} /* contig_hyperslab_dr_pio_test() */
