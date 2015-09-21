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
* Test program:	 tpage_buffer
*
* Test the Page Buffer Feature.
*
*************************************************************/

#include "hdf5.h"
#include "testhdf5.h"

#include "H5PBprivate.h"
#include "H5Iprivate.h"

/*
 * This file needs to access private information from the H5F package.
 */
#define H5MF_FRIEND		/*suppress error about including H5MFpkg	  */
#include "H5MFpkg.h"

#define H5F_FRIEND		/*suppress error about including H5Fpkg	  */
#define H5F_TESTING
#include "H5Fpkg.h"


#define FILENAME_LEN		1024
#define NUM_DSETS               5
#define NX                      100
#define NY                      50

/* helper routines */
static unsigned create_file(char *filename, hid_t fcpl, hid_t fapl);
static unsigned open_file(char *filename, hid_t fapl, hsize_t page_size, size_t page_buffer_size);

/* test routines */
static unsigned test_args(hid_t fapl);
static unsigned test_raw_data_handling(hid_t orig_fapl);
static unsigned test_lru_processing(hid_t orig_fapl);
static unsigned test_min_threshold(hid_t orig_fapl);
static unsigned test_stats_collection(hid_t orig_fapl);

const char *FILENAME[] = {
    "tfilepaged",
    NULL
};

