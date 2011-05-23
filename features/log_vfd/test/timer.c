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
    H5_timer_t      timer;
    H5_timevals_t   time1;
    H5_timevals_t   time2;

    char *buf;
    int i;

    TESTING("Timer start/stop");

    /* Start a timer */
    H5_timer_start(&timer);

    /* Do some fake work and get time 1 */
    for(i=0; i < 8*1024; i++)
    {
        fprintf(stderr, "FOO\b\b\b");
    }
    time1 = H5_timer_get_times(timer);

    /* Do some fake work and get time 2 */
    for(i=0; i < 8*1024; i++)
    {
        buf = (char *)calloc(1024*1024, sizeof(char));
        free(buf);
    }
    time2 = H5_timer_get_times(timer);

    /* Sanity check on values */

    PASSED();
    return 0;

error:
    return -1;

}



static herr_t
test_time_formatting(void)
{
    char *s;

    TESTING("Time string formats");

    /*      < 0,            N/A             */
    s = H5_timer_get_time_string(-1.0);
    if(strcmp(s, "N/A") != 0)
        TEST_ERROR;
    free(s);

    /*      0               0               */
    s = H5_timer_get_time_string(0.0);
    if(strcmp(s, "0 ns") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1e3 ns        nanoseconds     */
    s = H5_timer_get_time_string(123.0);
    if(strcmp(s, "123 ns") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1e6 ns        microseconds    */
    s = H5_timer_get_time_string(23456.0);
    if(strcmp(s, "23.5 us") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1e9 ns        milliseconds    */
    s = H5_timer_get_time_string(4567890.0);
    if(strcmp(s, "4.6 ms") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1 min         seconds         */
    s = H5_timer_get_time_string(3.14e9);
    if(strcmp(s, "3.14 s") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1 hr          mins, secs      */
    s = H5_timer_get_time_string(2.521e12);
    if(strcmp(s, "42 m 1 s") != 0)
        TEST_ERROR;
    free(s);

    /*      > 1 hr          hrs, mins, secs */
    s = H5_timer_get_time_string(9.756e12);
    if(strcmp(s, "2 h 42 m 36 s") != 0)
        TEST_ERROR;
    free(s);

    PASSED();
    return 0;

error:
    free(s);
    return -1;

}



int
main(void)
{
    int nerrors = 0;

    h5_reset();

    printf("Testing platform-independent timer functionality.\n");

    //nerrors += test_timers() < 0                ? 1 : 0;
    nerrors += test_time_formatting() < 0       ? 1 : 0;

    if(nerrors) {
    printf("***** %d platform-independent timer TEST%s FAILED! *****\n",
        nerrors, nerrors > 1 ? "S" : "");
    return 1;
    }

    printf("All platform-independent timer tests passed.\n");
    return 0;
}