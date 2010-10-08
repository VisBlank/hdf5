/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

/* Programmer:  Mike McGreevy
 *              October 7, 2010
 */
#include "h5test.h"

#define H5F_PACKAGE
#include "H5Fpkg.h"
#include "H5Fprivate.h"
#include "H5Iprivate.h"
#include "H5FDprivate.h"

/* Filename */
#define FILENAME "accum.h5"

/* Make file global to all tests */
H5F_t * f = NULL;

/* Function Prototypes */
herr_t test_write_read(void);
herr_t test_write_read_nonacc_front(void);
herr_t test_write_read_nonacc_end(void);
herr_t test_accum_overlap(void);
herr_t test_accum_non_overlap_size(void);
herr_t test_append_resize_small_clean(void);
herr_t test_append_resize_small_dirty(void);
herr_t test_read_after(void);
herr_t test_prepend_resize_small(void);
herr_t test_prepend_resize_large(void);

/* Helper Function Prototypes */
void accum_printf(void);

/* Private Test H5Faccum Function Wrappers */
#define accum_write(a,s,b) H5F_accum_write(f, H5AC_dxpl_id, H5FD_MEM_DEFAULT,(haddr_t)a,(size_t)s,b)
#define accum_read(a,s,b)  H5F_accum_read(f, H5AC_dxpl_id, H5FD_MEM_DEFAULT,(haddr_t)a,(size_t)s,b)
#define accum_free(a,s)  H5F_accum_free(f, H5AC_dxpl_id, H5FD_MEM_DEFAULT, (haddr_t)a, (size_t)s)
#define accum_flush()    H5F_accum_flush(f, H5AC_dxpl_id)
#define accum_reset()    H5F_accum_reset(f, H5AC_dxpl_id)

/* ================= */
/* Main Test Routine */
/* ================= */


/*-------------------------------------------------------------------------
 * Function:    main
 * 
 * Purpose:     Test the metadata accumulator code
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Mike McGreevy
 *              October 7, 2010
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    unsigned nerrors = 0;        /* track errors */
    hid_t fid = -1;

    /* ========== */
    /* Test Setup */
    /* ========== */
    
    puts("Testing the metadata accumulator");

    /* Create a test file */
    if ((fid = H5Fcreate(FILENAME, 
                         H5F_ACC_TRUNC, 
                         H5P_DEFAULT, 
                         H5P_DEFAULT)) < 0) TEST_ERROR;

    /* Get H5F_t * to internal file structure */
    if ((f = (H5F_t *)H5I_object(fid)) == NULL) TEST_ERROR;

    /* We'll be writing lots of garbage data, so extend the
        file a ways. 10MB should do. */
    H5FD_set_eoa(f->shared->lf, H5AC_dxpl_id, (haddr_t)(1024*1024*10));

    /* Reset metadata accumulator for the file */
    accum_reset();

    /* ============== */
    /* Test Functions */
    /* ============== */

    /* add more test functions to this list! do it! */
    nerrors += test_write_read();
    nerrors += test_write_read_nonacc_front();
    nerrors += test_write_read_nonacc_end();
    nerrors += test_accum_overlap();
    nerrors += test_accum_overlap_clean();
    nerrors += test_accum_non_overlap_size();
    nerrors += test_append_resize_small_clean();
    nerrors += test_append_resize_small_dirty();
    nerrors += test_prepend_resize_small();
    nerrors += test_prepend_resize_large();
    nerrors += test_read_after();
    nerrors += test_free();

    /* End of test code, close and delete file */
    H5Fclose(fid);
    HDremove(FILENAME);

    if(nerrors)
        goto error;
    puts("All metadata accumulator tests passed.");
    return 0;
error: 
    puts("*** TESTS FAILED ***");
    return 1;
} /* end main() */

/* ============================= */
/* Individual Unit Test Routines */
/* ============================= */


/*-------------------------------------------------------------------------
 * Function:    test_write_read
 * 
 * Purpose:     Simple test to write to then read from metadata accumulator.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Mike McGreevy
 *              October 7, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t test_write_read(void)
{
    int i = 0;
    int write_buf[1024], read_buf[1024];

    TESTING("simple write/read to/from metadata accumulator");

    /* Fill buffer with data, zero out read buffer */
    for(i=0;i<1024;i++) write_buf[i]=i+1;
    for(i=0;i<1024;i++) read_buf[i]=0;

    /* Do a simple write/read/verify of data */
    /* Write 1KB at Address 0 */
    if (accum_write(0,1024,write_buf) < 0) TEST_ERROR;
    if (accum_read(0,1024,read_buf) < 0) TEST_ERROR;
    if (memcmp(write_buf,read_buf,1024) != 0 ) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_write_read */ 


