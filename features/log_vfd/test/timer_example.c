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

/* This example demonstrates the proper use of HDF5 timers.  It is the
 * example code from the HDF5 timer documentation.
 *
 * This program does not look like the other tests.  We only include it
 * here to ensure that it's compiled and will break if the timer
 * API changes.
 */
 
#include <stdio.h>
#include <stdlib.h>
 
#include "hdf5.h"
#include "H5private.h"

int main(void)
{
    H5_timer_t      timer;

    H5_timevals_t   times1;
    H5_timevals_t   times2;
    H5_timevals_t   total_times;

    herr_t          err;
    
    /* Clearly, this sample code would need real error checking... */

    /* Initialization */
    err = H5_timer_init(&timer);

    /* Time a section of code */
    err = H5_timer_start(&timer);
    /* Do some work here */
    err = H5_timer_stop(&timer);
    err = H5_timer_get_times(timer, &times1);

    /* Time another section of code */
    err = H5_timer_start(&timer);
    /* Do some work here */
    err = H5_timer_stop(&timer);
    err = H5_timer_get_times(timer, &times2);

    err = H5_timer_get_total_times(timer, &total_times);

    /* Write out time statistics */
    printf("Event 1 times: (elapsed) %T    (system) %T    (user) %T\n\n",
        times1.elapsed, times1.system, times1.user);

    printf("Event 2 times: (elapsed) %T    (system) %T    (user) %T\n\n",
        times2.elapsed, times2.system, times2.user);

    printf("Total times: (elapsed) %T    (system) %T    (user) %T\n\n",
        total_times.elapsed, total_times.system, total_times.user);

    return EXIT_SUCCESS;
}