static unsigned
create_file(char *filename, hid_t fcpl, hid_t fapl)
{
    hid_t       file_id, dset_id, grp_id;
    hid_t       filespace;
    hsize_t     dimsf[2] = {NX, NY};       /* dataset dimensions */
    int         *data = NULL;                     /* pointer to data buffer to write */
    hid_t	dcpl;
    int         i, num_elements, j;
    char        dset_name[10];

    if((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl)) < 0)
        FAIL_STACK_ERROR;

    if((grp_id = H5Gcreate2(file_id, "GROUP", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR;

    num_elements  = NX * NY;
    data = (int *) HDmalloc(sizeof(int)*(size_t)num_elements);
    for (i=0; i < (int)num_elements; i++)
        data[i] = i;

    if((filespace = H5Screate_simple(2, dimsf, NULL)) < 0)
        FAIL_STACK_ERROR;

    if((dcpl = H5Pcreate(H5P_DATASET_CREATE)) < 0)
       FAIL_STACK_ERROR;
    if(H5Pset_alloc_time(dcpl, H5D_ALLOC_TIME_EARLY) < 0)
        FAIL_STACK_ERROR;

    for(i=0 ; i<NUM_DSETS; i++) {

        sprintf(dset_name, "D1dset%d", i);
        if((dset_id = H5Dcreate2(grp_id, dset_name, H5T_NATIVE_INT, filespace,
                                 H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;
        if(H5Dclose(dset_id) < 0)
            FAIL_STACK_ERROR;

        sprintf(dset_name, "D2dset%d", i);
        if((dset_id = H5Dcreate2(grp_id, dset_name, H5T_NATIVE_INT, filespace,
                                 H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;
        if(H5Dclose(dset_id) < 0)
            FAIL_STACK_ERROR;

        sprintf(dset_name, "D3dset%d", i);
        if((dset_id = H5Dcreate2(grp_id, dset_name, H5T_NATIVE_INT, filespace,
                                 H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;
        if(H5Dclose(dset_id) < 0)
            FAIL_STACK_ERROR;

        sprintf(dset_name, "dset%d", i);
        if((dset_id = H5Dcreate2(grp_id, dset_name, H5T_NATIVE_INT, filespace,
                                 H5P_DEFAULT, dcpl, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;

        if(H5Dwrite(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0)
            FAIL_STACK_ERROR;
        if(H5Dclose(dset_id) < 0)
            FAIL_STACK_ERROR;

        HDmemset(data, 0, (size_t)num_elements * sizeof(int));
        if((dset_id = H5Dopen2(grp_id, dset_name, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;        
        if(H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0)
            FAIL_STACK_ERROR;
        if(H5Dclose(dset_id) < 0)
            FAIL_STACK_ERROR;

        for (j=0; j < num_elements; j++) {
            if(data[j] != j) {
                fprintf(stderr, "Read different values than written\n");
                FAIL_STACK_ERROR;
            }
        }

        sprintf(dset_name, "D1dset%d", i);
        if(H5Ldelete(grp_id, dset_name, H5P_DEFAULT) < 0)
            FAIL_STACK_ERROR;
        sprintf(dset_name, "D2dset%d", i);
        if(H5Ldelete(grp_id, dset_name, H5P_DEFAULT) < 0)
            FAIL_STACK_ERROR;
        sprintf(dset_name, "D3dset%d", i);
        if(H5Ldelete(grp_id, dset_name, H5P_DEFAULT) < 0)
            FAIL_STACK_ERROR;
    }

    if(H5Gclose(grp_id) < 0)
        FAIL_STACK_ERROR;
    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(dcpl) < 0)
        FAIL_STACK_ERROR;
    if(H5Sclose(filespace) < 0)
        FAIL_STACK_ERROR;

    HDfree(data);
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(dcpl);
        H5Sclose(filespace);
        H5Gclose(grp_id);
        H5Fclose(file_id);
        if(data)
            HDfree(data);
    } H5E_END_TRY;
    return(1);
} /* create_file */

static unsigned
open_file(char *filename, hid_t fapl, hsize_t page_size, size_t page_buffer_size)
{
    hid_t       file_id, dset_id, grp_id;
    int         *data = NULL;                    /* pointer to data buffer to write */
    int         i, j, num_elements;
    char        dset_name[10];
    H5F_t       *f = NULL;

    if((file_id = H5Fopen(filename, H5F_ACC_RDONLY, fapl)) < 0)
        FAIL_STACK_ERROR;

    /* Get a pointer to the internal file object */
    if(NULL == (f = (H5F_t *)H5I_object(file_id)))
        FAIL_STACK_ERROR;

    if(f->shared->page_buf == NULL)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->page_size != page_size)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->max_size != page_buffer_size)
        FAIL_STACK_ERROR;

    if((grp_id = H5Gopen2(file_id, "GROUP", H5P_DEFAULT)) < 0)
        FAIL_STACK_ERROR;

    num_elements  = NX * NY;
    data = (int *) HDmalloc(sizeof(int)*(size_t)num_elements);

    for(i=0 ; i<NUM_DSETS; i++) {

        sprintf(dset_name, "dset%d", i);
        if((dset_id = H5Dopen2(grp_id, dset_name, H5P_DEFAULT)) < 0)
            FAIL_STACK_ERROR;

        if(H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0)
            FAIL_STACK_ERROR;

        if(H5Dclose(dset_id) < 0)
            FAIL_STACK_ERROR;

        for (j=0; j < num_elements; j++) {
            if(data[j] != j) {
                fprintf(stderr, "Read different values than written\n");
                FAIL_STACK_ERROR;
            }
        }
    }

    if(H5Gclose(grp_id) < 0)
        FAIL_STACK_ERROR;
    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;
    HDfree(data);

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(grp_id);
        H5Fclose(file_id);
        if(data)
            HDfree(data);
    } H5E_END_TRY;
    return(1);
}

static unsigned
test_args(hid_t orig_fapl)
{
    char filename[FILENAME_LEN]; /* Filename to use */
    hid_t file_id = -1;          /* File ID */
    hid_t fcpl, fapl;
    herr_t ret;

    TESTING("Settings for Page Buffering");

    h5_fixname(FILENAME[0], orig_fapl, filename, sizeof(filename));

    if((fapl = H5Pcopy(orig_fapl)) < 0) TEST_ERROR

    if((fcpl = H5Pcreate(H5P_FILE_CREATE)) < 0)
        TEST_ERROR;

    /* Test setting a page buffer without Paged Aggregation enabled - should fail */
    if(H5Pset_page_buffer_size(fapl, 512, 0, 0) < 0)
        TEST_ERROR;
    H5E_BEGIN_TRY {
        file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl);
    } H5E_END_TRY;
    if(file_id >= 0)
        TEST_ERROR;

    /* Test setting a page buffer with a size smaller than a single page size - should fail */
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, 100) < 0)
        TEST_ERROR;
    if(H5Pset_page_buffer_size(fapl, 99, 0, 0) < 0)
        TEST_ERROR;
    H5E_BEGIN_TRY {
        file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl);
    } H5E_END_TRY;
    if(file_id >= 0)
        TEST_ERROR;

    /* Test setting a page buffer with sum of min meta and raw data percentage > 100 - should fail */
    H5E_BEGIN_TRY {
        ret = H5Pset_page_buffer_size(fapl, 512, 50, 51);
    } H5E_END_TRY;
    if(ret >= 0)
        TEST_ERROR;

    /* Test setting a page buffer with a size equal to a single page size */
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, 100) < 0)
        TEST_ERROR;
    if(H5Pset_page_buffer_size(fapl, 100, 0, 0) < 0)
        TEST_ERROR;
    if(create_file(filename, fcpl, fapl) != 0)
        TEST_ERROR;
    if(open_file(filename, fapl, 100, 100) != 0)
        TEST_ERROR;

    /* Test setting a page buffer with a size slightly larger than a single page size */
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, 100) < 0)
        TEST_ERROR;
    if(H5Pset_page_buffer_size(fapl, 101, 0, 0) < 0)
        TEST_ERROR;
    if(create_file(filename, fcpl, fapl) != 0)
        TEST_ERROR;
    if(open_file(filename, fapl, 100, 100) != 0)
        TEST_ERROR;

    /* Test setting a large page buffer size and page size */
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, 4194304) < 0)
        TEST_ERROR;
    if(H5Pset_page_buffer_size(fapl, 16777216, 0, 0) < 0)
        TEST_ERROR;
    if(create_file(filename, fcpl, fapl) != 0)
        TEST_ERROR;
    if(open_file(filename, fapl, 4194304, 16777216) != 0)
        TEST_ERROR;

    /* Test setting a 1 byte page buffer size and page size */
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, 1) < 0)
        TEST_ERROR;
    if(H5Pset_page_buffer_size(fapl, 1, 0, 0) < 0)
        TEST_ERROR;
    if(create_file(filename, fcpl, fapl) != 0)
        TEST_ERROR;
    if(open_file(filename, fapl, 1, 1) != 0)
        TEST_ERROR;


    if(H5Pclose(fcpl) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fapl) < 0)
        FAIL_STACK_ERROR;

    PASSED()
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl);
        H5Pclose(fcpl);
    } H5E_END_TRY;
    return(1);
} /* test_args */

