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




/*-------------------------------------------------------------------------
 * Function:    test_timer_system_user
 *
 * Purpose:     Tests the ability to get system and user times from the
 *              timers.
 *              Some platforms may require special code to get system and
 *              user times.  If we do not support that particular platform
 *              dependent functionality, this test is skipped.
 *
 * Return:      Success:        0
 *              Failure:        -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_timer_system_user(H5_timevals_t t1, H5_timevals_t t2)
{
    TESTING("system/user times");

    /* The system and user times may not be present on some systems.  They
      * will be -1.0 if they are not.
     */
    if(t1.system_ps < 0.0 || t2.system_ps < 0.0 
        || t1.user_ps < 0.0 || t2.user_ps < 0.0) {
        SKIPPED();
        printf("NOTE: No suitable way to get system/user times on this platform.\n");
        return 0;
    }

    /* Time should not have decreased.  Depending on the resolution of the
     * timer, it may have stayed the same.
     */
    if(t2.system_ps < t1.system_ps || t2.user_ps < t1.user_ps)
        TEST_ERROR;

    PASSED();
    return 0;

error:
    return -1;
}




/*-------------------------------------------------------------------------
 * Function:    test_timer_elapsed
 *
 * Purpose:     Tests the ability to get elapsed times from the timers.
 *              We should always be able to get an elapsed time,
 *              regardless of the time libraries or platform.
 *
 * Return:      Success:        0
 *              Failure:        -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_timer_elapsed(H5_timevals_t t1, H5_timevals_t t2)
{
    TESTING("elapsed times");

    /* Elapsed time should always be present.  Elapsed time will be -1.0
     * if it is not.
     */
    if(t1.elapsed_ps < 0.0 || t2.elapsed_ps < 0.0)
        TEST_ERROR;

    /* Time should not have decreased.  Depending on the resolution of the
     * timer, it may have stayed the same.
     */
    if(t2.elapsed_ps < t1.elapsed_ps)
        TEST_ERROR;

    PASSED();
    return 0;

error:
    return -1;
}




/*-------------------------------------------------------------------------
 * Function:    test_time_formatting
 *
 * Purpose:     Tests time string creation.
 *
 * Return:      Success:        0
 *              Failure:        -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_time_formatting(void)
{
    char *s = NULL;

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
    s = H5_timer_get_time_string(123000.0);
    if(strcmp(s, "123 ns") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1e6 ns        microseconds    */
    s = H5_timer_get_time_string(23456000.0);
    if(strcmp(s, "23.5 us") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1e9 ns        milliseconds    */
    s = H5_timer_get_time_string(4567890000.0);
    if(strcmp(s, "4.6 ms") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1 min         seconds         */
    s = H5_timer_get_time_string(3.14e12);
    if(strcmp(s, "3.14 s") != 0)
        TEST_ERROR;
    free(s);

    /*      < 1 hr          mins, secs      */
    s = H5_timer_get_time_string(2.521e15);
    if(strcmp(s, "42 m 1 s") != 0)
        TEST_ERROR;
    free(s);

    /*      > 1 hr          hrs, mins, secs */
    s = H5_timer_get_time_string(9.756e15);
    if(strcmp(s, "2 h 42 m 36 s") != 0)
        TEST_ERROR;
    free(s);

    PASSED();
    return 0;

error:
    free(s);
    return -1;

}




/*-------------------------------------------------------------------------
 * Function:    main
 *
 * Purpose:     Tests the basic functionality of the platform-independent
 *              timers
 *
 * Return:      Success:        0
 *              Failure:        1
 *
 * Programmer:  Dana Robinson
 *              May, 2011
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    H5_timer_t      timer;
    H5_timevals_t   time1;
    H5_timevals_t   time2;

    char            *buf;
    int             i;

    int             nerrors = 0;

    h5_reset();

    printf("Testing platform-independent timer functionality.\n");


    /* Start a timer */
    H5_timer_start(&timer);

    /* Do some fake work and get time 1 */
    for(i=0; i < 1024; i++)
    {
        buf = (char *)HDmalloc(1024 * i * sizeof(char));
        free(buf);
    }
    H5_timer_get_times(timer, &time1);

    /* Do some fake work and get time 2 */
    for(i=0; i < 1024; i++)
    {
        buf = (char *)HDmalloc(1024 * i * sizeof(char));
        free(buf);
    }
    H5_timer_get_times(timer, &time2);

    nerrors += test_timer_system_user(time1, time2)     < 0    ? 1 : 0;
    nerrors += test_timer_elapsed(time1, time2)         < 0    ? 1 : 0;
    nerrors += test_time_formatting()                   < 0    ? 1 : 0;

    if(nerrors) {
        printf("***** %d platform-independent timer TEST%s FAILED! *****\n",
        nerrors, nerrors > 1 ? "S" : "");
        return 1;
    } else {
        printf("All platform-independent timer tests passed.\n");
        return 0;
    }
}