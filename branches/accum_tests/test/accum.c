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
herr_t test_accum_overlap(void);

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
    nerrors += test_accum_overlap();





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
    int i;

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

