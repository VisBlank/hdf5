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

/*-------------------------------------------------------------------------
 * Created: H5timer.c
 *          Aug 21 2006
 *          Quincey Koziol <koziol@hdfgroup.org>
 *
 * Purpose: Internal, platform-independent 'timer' support routines.
 *-------------------------------------------------------------------------
 */

/****************/
/* Module Setup */
/****************/


/***********/
/* Headers */
/***********/
#include "H5private.h"		/* Generic Functions			*/


/****************/
/* Local Macros */
/****************/


/******************/
/* Local Typedefs */
/******************/


/********************/
/* Package Typedefs */
/********************/


/********************/
/* Local Prototypes */
/********************/


/*********************/
/* Package Variables */
/*********************/


/*****************************/
/* Library Private Variables */
/*****************************/


/*******************/
/* Local Variables */
/*******************/


/*-------------------------------------------------------------------------
 * Function:    _timer_get_timevals
 *
 * Purpose:     Internal platform-specific function to get time system,
 *              user and elapsed time values.
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
static herr_t
_timer_get_timevals(H5_timevals_t *times /*in,out*/)
{
    int err = 0;        /* Error code */

    /* System and user time */
#if defined(H5_HAVE_GETRUSAGE)
    struct rusage res;
#endif

    /* elapsed time */
#if defined(H5_HAVE_CLOCK_GETTIME)
    struct timespec ts;
#endif

    assert(times);


    /* Windows call handles both system/user and elapsed times */
#if defined(_WIN32)
    err = H5_get_win32_times(times);
    if(err < 0) {
        times->elapsed   = -1;
        times->system    = -1;
        times->user      = -1;
        return -1;
    }
    else {
        return 0;
    }
#endif


    /*************************
     * System and user times *
     *************************/

#if defined(H5_HAVE_GETRUSAGE)

    err = getrusage(RUSAGE_SELF, &res);
    if(err < 0)
        return -1;
    times->system = (double)res.ru_stime.tv_sec + ((double)res.ru_stime.tv_usec / 1.0E6F);
    times->user   = (double)res.ru_utime.tv_sec + ((double)res.ru_utime.tv_usec / 1.0E6F);

#else

    /* No suitable way to get system/user times */
    /* This is not an error condition, they just won't be available */
    times->system = -1.0;
    times->user   = -1.0;

#endif


    /****************
     * Elapsed time *
     ****************/

    /* NOTE: Not having a way to get elapsed time IS an error, unlike
     * the system and user times.
     */

#if defined(H5_HAVE_MACH_MACH_TIME_H)

    /* Mac OS X */
    times->elapsed = H5_get_mach_time_seconds();
    if(times->elapsed < 0.0)
        return -1;

#elif defined(H5_HAVE_CLOCK_GETTIME)

    err = clock_gettime(CLOCK_MONOTONIC, &ts);
    if(err != 0)
        return -1;
    times->elapsed = (double)ts.tv_sec + ((double)ts.tv_nsec / 1.0E9F);

#else

    /* Die here.  We'd like to know about this so we can support some
     * other time functionality.
     */
    assert(0);

#endif

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    H5_timer_init
 *
 * Purpose:     Initialize a platform-independent timer.
 *
 *              Timer usage is as follows:
 *
 *              1) Call H5_timer_init(), passing in a timer struct, to set
 *                 up the timer.
 *
 *              2) Wrap any code you'd like to time with calls to
 *                 H5_timer_start/stop().  For accurate timing, place these
 *                 calls as close to the code of interest as possible.  You
 *                 can call start/stop multiple times on the same timer -
 *                 when you do this, H5_timer_get_times() will return time
 *                 values for the current/last session and
 *                 H5_timer_get_total_times() will return the summed times
 *                 of all sessions (see #3 and #4, below).
 *
 *              3) Use H5_timer_get_times() to get the current system, user
 *                 and elapsed times from a running timer.  If called on a
 *                 stopped timer, this will return the time recorded at the
 *                 stop point.
 *
 *              4) Call H5_timer_get_total_times() to get the total system,
 *                 user and elapsed times recorded across multiple start/stop
 *                 sessions.  If called on a running timer, it will return the
 *                 time recorded up to that point.  On a stopped timer, it
 *                 will return the time recorded at the stop point.
 *
 *              NOTE: Obtaining a time point is not free!  Keep in mind that
 *                    the time functions make system calls and can have
 *                    non-trivial overhead.  If you call one of the get_time
 *                    functions on a running timer, that overhead will be
 *                    added to the reported times.
 *
 *              5) All times recorded will be in seconds.  These can be
 *                 converted into human-readable strings with the
 *                 H5_timer_get_time_string() function.
 *
 *              6) A timer can be reset using by calling H5_timer_init() on
 *                 it.  This will set its state to 'stopped' and reset all
 *                 accumulated times to zero.
 *
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5_timer_init(H5_timer_t *timer /*in,out*/)
{
    assert(timer);

    /* Initialize everything */

    timer->initial.elapsed = 0.0F;
    timer->initial.system  = 0.0F;
    timer->initial.user    = 0.0F;

    timer->final_interval.elapsed = 0.0F;
    timer->final_interval.system  = 0.0F;
    timer->final_interval.user    = 0.0F;

    timer->total.elapsed = 0.0F;
    timer->total.system  = 0.0F;
    timer->total.user    = 0.0F;

    timer->is_running       = 0;

#if !defined(H5_HAVE_GETRUSAGE) && !defined(_WIN32)
    timer->has_user_system_times = 0;
#else
    timer->has_user_system_times = 1;
#endif

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    H5_timer_start
 *
 * Purpose:     Start tracking time in a platform-independent timer.
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5_timer_start(H5_timer_t *timer /*in,out*/)
{
    herr_t  err = -1;

    assert(timer);

    /* Start the timer
     * This sets the "initial" times to the system-defined start times.
     */
    err = _timer_get_timevals(&(timer->initial));
    if(err < 0)
        return -1;

    timer->is_running = 1;

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    H5_timer_stop
 *
 * Purpose:     Stop tracking time in a platform-independent timer.
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5_timer_stop(H5_timer_t *timer /*in,out*/)
{
    herr_t          err = -1;

    assert(timer);

    /* Stop the timer */
    err = _timer_get_timevals(&(timer->final_interval));
    if(err < 0)
        return -1;

    /* The "final" times are stored as intervals (final - initial)
     * for more useful reporting to the user.
     */
    timer->final_interval.elapsed = timer->final_interval.elapsed - timer->initial.elapsed;
    timer->final_interval.system  = timer->final_interval.system  - timer->initial.system;
    timer->final_interval.user    = timer->final_interval.user    - timer->initial.user;

    /* Add the intervals to the elapsed time */
    timer->total.elapsed += timer->final_interval.elapsed;
    timer->total.system  += timer->final_interval.system;
    timer->total.user    += timer->final_interval.user;

    timer->is_running = 0;

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    H5_timer_get_times
 *
 * Purpose:     Get the system, user and elapsed times from a timer.  These
 *              are the times since the timer was last started and will be
 *              0.0 in a timer that has not been started since it was
 *              initialized.
 *
 *              This function can be called either before or after 
 *              H5_timer_stop() has been called.  If it is called before the
 *              stop function, the timer will continue to run.
 *
 *              The system and user times will be -1.0 if those times cannot
 *              be computed on a particular platform.  The elapsed time will
 *              always be present.
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5_timer_get_times(H5_timer_t timer, H5_timevals_t *times /*in,out*/)
{
    H5_timevals_t   now;
    herr_t          err = -1;

    assert(times);

    if(timer.is_running) {

        /* Get the current times and report the current intervals without
         * stopping the timer.
         */
        err = _timer_get_timevals(&now);
        if(err < 0)
            return -1;

        times->elapsed = now.elapsed - timer.initial.elapsed;
        times->system  = now.system  - timer.initial.system;
        times->user    = now.user    - timer.initial.user;

    }
    else {
        times->elapsed = timer.final_interval.elapsed;
        times->system  = timer.final_interval.system;
        times->user    = timer.final_interval.user;
    }

    return 0;
}


/*-------------------------------------------------------------------------
 * Function:    H5_timer_get_total_times
 *
 * Purpose:     Get the TOTAL system, user and elapsed times recorded by
 *              the timer since its initialization.  This is the sum of all
 *              times recorded while the timer was running.
 *
 *              These will be 0.0 in a timer that has not been started
 *              since it was initialized.  Calling H5_timer_init() on a
 *              timer will reset these values to 0.0.
 *
 *              This function can be called either before or after 
 *              H5_timer_stop() has been called.  If it is called before the
 *              stop function, the timer will continue to run.
 *
 *              The system and user times will be -1.0 if those times cannot
 *              be computed on a particular platform.  The elapsed time will
 *              always be present.
 *
 * Return:      Success:    0
 *              Failure:    -1
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */
herr_t
H5_timer_get_total_times(H5_timer_t timer, H5_timevals_t *times /*in,out*/)
{
    H5_timevals_t   now;
    herr_t          err = -1;

    assert(times);

    if(timer.is_running) {

        /* Get the current times and report the current totals without
         * stopping the timer.
         */
        err = _timer_get_timevals(&now);
        if(err < 0)
            return -1;

        times->elapsed = timer.total.elapsed + (now.elapsed - timer.initial.elapsed);
        times->system  = timer.total.system  + (now.system  - timer.initial.system);
        times->user    = timer.total.user    + (now.user    - timer.initial.user);

    }
    else {
        times->elapsed = timer.total.elapsed;
        times->system  = timer.total.system;
        times->user    = timer.total.user;
    }

    return 0;
}

