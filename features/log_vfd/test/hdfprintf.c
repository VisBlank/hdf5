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

/* Output file for HDfprintf() */
#define HDPRINTF_TESTFILE "hdfprintf_tests_out.txt"



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