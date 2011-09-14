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
#define HDPRINTF_TESTFILE "hdfprintf_tests_out.txt"

/* Size of lines read back from the output files.
 */
#define HDPRINTF_LINE_SIZE 256



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
#define N_B_LINES 9

static herr_t
test_B(void)
{
    char std[N_B_LINES][HDPRINTF_LINE_SIZE] = {
        "N/A\n",
        "0.0 B/s\n",
        "100.00 B/s\n",
        "97.66 kB/s\n",
        "95.37 MB/s\n",
        "93.13 GB/s\n",
        "90.95 TB/s\n",
        "88.82 PB/s\n",
        "86.74 EB/s\n"
    };
    char line[HDPRINTF_LINE_SIZE];
    FILE    *f      = NULL;
    int     i       = -1;
    int     err     = -1;
    char    *err_p  = NULL;

    f = fopen(HDPRINTF_TESTFILE, "w+");
    if(NULL == f)
        TEST_ERROR;

    /* Write representative bandwidths out to a file
     *
     * It's unwise to test what happens at the interval boundaries
     * due to floating point math issues.  For example, 1 MB/s
     * might print as 1000 kB/s.
     *
     * Equality to zero is ok to test due to special handling
     * in the library.
     */

    HDfprintf(f, "%B\n", -1.0);         /* < 0.0 (invalid bandwidth) */
    HDfprintf(f, "%B\n", 0.0);          /* = 0.0 */
    HDfprintf(f, "%B\n", 1.0E2);        /* < 1 kB/s */
    HDfprintf(f, "%B\n", 1.0E5);        /* < 1 MB/s */
    HDfprintf(f, "%B\n", 1.0E8);        /* < 1 GB/s */
    HDfprintf(f, "%B\n", 1.0E11);       /* < 1 TB/s */
    HDfprintf(f, "%B\n", 1.0E14);       /* < 1 PB/s */
    HDfprintf(f, "%B\n", 1.0E17);       /* < 1 EB/s */
    HDfprintf(f, "%B\n", 1.0E20);       /* > 1 EB/s */

    rewind(f);

    for(i = 0; i < N_B_LINES; i++)
    {
        err_p = fgets(line, sizeof(line), f);
        if(NULL == err_p || feof(f) || strncmp(line, std[i], sizeof(line)))
            TEST_ERROR;
    }

    /* Make sure that the output file does not have any
     * more data in it...
     */
    fgets(line, sizeof(line), f);
    if(!feof(f))
        TEST_ERROR;

    err = fclose(f);
    if(EOF == err)
        TEST_ERROR;

    err = remove(HDPRINTF_TESTFILE);
    if(EOF == err)
        TEST_ERROR;

    return 0;

error:
    fclose(f);
    remove(HDPRINTF_TESTFILE);
    return -1;

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
#define N_T_LINES 11

static herr_t
test_T(void)
{
    char std[N_T_LINES][HDPRINTF_LINE_SIZE] = {
        "N/A\n",
        "0.0 s\n",
        "100 ps\n",
        "100.0 ns\n",
        "100.0 us\n",
        "100.0 ms\n",
        "59.23 s\n",
        "59 m 59 s\n",
        "3 h 2 m 1 s\n",
        "3 h 2 m 2 s\n",
        "1 d 1 h 15 m 5 s\n"
    };
    char line[HDPRINTF_LINE_SIZE];
    FILE    *f      = NULL;
    int     i       = -1;
    int     err     = -1;
    char    *err_p  = NULL;

    f = fopen(HDPRINTF_TESTFILE, "w+");
    if(NULL == f)
        TEST_ERROR;

    /* Write representative times out to a file
     *
     * It's unwise to test what happens at the interval boundaries
     * due to floating point math issues.  For example, 1 s
     * might print as 1000 ms.
     *
     * Equality to zero is ok to test due to special handling
     * in the library.
     */

    HDfprintf(f, "%T\n", -1.0);         /* < 0.0 (invalid time) */
    HDfprintf(f, "%T\n", 0.0);          /* = 0.0 */
    HDfprintf(f, "%T\n", 1.0E-10);      /* < 1 ns (ps) */
    HDfprintf(f, "%T\n", 1.0E-7);       /* < 1 us */
    HDfprintf(f, "%T\n", 1.0E-4);       /* < 1 ms */
    HDfprintf(f, "%T\n", 1.0E-1);       /* < 1 s */
    HDfprintf(f, "%T\n", 59.23);        /* < 1 m */
    HDfprintf(f, "%T\n", 3599.456);     /* < 1 h */
    HDfprintf(f, "%T\n", 10921.476);    /* > 1 h */
    HDfprintf(f, "%T\n", 10921.876);    /* > 1 h (test rounding)*/
    HDfprintf(f, "%T\n", 90905.345);    /* >> 1 h */

    rewind(f);

    for(i = 0; i < N_T_LINES; i++)
    {
        err_p = fgets(line, sizeof(line), f);
        if(NULL == err_p || feof(f) || strncmp(line, std[i], sizeof(line)))
            TEST_ERROR;
    }

    /* Make sure that the output file does not have any
     * more data in it...
     */
    fgets(line, sizeof(line), f);
    if(!feof(f))
        TEST_ERROR;

    err = fclose(f);
    if(EOF == err)
        TEST_ERROR;

    err = remove(HDPRINTF_TESTFILE);
    if(EOF == err)
        TEST_ERROR;

    return 0;

error:
    fclose(f);
    remove(HDPRINTF_TESTFILE);
    return -1;
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