static unsigned
test_raw_data_handling(hid_t orig_fapl)
{
    char filename[FILENAME_LEN]; /* Filename to use */
    hid_t file_id = -1;          /* File ID */
    hid_t fcpl, fapl;
    hid_t dxpl_id = H5P_DATASET_XFER_DEFAULT;
    H5P_genplist_t *plist = (H5P_genplist_t *)H5I_object(dxpl_id);
    size_t page_count = 0;
    int i, num_elements = 1000;
    haddr_t addr;
    int *data = NULL;
    H5F_t *f = NULL;

    TESTING("Raw Data Handling");

    h5_fixname(FILENAME[0], orig_fapl, filename, sizeof(filename));

    if((fapl = H5Pcopy(orig_fapl)) < 0) TEST_ERROR

    data = (int *) HDmalloc(sizeof(int)*(size_t)num_elements);

    if((fcpl = H5Pcreate(H5P_FILE_CREATE)) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, sizeof(int)*100) < 0)
        TEST_ERROR;
    if(H5Pset_page_buffer_size(fapl, sizeof(int)*1000, 0, 0) < 0)
        TEST_ERROR;

    if((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl)) < 0)
        FAIL_STACK_ERROR;

    /* Get a pointer to the internal file object */
    if(NULL == (f = (H5F_t *)H5I_object(file_id)))
        FAIL_STACK_ERROR;

    /* allocate space for a 1000 elements */
    if(HADDR_UNDEF == (addr = H5MF_alloc(f, H5FD_MEM_DRAW, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;

    /* intialize all the elements to have a value of -1 */
    for(i=0 ; i<num_elements ; i++)
        data[i] = -1;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr, sizeof(int)*(size_t)num_elements, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    /* update the first 50 elements to have values 0-49 - this will be
       a page buffer update with 1 page resulting in the page
       buffer. */
    for(i=0 ; i<50 ; i++)
        data[i] = i;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count ++;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* update elements 150 - 225, with values 150 - 224 - this will
       bring two more pages into the page buffer. */
    for(i=0 ; i<75 ; i++)
        data[i] = i+150;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr+(sizeof(int)*150), sizeof(int)*75, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count += 2;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* update elements 50 - 150, this will go to disk but also update
       existing pages in the page buffer. */
    for(i=0 ; i<100 ; i++)
        data[i] = i+50;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr+(sizeof(int)*50), sizeof(int)*100, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* Update elements 225-300 - this will update an existing page in the PB */
    for(i=0 ; i<75 ; i++)
        data[i] = i+225;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr+(sizeof(int)*225), sizeof(int)*75, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* Do a full page write to block 300-400 - should bypass the PB */
    for(i=0 ; i<100 ; i++)
        data[i] = i+300;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr+(sizeof(int)*300), sizeof(int)*100, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* read elements 400 - 600, this should not affect the PB, and should read -1s */
    if(H5F_block_read(f, H5FD_MEM_DRAW, addr+(sizeof(int)*400), sizeof(int)*200, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    for (i=0; i < 200; i++) {
        if(data[i] != -1) {
            fprintf(stderr, "Read different values than written\n");
            FAIL_STACK_ERROR;
        }
    }
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* read elements 600 - 601, this should read -1 and bring in an entire page of addr 600 */
    if(H5F_block_read(f, H5FD_MEM_DRAW, addr+(sizeof(int)*600), sizeof(int)*1, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    for (i=0; i < 1; i++) {
        if(data[i] != -1) {
            fprintf(stderr, "Read different values than written\n");
            FAIL_STACK_ERROR;
        }
    }
    page_count ++;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* read elements 175 - 225, this should use the PB existing pages */
    if(H5F_block_read(f, H5FD_MEM_DRAW, addr+(sizeof(int)*175), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    for (i=0; i < 50; i++) {
        if(data[i] != i+175) {
            fprintf(stderr, "Read different values than written\n");
            FAIL_STACK_ERROR;
        }
    }
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* read elements 0 - 400 using the VFD.. this should result in -1s
       except for the writes that went through the PB (50-150 & 300-400) */
    if(H5FD_read(f->shared->lf, plist, H5FD_MEM_DRAW, addr, sizeof(int)*400, data) < 0)
        FAIL_STACK_ERROR;
    i = 0;
    while (i < 400) {
        if((i>=50 && i<150) || (i>=300)) {
            if(data[i] != i) {
                fprintf(stderr, "Read different values than written\n");
                FAIL_STACK_ERROR;
            }
        }
        else {
            if(data[i] != -1) {
                fprintf(stderr, "Read different values than written\n");
                FAIL_STACK_ERROR;
            }
        }
        i++;
    }

    /* read elements 0 - 400 using the PB.. this should result in all
       what we have written so far and should get the updates from the PB */
    if(H5F_block_read(f, H5FD_MEM_DRAW, addr, sizeof(int)*400, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    for (i=0; i < 400; i++) {
        if(data[i] != i) {
            fprintf(stderr, "Read different values than written\n");
            FAIL_STACK_ERROR;
        }
    }

    /* update elements 200 - 700 to value 0, this will go to disk but
       also evict existing pages from the PB (page 200 & 600 that are
       existing). */
    for(i=0 ; i<500 ; i++)
        data[i] = 0;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr+(sizeof(int)*200), sizeof(int)*500, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count -= 2;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* read elements 0 - 500.. this should go to disk then update the
       buffer result 100-200 with existing pages */
    if(H5F_block_read(f, H5FD_MEM_DRAW, addr, sizeof(int)*500, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    i=0;
    while (i < 500) {
        if(i<200) {
            if(data[i] != i) {
                fprintf(stderr, "Read different values than written\n");
                FAIL_STACK_ERROR;
            }
        }
        else {
            if(data[i] != 0) {
                fprintf(stderr, "Read different values than written\n");
                FAIL_STACK_ERROR;
            }
        }
        i++;
    }
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fcpl) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fapl) < 0)
        FAIL_STACK_ERROR;
    HDfree(data);

    PASSED()
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl);
        H5Pclose(fcpl);
        H5Fclose(file_id);
        if(data)
            HDfree(data);
    } H5E_END_TRY;
    return(1);
} /* test_raw_data_handling */

static unsigned
test_lru_processing(hid_t orig_fapl)
{
    char filename[FILENAME_LEN]; /* Filename to use */
    hid_t file_id = -1;          /* File ID */
    hid_t fcpl, fapl;
    hid_t dxpl_id = H5P_DATASET_XFER_DEFAULT;
    size_t page_count = 0;
    int i, num_elements = 1000;
    haddr_t addr, search_addr;
    int *data = NULL;
    H5F_t *f = NULL;

    TESTING("LRU Processing");

    h5_fixname(FILENAME[0], orig_fapl, filename, sizeof(filename));

    if((fapl = H5Pcopy(orig_fapl)) < 0) TEST_ERROR

    data = (int *) HDmalloc(sizeof(int)*(size_t)num_elements);

    if((fcpl = H5Pcreate(H5P_FILE_CREATE)) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, sizeof(int)*100) < 0)
        TEST_ERROR;
    /* keep 2 pages at max in the page buffer */
    if(H5Pset_page_buffer_size(fapl, sizeof(int)*200, 20, 0) < 0)
        TEST_ERROR;

    if((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl)) < 0)
        FAIL_STACK_ERROR;

    /* Get a pointer to the internal file object */
    if(NULL == (f = (H5F_t *)H5I_object(file_id)))
        FAIL_STACK_ERROR;

    /* allocate space for a 1000 elements */
    if(HADDR_UNDEF == (addr = H5MF_alloc(f, H5FD_MEM_DRAW, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;

    /* intialize all the elements to have a value of -1 */
    for(i=0 ; i<num_elements ; i++)
        data[i] = -1;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr, sizeof(int)*(size_t)num_elements, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    /* update the first 50 elements to have values 0-49 - this will be
       a page buffer update with 1 page resulting in the page
       buffer. */
    for(i=0 ; i<50 ; i++)
        data[i] = i;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count ++;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* update elements 150 - 225, with values 150 - 224 - this will
       bring two pages into the page buffer and evict 0. */
    for(i=0 ; i<75 ; i++)
        data[i] = i+150;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr+(sizeof(int)*150), sizeof(int)*75, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count = 2;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    /* The two pages should be the ones with address 100 and 200; 0
       should have been evicted */
    search_addr = addr;
    if(NULL != H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*100;
    if(NULL == H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*200;
    if(NULL == H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;

    /* update elements 150-151, this will update existing pages in the
       page buffer and move it to the top of the LRU. */
    for(i=0 ; i<1 ; i++)
        data[i] = i+150;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr+(sizeof(int)*150), sizeof(int)*1, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* read elements 600 - 601, this should read -1 and bring in an
       entire page of addr 600, and evict page 200 */
    if(H5F_block_read(f, H5FD_MEM_DRAW, addr+(sizeof(int)*600), sizeof(int)*1, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    for (i=0; i < 1; i++) {
        if(data[i] != -1) {
            fprintf(stderr, "Read different values than written\n");
            FAIL_STACK_ERROR;
        }
    }
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*200;
    if(NULL != H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*100;
    if(NULL == H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*600;
    if(NULL == H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;

    /* read elements 175 - 225, this should move 100 to the top, evict 600 and bring in 200 */
    if(H5F_block_read(f, H5FD_MEM_DRAW, addr+(sizeof(int)*175), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    for (i=0; i < 50; i++) {
        if(data[i] != i+175) {
            fprintf(stderr, "Read different values than written\n");
            FAIL_STACK_ERROR;
        }
    }
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*600;
    if(NULL != H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*100;
    if(NULL == H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*200;
    if(NULL == H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;

    /* update elements 200 - 700 to value 0, this will go to disk but
       also discarding existing pages from the PB (page 200). */
    for(i=0 ; i<500 ; i++)
        data[i] = 0;
    if(H5F_block_write(f, H5FD_MEM_DRAW, addr+(sizeof(int)*200), sizeof(int)*500, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count -= 1;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*100;
    if(NULL == H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;
    search_addr = addr + sizeof(int)*200;
    if(NULL != H5SL_search(f->shared->page_buf->slist_ptr, &(search_addr)))
        FAIL_STACK_ERROR;

    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fcpl) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fapl) < 0)
        FAIL_STACK_ERROR;
    HDfree(data);

    PASSED()
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl);
        H5Pclose(fcpl);
        H5Fclose(file_id);
        if(data)
            HDfree(data);
    } H5E_END_TRY;
    return(1);
} /* test_lru_processing */

static unsigned
test_min_threshold(hid_t orig_fapl)
{
    char filename[FILENAME_LEN]; /* Filename to use */
    hid_t file_id = -1;          /* File ID */
    hid_t fcpl, fapl;
    hid_t dxpl_id = H5P_DATASET_XFER_DEFAULT;
    size_t page_count = 0;
    int i, num_elements = 500;
    H5PB_t *page_buf;
    haddr_t meta_addr, raw_addr;
    int *data = NULL;
    H5F_t *f = NULL;

    TESTING("Minimum Metadata threshold Processing");
    printf("\n");
    h5_fixname(FILENAME[0], orig_fapl, filename, sizeof(filename));

    if((fapl = H5Pcopy(orig_fapl)) < 0) TEST_ERROR

    data = (int *) HDmalloc(sizeof(int)*(size_t)num_elements);

    if((fcpl = H5Pcreate(H5P_FILE_CREATE)) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, sizeof(int)*100) < 0)
        TEST_ERROR;

    printf("\tMinimum metadata threshould = 100%%\n");
    /* keep 5 pages at max in the page buffer and 5 meta page minimum */
    if(H5Pset_page_buffer_size(fapl, sizeof(int)*500, 100, 0) < 0)
        TEST_ERROR;
    /* create the file */
    if((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl)) < 0)
        FAIL_STACK_ERROR;
    /* Get a pointer to the internal file object */
    if(NULL == (f = (H5F_t *)H5I_object(file_id)))
        FAIL_STACK_ERROR;
    page_buf = f->shared->page_buf;

    if(page_buf->min_meta_count != 5)
        FAIL_STACK_ERROR;
    if(page_buf->min_raw_count != 0)
        FAIL_STACK_ERROR;

    if(HADDR_UNDEF == (meta_addr = H5MF_alloc(f, H5FD_MEM_SUPER, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;
    if(HADDR_UNDEF == (raw_addr = H5MF_alloc(f, H5FD_MEM_DRAW, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;

    /* write all raw data, this would end up in page buffer since there is no metadata yet*/
    for(i=0 ; i<50 ; i++)
        data[i] = i;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count += 5;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(page_buf->raw_count != 5)
        FAIL_STACK_ERROR;

    /* write all meta data, this would end up in page buffer */
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(page_buf->meta_count != 5)
        FAIL_STACK_ERROR;
    if(page_buf->raw_count != 0)
        FAIL_STACK_ERROR;

    /* write and read more raw data and make sure that they don't end up in
       page buffer since the minimum metadata is actually the entire
       page buffer */
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*50), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*175), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*250), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*375), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*450), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    if(page_buf->meta_count != 5)
        FAIL_STACK_ERROR;
    if(page_buf->raw_count != 0)
        FAIL_STACK_ERROR;

    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;


    printf("\tMinimum raw data threshould = 100%%\n");
    page_count = 0;
    /* keep 5 pages at max in the page buffer and 5 raw page minimum */
    if(H5Pset_page_buffer_size(fapl, sizeof(int)*500, 0, 100) < 0)
        TEST_ERROR;
    /* create the file */
    if((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl)) < 0)
        FAIL_STACK_ERROR;
    /* Get a pointer to the internal file object */
    if(NULL == (f = (H5F_t *)H5I_object(file_id)))
        FAIL_STACK_ERROR;
    page_buf = f->shared->page_buf;

    if(page_buf->min_meta_count != 0)
        FAIL_STACK_ERROR;
    if(page_buf->min_raw_count != 5)
        FAIL_STACK_ERROR;

    if(HADDR_UNDEF == (meta_addr = H5MF_alloc(f, H5FD_MEM_SUPER, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;
    if(HADDR_UNDEF == (raw_addr = H5MF_alloc(f, H5FD_MEM_DRAW, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;

    /* write all meta data, this would end up in page buffer since there is no raw data yet*/
    for(i=0 ; i<50 ; i++)
        data[i] = i;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count += 5;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(page_buf->meta_count != 5)
        FAIL_STACK_ERROR;

    /* write/read all raw data, this would end up in page buffer */
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(page_buf->raw_count != 5)
        FAIL_STACK_ERROR;
    if(page_buf->meta_count != 0)
        FAIL_STACK_ERROR;

    /* write and read more meta data and make sure that they don't end up in
       page buffer since the minimum metadata is actually the entire
       page buffer */
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*50), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*175), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*250), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*375), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*450), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(page_buf->raw_count != 5)
        FAIL_STACK_ERROR;
    if(page_buf->meta_count != 0)
        FAIL_STACK_ERROR;

    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;



    printf("\tMinimum metadata threshould = 20%%, Minimum rawdata threshould = 40%%\n");
    page_count = 0;
    /* keep 5 pages at max in the page buffer 2 meta pages, 2 raw pages  minimum */
    if(H5Pset_page_buffer_size(fapl, sizeof(int)*500, 40, 40) < 0)
        TEST_ERROR;
    /* create the file */
    if((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl)) < 0)
        FAIL_STACK_ERROR;
    /* Get a pointer to the internal file object */
    if(NULL == (f = (H5F_t *)H5I_object(file_id)))
        FAIL_STACK_ERROR;
    page_buf = f->shared->page_buf;

    if(page_buf->min_meta_count != 2)
        FAIL_STACK_ERROR;
    if(page_buf->min_raw_count != 2)
        FAIL_STACK_ERROR;

    if(HADDR_UNDEF == (meta_addr = H5MF_alloc(f, H5FD_MEM_SUPER, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;
    if(HADDR_UNDEF == (raw_addr = H5MF_alloc(f, H5FD_MEM_DRAW, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;

    /* intialize all the elements to have a value of -1 */
    for(i=0 ; i<num_elements ; i++)
        data[i] = -1;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*(size_t)num_elements, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*(size_t)num_elements, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    /* fill the page buffer with raw data */
    for(i=0 ; i<50 ; i++)
        data[i] = i;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count += 5;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 5)
        FAIL_STACK_ERROR;

    /* add 3 meta entries evicting 3 raw entries */
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->meta_count != 3)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 2)
        FAIL_STACK_ERROR;

    /* adding more meta entires should replace meta entries since raw data is at its minimum */
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->meta_count != 3)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 2)
        FAIL_STACK_ERROR;

    /* bring existing raw entires up the LRU */
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*375), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    /* adding 2 raw entries (even with 1 call) should only evict 1 meta entry and another raw entry */
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*175), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->meta_count != 2)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 3)
        FAIL_STACK_ERROR;

    /* adding 2 meta entries should replace 2 entires at the bottom of the LRU */
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*49), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*121), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->meta_count != 2)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 3)
        FAIL_STACK_ERROR;

    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;

    printf("\tMinimum metadata threshould = 20%%\n");
    page_count = 0;
    /* keep 5 pages at max in the page buffer and 1 meta page minimum */
    if(H5Pset_page_buffer_size(fapl, sizeof(int)*500, 39, 0) < 0)
        TEST_ERROR;
    /* create the file */
    if((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl)) < 0)
        FAIL_STACK_ERROR;
    /* Get a pointer to the internal file object */
    if(NULL == (f = (H5F_t *)H5I_object(file_id)))
        FAIL_STACK_ERROR;
    page_buf = f->shared->page_buf;

    if(page_buf->min_meta_count != 1)
        FAIL_STACK_ERROR;
    if(page_buf->min_raw_count != 0)
        FAIL_STACK_ERROR;

    if(HADDR_UNDEF == (meta_addr = H5MF_alloc(f, H5FD_MEM_SUPER, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;
    if(HADDR_UNDEF == (raw_addr = H5MF_alloc(f, H5FD_MEM_DRAW, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;

    /* intialize all the elements to have a value of -1 */
    for(i=0 ; i<num_elements ; i++)
        data[i] = -1;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*(size_t)num_elements, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*(size_t)num_elements, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    /* fill the page buffer with raw data */
    for(i=0 ; i<50 ; i++)
        data[i] = i;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    page_count += 5;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;

    /* add 2 meta entries evicting 2 raw entries */
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->meta_count != 2)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 3)
        FAIL_STACK_ERROR;

    /* bring the rest of the raw entries up the LRU */
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*250), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*350), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*450), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    /* write one more raw entry which replace one meta entry */
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*50), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->meta_count != 1)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 4)
        FAIL_STACK_ERROR;

    /* write one more raw entry which should replace another raw entry keeping min threshold of meta entries */
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*150), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->meta_count != 1)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 4)
        FAIL_STACK_ERROR;

    /* write a metadata entry that should replace the metadata entry at the bottom of the LRU */
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*250), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5SL_count(f->shared->page_buf->slist_ptr) != page_count)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->meta_count != 1)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->raw_count != 4)
        FAIL_STACK_ERROR;

    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fcpl) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fapl) < 0)
        FAIL_STACK_ERROR;
    HDfree(data);

    PASSED()
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl);
        H5Pclose(fcpl);
        H5Fclose(file_id);
        if(data)
            HDfree(data);
    } H5E_END_TRY;
    return(1);
} /* test_min_threshold */

