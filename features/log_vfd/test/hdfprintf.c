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
 * Programmer:  Dana Robinson <derobins@hdfgroup.org>
 *              August 17, 2011
 *
 * Purpose:     Tests the operation of the new format specifiers that have
 *              been added to HDfprintf().
 *
 *              Each test opens a file, writes some formatted output to it and
 *              then reads the output back in for comparison. 
 */

#include "h5test.h"

/* Output file for HDfprintf(). Recycled for all tests.
 */
#define HDPRINTF_TESTFILE "hdfprintf_TT_tests_out.txt"



/*-------------------------------------------------------------------------
 * Function:    test_B
 *
 * Purpose:     Tests the %B format specifier
 
 *              %B converts a floating point number of bytes/second into a
 *              human-readable time string.
 *
 * Return:      Success:        0
 *              Failure:        -1
 *
 * Programmer:  Dana Robinson
 *              August 17, 2011
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_B(void)
{
    FILE *f = NULL;

    f = fopen(HDPRINTF_TESTFILE, "w+");

    /* Write representative bandwidths out to a file */

    /* < 0.0 (invalid bandwidth) */
    HDfprintf(f, "%B\n", -1.0);
    /* = 0.0 */
    HDfprintf(f, "%B\n", 0.0);
    /* < 1 kB/s */
    HDfprintf(f, "%B\n", 1.0E2);
    /* = 1 kB/s */
    HDfprintf(f, "%B\n", 1.0E3);
    /* < 1 MB/s */
    HDfprintf(f, "%B\n", 1.0E5);
    /* = 1 MB/s */
    HDfprintf(f, "%B\n", 1.0E6);
    /* < 1 GB/s */
    HDfprintf(f, "%B\n", 1.0E8);
    /* = 1 GB/s */
    HDfprintf(f, "%B\n", 1.0E9);
    /* < 1 TB/s */
    HDfprintf(f, "%B\n", 1.0E11);
    /* = 1 TB/s */
    HDfprintf(f, "%B\n", 1.0E12);
    /* < 1 PB/s */
    HDfprintf(f, "%B\n", 1.0E14);
    /* = 1 PB/s */
    HDfprintf(f, "%B\n", 1.0E15);
    /* < 1 EB/s */
    HDfprintf(f, "%B\n", 1.0E17);
    /* = 1 EB/s */
    HDfprintf(f, "%B\n", 1.0E18);
    /* > 1 EB/s */
    HDfprintf(f, "%B\n", 1.0E20);

    rewind(f);

    fclose(f);
    remove(HDPRINTF_TESTFILE);

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    test_T
 *
 * Purpose:     Tests the %T format specifier
 
 *              %T converts a floating point number of seconds into a
 *              human-readable time string.
 *
 * Return:      Success:        0
 *              Failure:        -1
 *
 * Programmer:  Dana Robinson
 *              August 17, 2011
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_T(void)
{
    FILE *f = NULL;

    f = fopen(HDPRINTF_TESTFILE, "w+");

    /* Write representative times out to a file */

    /* < 0.0 (invalid time) */
    HDfprintf(f, "%T\n", -1.0);
    /* = 0.0 */
    HDfprintf(f, "%T\n", 0.0);
    /* < 1 us */
    HDfprintf(f, "%T\n", 1.0E-7);
    /* = 1 us */
    HDfprintf(f, "%T\n", 1.0E-6);
    /* < 1 ms */
    HDfprintf(f, "%T\n", 1.0E-4);
    /* = 1 ms */
    HDfprintf(f, "%T\n", 1.0E-3);
    /* < 1 s */
    HDfprintf(f, "%T\n", 1.0);
    /* = 1 s */
    HDfprintf(f, "%T\n", 1.0);
    /* < 1 m */
    HDfprintf(f, "%T\n", 59.0);
    /* = 1 m */
    HDfprintf(f, "%T\n", 60.0);
    /* < 1 h */
    HDfprintf(f, "%T\n", 3599.0);
    /* = 1 h */
    HDfprintf(f, "%T\n", 3600.0);
    /* > 1 h */
    HDfprintf(f, "%T\n", 3601.0);

    rewind(f);

    fclose(f);
    remove(HDPRINTF_TESTFILE);

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Tests the functionality of the new format specifiers that
 *              have been added to HDfprintf().
 *
 * Return:      Success:        0
 *              Failure:        1
 *
 * Programmer:  Dana Robinson
 *              Tuesday, August 17, 2011
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    int nerrors = 0;

    h5_reset();

    printf("Testing HDfprintf() format specifiers.\n");

    nerrors += test_B() < 0 ? 1 : 0;
    nerrors += test_T() < 0 ? 1 : 0;

    if(nerrors) {
        printf("***** %d HDfprintf TEST%s FAILED! *****\n",
        nerrors, nerrors > 1 ? "S" : "");
        return 1;
    }

    printf("All HDfprintf format specifier tests passed.\n");
    return 0;
}