/*-------------------------------------------------------------------------
 * Function:    test_write_read_nonacc_front
 * 
 * Purpose:     Simple test to write to then read from before metadata accumulator.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Allen Byrne
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t test_write_read_nonacc_front(void)
{
    int i = 0;
    int write_buf[2048], read_buf[2024];

    TESTING("simple write/read to/from before metadata accumulator");

    /* Fill buffer with data, zero out read buffer */
    for(i=0;i<2048;i++) write_buf[i]=i+1;
    for(i=0;i<1024;i++) read_buf[i]=0;
    
    /* Do a simple write/read/verify of data */
    /* Write 1KB at Address 0 */
    if (accum_write(0,1024,write_buf) < 0) TEST_ERROR;
    if (accum_flush() < 0) TEST_ERROR;
    accum_reset();
    if (accum_write(1024,1024,write_buf) < 0) TEST_ERROR;
    if (accum_read(0,1024,read_buf) < 0) TEST_ERROR;
    if (memcmp(write_buf,read_buf,1024) != 0 ) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_write_read */ 


/*-------------------------------------------------------------------------
 * Function:    test_write_read_nonacc_end
 * 
 * Purpose:     Simple test to write to then read from after metadata accumulator.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Allen Byrne
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t test_write_read_nonacc_end(void)
{
    int i = 0;
    int write_buf[2048], read_buf[2024];

    TESTING("simple write/read to/from after metadata accumulator");

    /* Fill buffer with data, zero out read buffer */
    for(i=0;i<2048;i++) write_buf[i]=i+1;
    for(i=0;i<1024;i++) read_buf[i]=0;
    
    /* Do a simple write/read/verify of data */
    /* Write 1KB at Address 0 */
    if (accum_write(1024,1024,write_buf) < 0) TEST_ERROR;
    if (accum_flush() < 0) TEST_ERROR;
    accum_reset();
    if (accum_write(0,1024,write_buf) < 0) TEST_ERROR;
    if (accum_read(1024,1024,read_buf) < 0) TEST_ERROR;
    if (memcmp(write_buf,read_buf,1024) != 0 ) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_write_read */ 


/*-------------------------------------------------------------------------
 * Function:    test_free
 * 
 * Purpose:     Simple test to free metadata accumulator.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Raymond Lu
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t test_free(void)
{
    int i = 0;
    int *write_buf = NULL;

    TESTING("simple freeing metadata accumulator");

    /* Write and free the whole accumulator. */ 
    write_buf = (int*)malloc(256*1024*sizeof(int));

    /* Fill buffer with data */
    for(i=0;i<256*1024;i++) write_buf[i]=i+1;
    
    if (accum_write(0,256*1024*sizeof(int),write_buf) < 0) TEST_ERROR;

    if (accum_free(0,256*1024*sizeof(int)) < 0) TEST_ERROR;

    /* Free an empty accumulator */
    if (accum_free(0,256*1024*sizeof(int)) < 0) TEST_ERROR;

    /* Write second quarter of the accumulator */
    if (accum_write(64*1024*sizeof(int),128*1024*sizeof(int),write_buf) < 0) 
        TEST_ERROR;

    /* Free the second quarter of the accumulator, the requested area 
     * is bigger than the data region on the right side. */
    if (accum_free(64*1024*sizeof(int),65*1024*sizeof(int)) < 0) TEST_ERROR;




    /* Write half of the accumulator. */ 
    if (accum_write(0,128*1024*sizeof(int),write_buf) < 0) TEST_ERROR;

    /* Free the first block of 4KB */
    if (accum_free(0,1024*sizeof(int)) < 0) TEST_ERROR;

    /* Free the block of 4KB at 127*4KB */
    if (accum_free(127*1024*sizeof(int),1024*sizeof(int)) < 0) TEST_ERROR;

    /* Free the block of 4KB at 2*4KB */
    if (accum_free(2*1024*sizeof(int),1024*sizeof(int)) < 0) TEST_ERROR;



    free(write_buf);

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_write_read */ 



