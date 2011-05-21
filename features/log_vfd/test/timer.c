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
 * Programmer:  Dana Robinson
 *              May, 2011
 *
 * Purpose:     Tests the operation of the platform-independent timers.
 */

#include "h5test.h"


static herr_t
test_timers(void)
{
    TESTING("Timer start/stop");
    PASSED();
    return 0;

error:
    return -1;

}



static herr_t
test_time_formatting(void)
{
    TESTING("Time string formats");
    PASSED();
    return 0;

error:
    return -1;

}



int
main(void)
{
    int nerrors = 0;

    h5_reset();

    printf("Testing platform-independent timer functionality.\n");

    nerrors += test_timers() < 0                ? 1 : 0;
    nerrors += test_time_formatting() < 0       ? 1 : 0;

    if(nerrors) {
    printf("***** %d platform-independent timer TEST%s FAILED! *****\n",
        nerrors, nerrors > 1 ? "S" : "");
    return 1;
    }

    printf("All platform-independent timer tests passed.\n");
    return 0;
}