static unsigned
test_stats_collection(hid_t orig_fapl)
{
    char filename[FILENAME_LEN]; /* Filename to use */
    hid_t file_id = -1;          /* File ID */
    hid_t fcpl, fapl;
    hid_t dxpl_id = H5P_DATASET_XFER_DEFAULT;
    int i, num_elements = 500;
    haddr_t meta_addr, raw_addr;
    int *data = NULL;
    H5F_t *f = NULL;

    TESTING("Statistics Collection");

    h5_fixname(FILENAME[0], orig_fapl, filename, sizeof(filename));

    if((fapl = H5Pcopy(orig_fapl)) < 0) TEST_ERROR

    data = (int *) HDmalloc(sizeof(int)*(size_t)num_elements);

    if((fcpl = H5Pcreate(H5P_FILE_CREATE)) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_strategy(fcpl, H5F_FSPACE_STRATEGY_PAGE, 0, (hsize_t)1) < 0)
        TEST_ERROR;
    if(H5Pset_file_space_page_size(fcpl, sizeof(int)*100) < 0)
        TEST_ERROR;

    /* keep 5 pages at max in the page buffer */
    if(H5Pset_page_buffer_size(fapl, sizeof(int)*500, 20, 0) < 0)
        TEST_ERROR;

    if((file_id = H5Fcreate(filename, H5F_ACC_TRUNC, fcpl, fapl)) < 0)
        FAIL_STACK_ERROR;

    /* Get a pointer to the internal file object */
    if(NULL == (f = (H5F_t *)H5I_object(file_id)))
        FAIL_STACK_ERROR;

    if(HADDR_UNDEF == (meta_addr = H5MF_alloc(f, H5FD_MEM_SUPER, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;
    if(HADDR_UNDEF == (raw_addr = H5MF_alloc(f, H5FD_MEM_DRAW, dxpl_id, sizeof(int)*(size_t)num_elements)))
        FAIL_STACK_ERROR;

    /* intialize all the elements to have a value of -1 */
    for(i=0 ; i<num_elements ; i++)
        data[i] = -1;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*(size_t)num_elements, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*(size_t)num_elements, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    for(i=0 ; i<100 ; i++)
        data[i] = i;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*250), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*350), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*450), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*200), sizeof(int)*100, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*50), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*150), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_write(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*400), sizeof(int)*91, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr, sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*100), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*300), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_DRAW, raw_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*200), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*300), sizeof(int)*100, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;
    if(H5F_block_read(f, H5FD_MEM_SUPER, meta_addr+(sizeof(int)*400), sizeof(int)*50, dxpl_id, data) < 0)
        FAIL_STACK_ERROR;

    //H5PB_print_stats(f->shared->page_buf);

    if(f->shared->page_buf->accesses[0] != 11)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->accesses[1] != 16)
        FAIL_STACK_ERROR;

    if(f->shared->page_buf->bypasses[0] != 3)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->bypasses[1] != 1)
        FAIL_STACK_ERROR;

    if(f->shared->page_buf->hits[0] != 0)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->hits[1] != 4)
        FAIL_STACK_ERROR;

    if(f->shared->page_buf->misses[0] != 8)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->misses[1] != 11)
        FAIL_STACK_ERROR;

    if(f->shared->page_buf->evictions[0] != 5)
        FAIL_STACK_ERROR;
    if(f->shared->page_buf->evictions[1] != 9)
        FAIL_STACK_ERROR;

    {
        int accesses[2], hits[2], misses[2], evictions[2], bypasses[2];

        H5PBget_stats(file_id, accesses, hits, misses, evictions, bypasses);

        if(accesses[0] != 11)
            FAIL_STACK_ERROR;
        if(accesses[1] != 16)
            FAIL_STACK_ERROR;
        if(bypasses[0] != 3)
            FAIL_STACK_ERROR;
        if(bypasses[1] != 1)
            FAIL_STACK_ERROR;
        if(hits[0] != 0)
            FAIL_STACK_ERROR;
        if(hits[1] != 4)
            FAIL_STACK_ERROR;
        if(misses[0] != 8)
            FAIL_STACK_ERROR;
        if(misses[1] != 11)
            FAIL_STACK_ERROR;
        if(evictions[0] != 5)
            FAIL_STACK_ERROR;
        if(evictions[1] != 9)
            FAIL_STACK_ERROR;

        H5PBreset_stats(file_id);
        H5PBget_stats(file_id, accesses, hits, misses, evictions, bypasses);

        if(accesses[0] != 0)
            FAIL_STACK_ERROR;
        if(accesses[1] != 0)
            FAIL_STACK_ERROR;
        if(bypasses[0] != 0)
            FAIL_STACK_ERROR;
        if(bypasses[1] != 0)
            FAIL_STACK_ERROR;
        if(hits[0] != 0)
            FAIL_STACK_ERROR;
        if(hits[1] != 0)
            FAIL_STACK_ERROR;
        if(misses[0] != 0)
            FAIL_STACK_ERROR;
        if(misses[1] != 0)
            FAIL_STACK_ERROR;
        if(evictions[0] != 0)
            FAIL_STACK_ERROR;
        if(evictions[1] != 0)
            FAIL_STACK_ERROR;
    }

    if(H5Fclose(file_id) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fcpl) < 0)
        FAIL_STACK_ERROR;
    if(H5Pclose(fapl) < 0)
        FAIL_STACK_ERROR;
    HDfree(data);

    PASSED()
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl);
        H5Pclose(fcpl);
        H5Fclose(file_id);
        if(data)
            HDfree(data);
    } H5E_END_TRY;
    return(1);
} /* test_stats_collection */

int
main(void)
{
    hid_t       fapl = -1;	   /* File access property list for data files */
    unsigned    nerrors = 0;       /* Cumulative error count */
    const char  *env_h5_drvr;      /* File Driver value from environment */

    /* Get the VFD to use */
    env_h5_drvr = HDgetenv("HDF5_DRIVER");
    if(env_h5_drvr == NULL)
        env_h5_drvr = "nomatch";

    if(0 == HDstrcmp(env_h5_drvr, "multi")) {
        SKIPPED();
        puts("Multi VFD doesn't support page buffering");
        return (0);
    }

    fapl = h5_fileaccess();

    nerrors += test_args(fapl);
    nerrors += test_raw_data_handling(fapl);
    nerrors += test_lru_processing(fapl);
    nerrors += test_min_threshold(fapl);
    nerrors += test_stats_collection(fapl);

    h5_clean_files(FILENAME, fapl);

    if(nerrors)
        goto error;

    puts("All Page Buffering tests passed.");

    return(0);

error:
    puts("*** TESTS FAILED ***");
    H5E_BEGIN_TRY {
        H5Pclose(fapl);
    } H5E_END_TRY;
    return(1);
} /* main() */