/*-------------------------------------------------------------------------
 * Function:    test_accum_overlap
 * 
 * Purpose:     This test will write a series of pieces of data
 *              to the accumulator with the goal of overlapping
 *              the writes in various different ways.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Mike McGreevy
 *              October 7, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t test_accum_overlap(void)
{
    int i = 0;
    int32_t wbuf[4096], rbuf[4096];

    /* Zero out read buffer */
    for(i=0;i<4096;i++) rbuf[i]=0;

    TESTING("overlapping write to metadata accumulator");

    /* Case 1: No metadata in accumulator */
    /* Write 10 1's at address 40 */
    /* @0:|          1111111111| */
    /* Put some data in the accumulator initially */
    for(i=0;i<10;i++) wbuf[i]=1;
    if (accum_write(40,10*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(40,10*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,10*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 2: End of new piece aligns with start of accumulated data */
    /* Write 5 2's at address 20 */
    /* @0:|     222221111111111| */
    for(i=0;i<5;i++) wbuf[i]=2;
    if (accum_write(20,5*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(20,5*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,5*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 3: Start of new piece aligns with start of accumulated data */
    /* Write 3 3's at address 20 */
    /* @0:|     333221111111111| */
    for(i=0;i<3;i++) wbuf[i]=3;
    if (accum_write(20,3*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(20,3*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,3*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 4: New piece overlaps start of accumulated data */
    /* Write 5 4's at address 8 */
    /* @0:|  444443221111111111| */
    for(i=0;i<5;i++) wbuf[i]=4;
    if (accum_write(8,5*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(8,5*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,5*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 5: New piece completely within accumulated data */
    /* Write 4 5's at address 48 */
    /* @0:|  444443221155551111| */
    for(i=0;i<4;i++) wbuf[i]=5;
    if (accum_write(48,4*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(48,4*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,4*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 6: End of new piece aligns with end of accumulated data */
    /* Write 3 6's at address 68 */
    /* @0:|  444443221155551666| */
    for(i=0;i<3;i++) wbuf[i]=6;
    if (accum_write(68,3*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(68,3*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,3*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 7: New piece overlaps end of accumulated data */
    /* Write 5 7's at address 76 */
    /* @0:|  4444432211555516677777| */
    for(i=0;i<5;i++) wbuf[i]=7;
    if (accum_write(76,5*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(76,5*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,5*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 8: Start of new piece aligns with end of accumulated data */
    /* Write 3 8's at address 96 */
    /* @0:|  4444432211555516677777888| */
    for(i=0;i<3;i++) wbuf[i]=8;
    if (accum_write(96,3*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(96,3*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,3*sizeof(int32_t)) != 0 ) TEST_ERROR;
 
    /* Set up expected data buffer and verify contents of
        accumulator as constructed by cases 1-8, above */    
    for(i=0;i<5;i++) wbuf[i]=4;
    for(i=5;i<6;i++) wbuf[i]=3;
    for(i=6;i<8;i++) wbuf[i]=2;
    for(i=8;i<10;i++) wbuf[i]=1;
    for(i=10;i<14;i++) wbuf[i]=5;
    for(i=14;i<15;i++) wbuf[i]=1;
    for(i=15;i<17;i++) wbuf[i]=6;
    for(i=17;i<22;i++) wbuf[i]=7;
    for(i=22;i<25;i++) wbuf[i]=8;
    if (accum_read(8,25*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,25*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 9: New piece completely before accumulated data */
    /* Write 1 9 at address 0 */
    /* @0:|9 4444432211555516677777888| */
    for(i=0;i<1;i++) wbuf[i]=9;
    if (accum_write(0,1*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(0,1*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,1*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 10: New piece completely after accumulated data */
    /* Write 4 3's at address 116 */
    /* @0:|9 4444432211555516677777888  3333| */
    for(i=0;i<4;i++) wbuf[i]=3;
    if (accum_write(116,4*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(116,4*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,4*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 11: New piece completely overlaps accumulated data */
    /* Write 6 4's at address 112 */
    /* @0:|9 4444432211555516677777888 444444| */
    for(i=0;i<6;i++) wbuf[i]=4;
    if (accum_write(112,6*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(112,6*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,6*sizeof(int32_t)) != 0 ) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_accum_overlap */


/*-------------------------------------------------------------------------
 * Function:    test_accum_overlap_clean
 *
 * Purpose:     This test will write a series of pieces of data
 *              to the accumulator with the goal of overlapping
 *              the writes in various different ways, with clean
 *              areas in the accumulator.
 *
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 *
 * Programmer:  Neil Fortner
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t test_accum_overlap_clean(void)
{
    int i = 0;
    int32_t wbuf[4096], rbuf[4096];

    /* Zero out read buffer */
    for(i=0;i<4096;i++) rbuf[i]=0;

    TESTING("overlapping write to partially clean metadata accumulator");

    /* Case 1: No metadata in accumulator */
    /* Write 10 1's at address 40 */
    /* @0:|          1111111111| */
    /* Put some data in the accumulator initially */
    for(i=0;i<10;i++) wbuf[i]=1;
    if (accum_write(40,10*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(40,10*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,10*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 2: End of new piece aligns with start of clean accumulated data */
    /* Write 5 2's at address 20 */
    /* @0:|     222221111111111| */
    if(accum_flush() < 0) TEST_ERROR;
    for(i=0;i<5;i++) wbuf[i]=2;
    if (accum_write(20,5*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(20,5*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,5*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 3: Start of new piece aligns with start of accumulated data,
     * completely encloses dirty section of accumulator */
    /* Write 6 3's at address 20 */
    /* @0:|  333333111111111| */
    for(i=0;i<6;i++) wbuf[i]=3;
    if (accum_write(20,6*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(20,6*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,6*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 4: New piece completely within accumulated data, overlaps
     * end of dirty section of accumulator */
    /* Write 2 4's at address 40 */
    /* @0:|  333334411111111| */
    for(i=0;i<2;i++) wbuf[i]=4;
    if (accum_write(40,2*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(40,2*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,2*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 5: New piece completely within accumulated data, completely
     * after dirty section of accumulator */
    /* Write 2 5's at address 52 */
    /* @0:|  333334415511111| */
    for(i=0;i<2;i++) wbuf[i]=5;
    if (accum_write(52,2*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(52,2*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,2*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 6: New piece completely within clean accumulated data */
    /* Write 3 6's at address 44 */
    /* @0:|  333334666511111| */
    if(accum_flush() < 0) TEST_ERROR;
    for(i=0;i<3;i++) wbuf[i]=6;
    if (accum_write(44,3*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(44,3*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,3*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 7: New piece overlaps start of clean accumulated data */
    /* Write 2 7's at address 16 */
    /* @0:|  7733334666511111| */
    if(accum_flush() < 0) TEST_ERROR;
    for(i=0;i<2;i++) wbuf[i]=7;
    if (accum_write(16,2*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(16,2*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,2*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 8: New piece overlaps start of accumulated data, completely
     * encloses dirty section of accumulator */
    /* Write 4 8's at address 12 */
    /* @0:|  88883334666511111| */
    for(i=0;i<4;i++) wbuf[i]=8;
    if (accum_write(12,4*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(12,4*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,4*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 9: Start of new piece aligns with end of clean accumulated data */
    /* Write 3 9's at address 80 */
    /* @0:|  88883334666511111999| */
    if(accum_flush() < 0) TEST_ERROR;
    for(i=0;i<3;i++) wbuf[i]=9;
    if (accum_write(80,3*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(80,3*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,3*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 10: New piece overlaps end of clean accumulated data */
    /* Write 3 2's at address 88 */
    /* @0:|  888833346665111119922| */
    if(accum_flush() < 0) TEST_ERROR;
    for(i=0;i<2;i++) wbuf[i]=2;
    if (accum_write(88,2*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(88,2*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,2*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 11: New piece overlaps end of accumulated data, completely encloses
     * dirty section of accumulator */
    /* Write 4 7's at address 84 */
    /* @0:|  8888333466651111197777| */
    for(i=0;i<4;i++) wbuf[i]=7;
    if (accum_write(84,4*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(84,4*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,4*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Set up expected data buffer and verify contents of
        accumulator as constructed by cases 1-11, above */
    for(i=0;i<4;i++) wbuf[i]=8;
    for(i=4;i<7;i++) wbuf[i]=3;
    for(i=7;i<8;i++) wbuf[i]=4;
    for(i=8;i<11;i++) wbuf[i]=6;
    for(i=11;i<12;i++) wbuf[i]=5;
    for(i=12;i<17;i++) wbuf[i]=1;
    for(i=17;i<18;i++) wbuf[i]=9;
    for(i=18;i<22;i++) wbuf[i]=7;
    if (accum_read(12,22*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,22*sizeof(int32_t)) != 0 ) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_accum_overlap_clean */


/*-------------------------------------------------------------------------
 * Function:    test_accum_non_overlap
 * 
 * Purpose:     This test will write a series of pieces of data
 *              to the accumulator with the goal of not overlapping
 *              the writes with a data size larger then the accum size.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Allen Byrne
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t test_accum_non_overlap_size(void)
{
    int i = 0;
    int32_t wbuf[4096], rbuf[4096];

    /* Zero out read buffer */
    for(i=0;i<4096;i++) rbuf[i]=0;

    TESTING("non-overlapping write to accumulator larger then accum_size");

    /* Case 1: No metadata in accumulator */
    /* Write 10 1's at address 140 */
    /* @0:|     1111111111| */
    /* Put some data in the accumulator initially */
    for(i=0;i<10;i++) wbuf[i]=1;
    if (accum_write(140,10*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(140,10*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,10*sizeof(int32_t)) != 0 ) TEST_ERROR;

    /* Case 9: New piece completely before accumulated data */
    /* Write 20 9 at address 0 */
    /* @0:|9   1111111111| */
    for(i=0;i<20;i++) wbuf[i]=9;
    if (accum_write(0,20*sizeof(int32_t),wbuf) < 0) TEST_ERROR;
    if (accum_read(0,20*sizeof(int32_t),rbuf) < 0) TEST_ERROR;
    if (memcmp(wbuf,rbuf,20*sizeof(int32_t)) != 0 ) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_accum_non_overlap */


/*-------------------------------------------------------------------------
 * Function:    test_prepend_resize_small
 * 
 * Purpose:     This test sets up the case where there is a lot of dirty 
 *              metadata in the accumulator, and a new piece of small metadata 
 *              is added that prepends to the existing data. The prepend results
 *              in the accumulator going into the resize mechanism, but not
 *              reducing the size of the accumulator.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Mike McGreevy
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t 
test_prepend_resize_small(void)
{
    int i = 0;
    int s = 1048576;    /* size of buffer */
    int32_t wbuf[s], rbuf[s];

    /* Zero out read buffer */
    for(i=0;i<s;i++) rbuf[i]=0;

    /* Fill up write buffer */
    for(i=0;i<s;i++) wbuf[i]=i+1;

    TESTING("prepending small entry to accumulator to force resize");

    /* Write data to the accumulator to fill it just under max size (but not full) */
    if(accum_write(1048576,1048575,wbuf)<0) TEST_ERROR;

    /* Write a small piece of data to accumulator that 
        will append to the front of it, and force a resize. */
    if(accum_write(1048571,5,wbuf)<0) TEST_ERROR;

    /* Read back and verify both pieces of data */
    if(accum_read(1048576,1048575,rbuf)<0) TEST_ERROR;
    if(memcmp(wbuf,rbuf,1048576)!=0) TEST_ERROR;

    if(accum_read(1048571,5,rbuf)<0) TEST_ERROR;
    if(memcmp(wbuf,rbuf,5)!=0) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_prepend_resize_small */ 


/*-------------------------------------------------------------------------
 * Function:    test_prepend_resize_large
 * 
 * Purpose:     This test sets up the case where there is a lot of dirty 
 *              metadata in the accumulator, and a new piece of large metadata 
 *              is added that prepends to the existing data. The prepend results
 *              in the accumulator going into the resize mechanism, but not
 *              resizing because the new entry is large.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Mike McGreevy
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t 
test_prepend_resize_large(void)
{
    int i = 0;
    int s = 1048576;    /* size of buffer */
    int32_t wbuf[s], rbuf[s];

    /* Zero out read buffer */
    for(i=0;i<s;i++) rbuf[i]=0;

    /* Fill up write buffer */
    for(i=0;i<s;i++) wbuf[i]=i+1;

    TESTING("prepending large entry to accumulator to force resize");

    /* Write data to the accumulator to fill it just under max size (but not full) */
    if(accum_write(1048576,1048575,wbuf)<0) TEST_ERROR;

    /* Write a large piece of data to accumulator that 
        will append to the front of it, and force a resize. */
    if(accum_write(5,1048571,wbuf)<0) TEST_ERROR;

    /* Read back and verify both pieces of data */
    if(accum_read(1048576,1048575,rbuf)<0) TEST_ERROR;
    if(memcmp(wbuf,rbuf,1048576)!=0) TEST_ERROR;

    if(accum_read(5,1048571,rbuf)<0) TEST_ERROR;
    if(memcmp(wbuf,rbuf,1048571)!=0) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_prepend_resize_large */ 


/*-------------------------------------------------------------------------
 * Function:    test_append_resize_small_clean
 * 
 * Purpose:     This test will verify the case when the metadata accumulator
 *              contains a block of clean metadata. A new piece is added that
 *              aligns with the end of the data (and accumulator) such that
 *              the accumulator needs to resize to account for it. After doing
 *              the resize, the accumulator has grown too large, so it shrinks
 *              in order to stay within the maximum size limit.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Mike McGreevy
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t 
test_append_resize_small_clean(void)
{
    int i = 0;
    int s = 1048576;    /* size of buffer */
    int32_t wbuf[s], rbuf[s];

    /* Zero out read buffer */
    for(i=0;i<s;i++) rbuf[i]=0;

    /* Fill up write buffer */
    for(i=0;i<s;i++) wbuf[i]=i+1;

    TESTING("appending small entry to clean accumulator to force resize");

    /* Write data to the accumulator to fill it just under max size (but not full) */
    if(accum_write(0,1048571,wbuf)<0) TEST_ERROR;

    /* Flush the accumulator -- we want to test the case when
        accumulator contains clean data */
    accum_flush();

    /* Write a new (small) piece of data that forces a resize of the accumulator for the smaller */
    if(accum_write(1048571,10,wbuf)<0) TEST_ERROR;

    /* Write a piece of metadata outside current accumulator to force write
        to disk */
    if(accum_write(0,1,wbuf)<0) TEST_ERROR;

    /* Read in the piece we wrote to disk above, and then verify that 
        the data is as expected */
    if(accum_read(1048571,10,rbuf)<0) TEST_ERROR;
    if(memcmp(wbuf,rbuf,10)!=0) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_append_resize_small_clean */ 


/*-------------------------------------------------------------------------
 * Function:    test_append_resize_small_dirty
 * 
 * Purpose:     This test will verify the case when the metadata accumulator
 *              contains a block of metadata, some of which is dirty, and a 
 *              new piece is added that aligns with the end of the data such that
 *              the accumulator needs to resize to account for it. After doing
 *              the resize, the accumulator has grown too large, so it shrinks
 *              in order to stay within the maximum size limit, and slides the
 *              dirty region down while doing so.
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Mike McGreevy
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t 
test_append_resize_small_dirty(void)
{
    int i = 0;
    int s = 1048576;    /* size of buffer */
    int32_t wbuf[s], rbuf[s];

    /* Zero out read buffer */
    for(i=0;i<s;i++) rbuf[i]=0;

    /* Fill up write buffer */
    for(i=0;i<s;i++) wbuf[i]=i+1;

    TESTING("appending small entry to dirty accumulator to force resize");

    /* Case 1 - dirty region aligns with new metadata */
    /* Write data to the accumulator to fill it just under max size (but not full) */
    if(accum_write(0,1048571,wbuf)<0) TEST_ERROR;

    /* Flush the accumulator to clean it */
    accum_flush();

    /* write to part of the accumulator so it's dirty, but not entirely dirty */
    if(accum_write(2,1048569,wbuf)<0) TEST_ERROR;

    /* Write a new (small) piece of data that forces a resize of the accumulator for the smaller */
    if(accum_write(1048571,10,wbuf)<0) TEST_ERROR;

    /* Write a piece of metadata outside current accumulator to force write
        to disk */
    if(accum_write(0,1,wbuf)<0) TEST_ERROR;

    /* Read in the piece we wrote to disk above, and then verify that 
        the data is as expected */
    if(accum_read(1048571,10,rbuf)<0) TEST_ERROR;
    if(memcmp(wbuf,rbuf,10)!=0) TEST_ERROR;
    accum_reset();

    /* Case 2 - dirty region does not align with dirty metadata */
    accum_reset();
    /* Write data to the accumulator to fill it just under max size (but not full) */
    if(accum_write(0,1048571,wbuf)<0) TEST_ERROR;

    /* Flush the accumulator to clean it */
    accum_flush();

    /* write to part of the accumulator so just the start is dirty */
    if(accum_write(0,5,wbuf)<0) TEST_ERROR;

    /* Write a new (small) piece of data that forces a resize of the accumulator for the smaller */
    if(accum_write(1048571,349523,wbuf)<0) TEST_ERROR;

    /* Write a piece of metadata outside current accumulator to force write
        to disk */
    if(accum_write(1398900,1,wbuf)<0) TEST_ERROR;

    /* Read in the piece we wrote to disk above, and then verify that 
        the data is as expected */
    if(accum_read(1048571,349523,rbuf)<0) TEST_ERROR;
    if(memcmp(wbuf,rbuf,349523)!=0) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* test_append_resize_small_dirty */ 


/*-------------------------------------------------------------------------
 * Function:    test_read_after
 * 
 * Purpose:     This test will verify the case when metadata is read partly 
 *              from the accumulator and partly from disk.  The test will 
 *              write a block of data at address 512, force the data to be
 *              written to disk, write new data partially overlapping the 
 *              original block from below, then read data at address 512.  
 *              The data read should be partly new and partly original. 
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Larry Knox
 *              October 8, 2010
 *
 *-------------------------------------------------------------------------
 */
herr_t 
test_read_after(void)
{
    int i = 0;
    int s = 128;    /* size of buffer */
    int32_t wbuf[s], rbuf[s];

    /* Zero out read buffer */
    for(i=0;i<s;i++) rbuf[i]=0;

    /* Fill up write buffer with 1s */
    for(i=0;i<s;i++) wbuf[i]=1;

    TESTING("reading data partially from both accumulator and disk");
    
    /* Write data to the accumulator to fill it. */
    if(accum_write(512,512,wbuf)<0) TEST_ERROR;

    /* Write a piece of metadata outside current accumulator to force write
        to disk */
    if(accum_write(0,1,wbuf)<0) TEST_ERROR;

    /* Fill up write buffer with 2s */
    for(i=0;i<s;i++) wbuf[i]=2;
     
    /* Write a block of 2s of the original size that will overlap the lower half
        of the original block */
    if(accum_write(256,512,wbuf)<0) TEST_ERROR;

    /* Read 128 bytes at the original address, and then  */ 
    if(accum_read(512,512,rbuf)<0) TEST_ERROR;

    /* Set the second half of wbuf back to 1s */ 
    for(i=64;i<s;i++) wbuf[i]=1;

    /* Read in the piece we wrote to disk above, and then verify that 
        the data is as expected */
    if(accum_read(512,512,rbuf)<0) TEST_ERROR;
    if(memcmp(wbuf,rbuf,128)!=0) TEST_ERROR;

    PASSED();
    accum_reset();
    return 0;
error:
    return 1;
} /* end test_read_after */

/*-------------------------------------------------------------------------
 * Function:    accum_printf
 * 
 * Purpose:     Debug function to print some stats about the accumulator
 * 
 * Return:      Success: SUCCEED
 *              Failure: FAIL
 * 
 * Programmer:  Mike McGreevy
 *              October 7, 2010
 *
 *-------------------------------------------------------------------------
 */
void
accum_printf(void)
{
    H5F_meta_accum_t * accum = &f->shared->accum;

    printf("\n");
    printf("Current contents of accumulator:\n");
    if (accum->alloc_size == 0) {
        printf("=====================================================\n");
        printf(" No accumulator allocated.\n");
        printf("=====================================================\n");
    } else {
        printf("=====================================================\n");
        printf(" accumulator allocated size == %lu\n", (unsigned long)accum->alloc_size);
        printf(" accumulated data size      == %lu\n", (unsigned long)accum->size);
        printf(" accumulator dirty?         == %d\n", accum->dirty);
        printf("=====================================================\n");
        printf(" start of accumulated data, loc = %llu\n", accum->loc);
        if (accum->dirty) printf(" start of dirty region, loc = %llu\n", accum->loc + accum->dirty_off);
        if (accum->dirty) printf(" end of dirty region,   loc = %llu\n", accum->loc + accum->dirty_off + accum->dirty_len);
        printf(" end of accumulated data,   loc = %llu\n", accum->loc + accum->size);
        printf(" end of accumulator allocation,   loc = %llu\n", accum->loc + accum->alloc_size);
        printf("=====================================================\n");
    }
    printf("\n\n");
} /* accum_printf() */

