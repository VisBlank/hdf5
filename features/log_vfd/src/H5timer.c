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

/* Generic functions, including H5_timer_t */
#include "H5private.h"




/*-------------------------------------------------------------------------
 * Function: H5_timer_begin
 *
 * Purpose: Initialize a timer to time something.
 *
 * Return: void
 *
 * Programmer: Robb Matzke
 *             Thursday, April 16, 1998
 *-------------------------------------------------------------------------
 */
void
H5_timer_begin (H5_timer_OLD_t *timer)
{

#ifdef H5_HAVE_GETRUSAGE
    struct rusage     rusage;
#endif

#ifdef H5_HAVE_GETTIMEOFDAY
    struct timeval    etime;
#endif

    assert (timer);

#ifdef H5_HAVE_GETRUSAGE
    HDgetrusage (RUSAGE_SELF, &rusage);
    timer->utime = (double)rusage.ru_utime.tv_sec +
                   ((double)rusage.ru_utime.tv_usec / 1.0e6F);
    timer->stime = (double)rusage.ru_stime.tv_sec +
                   ((double)rusage.ru_stime.tv_usec / 1.0e6F);
#else
    timer->utime = 0.0F;
    timer->stime = 0.0F;
#endif

#ifdef H5_HAVE_GETTIMEOFDAY
    HDgettimeofday (&etime, NULL);
    timer->etime = (double)etime.tv_sec + ((double)etime.tv_usec / 1.0e6F);
#else
    timer->etime = 0.0F;
#endif

} /* end H5_timer_begin() */


/*-------------------------------------------------------------------------
 * Function:	H5_timer_end
 *
 * Purpose:	This function should be called at the end of a timed region.
 *		The SUM is an optional pointer which will accumulate times.
 *		TMS is the same struct that was passed to H5_timer_start().
 *		On return, TMS will contain total times for the timed region.
 *
 * Return:	void
 *
 * Programmer:	Robb Matzke
 *              Thursday, April 16, 1998
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
void
H5_timer_end (H5_timer_OLD_t *sum/*in,out*/, H5_timer_OLD_t *timer/*in,out*/)
{
    H5_timer_OLD_t      now;

    assert (timer);
    H5_timer_begin (&now);

    timer->utime = MAX(0.0F, now.utime - timer->utime);
    timer->stime = MAX(0.0F, now.stime - timer->stime);
    timer->etime = MAX(0.0F, now.etime - timer->etime);

    if (sum) {
        sum->utime += timer->utime;
        sum->stime += timer->stime;
        sum->etime += timer->etime;
    }
} /* end H5_timer_end() */


/*-------------------------------------------------------------------------
 * Function: H5_bandwidth
 *
 * Purpose: Prints the bandwidth (bytes per second) in a field 10
 *          characters wide widh four digits of precision like this:
 *
 *  NaN             If <=0 seconds
 *  1234.  TB/s
 *  123.4  TB/s
 *  12.34  GB/s
 *  1.234  MB/s
 *  4.000  kB/s
 *  1.000  B/s
 *  0.000  B/s      If NBYTES==0
 *  1.2345e-10      For bandwidth less than 1
 *  6.7893e+94      For exceptionally large values
 *  6.678e+106      For really big values
 *
 * Return: void
 *
 * Programmer: Robb Matzke
 *             Wednesday, August  5, 1998
 *-------------------------------------------------------------------------
 */
void
H5_bandwidth(char *buf/*out*/, double nbytes, double nseconds)
{
    double      bw;

    if(nseconds <= 0.0F) {
        HDstrcpy(buf, "       NaN");
    } else {
        bw = nbytes/nseconds;
        if(fabs(bw) < 0.0000000001F) {
            /* That is == 0.0, but direct comparison between floats is bad */
            HDstrcpy(buf, "0.000  B/s");
        } else if(bw < 1.0F) {
            sprintf(buf, "%10.4e", bw);
        } else if(bw < 1024.0F) {
            sprintf(buf, "%05.4f", bw);
            HDstrcpy(buf+5, "  B/s");
        } else if(bw < (1024.0F * 1024.0F)) {
            sprintf(buf, "%05.4f", bw / 1024.0F);
            HDstrcpy(buf+5, " kB/s");
        } else if(bw < (1024.0F * 1024.0F * 1024.0F)) {
            sprintf(buf, "%05.4f", bw / (1024.0F * 1024.0F));
            HDstrcpy(buf+5, " MB/s");
        } else if(bw < (1024.0F * 1024.0F * 1024.0F * 1024.0F)) {
            sprintf(buf, "%05.4f", bw / (1024.0F * 1024.0F * 1024.0F));
            HDstrcpy(buf+5, " GB/s");
        } else if(bw < (1024.0F * 1024.0F * 1024.0F * 1024.0F * 1024.0F)) {
            sprintf(buf, "%05.4f", bw / (1024.0F * 1024.0F * 1024.0F * 1024.0F));
            HDstrcpy(buf+5, " TB/s");
        } else {
            sprintf(buf, "%10.4e", bw);
            if(HDstrlen(buf) > 10) {
                sprintf(buf, "%10.3e", bw);
            }
        }
    }
} /* end H5_bandwidth() */




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




/*-------------------------------------------------------------------------
 * Function:    H5_timer_get_time_string
 *
 * Purpose:     Converts a time (in seconds) into a human-readable string
 *              suitable for log messages.
 *
 * Return:      Success:  The time string.
 *
 *                        The general format of the time string is:
 *
 *                        "N/A"                 time < 0 (invalid time)
 *                        "%.f ns"              time < 1 microsecond
 *                        "%.1f us"             time < 1 millisecond
 *                        "%.1f ms"             time < 1 second
 *                        "%.2f s"              time < 1 minute
 *                        "%.f m %.f s"         time < 1 hour
 *                        "%.f h %.f m %.f s"   longer times
 *
 *              Failure:  NULL
 *
 * Programmer:  Dana Robinson
 *              May 2011
 *
 *-------------------------------------------------------------------------
 */

/* Size of the generated time string.
 * Most time strings should be < 20 or so characters (max!) so this should be a
 * safe size.  Dynamically allocating the correct size would be painful.
 */
#define H5TIMER_TIME_STRING_LEN 256

char *
H5_timer_get_time_string(double seconds)
{
    char *s;                /* output string */

    /* Used when the time is greater than 59 seconds */
    double days;
    double hours;
    double minutes;
    double remainder_sec;
    double conversion;

    if (seconds > 60.0F) {

        remainder_sec = seconds;

        /* Extract days */
        conversion = 24.0F * 60.0F * 60.0F;  /* seconds per day */
        days = floor(remainder_sec / conversion);
        remainder_sec = remainder_sec - (days * conversion);

        /* Extract hours */
        conversion = 60.0F * 60.0F;
        hours = floor(remainder_sec / conversion);
        remainder_sec = remainder_sec - (hours * conversion);

        /* Extract minutes */
        conversion = 60.0F;
        minutes = floor(remainder_sec / conversion);
        remainder_sec = remainder_sec - (minutes * conversion);

        /* The # of seconds left is stored in remainder_sec */
    }

    /* Initialize */
    s = (char *)calloc(H5TIMER_TIME_STRING_LEN, sizeof(char));
    if(NULL == s)
        return NULL;

    /* Do we need a format string? Some people might like a certain 
     * number of milliseconds or s before spilling to the next highest
     * time unit.  Perhaps this could be passed as an integer.
     * (name? round_up_size? ?)
     */
    if(seconds < 0.0F) {
        sprintf(s, "N/A");
    }
    else if(0.0F == seconds) {
        sprintf(s, "0.0 s");
    }
    else if(seconds < 1.0E-6F) {
        /* t < 1 us, Print time in ns */
        sprintf(s, "%.f ns", seconds * 1.0E9F);
    }
    else if (seconds < 1.0E-3F) {
        /* t < 1 ms, Print time in us */
        sprintf(s, "%.1f us", seconds * 1.0E6F);
    }
    else if (seconds < 1.0F) {
        /* t < 1 s, Print time in ms */
        sprintf(s, "%.1f ms", seconds * 1.0E3F);
    }
    else if (seconds < 60.0F) {
        /* t < 1 m, Print time in s */
        sprintf(s, "%.2f s", seconds);
    }
    else if (seconds < 60.0F * 60.0F) {
        /* t < 1 h, Print time in m and s */
        sprintf(s, "%.f m %.f s", minutes, remainder_sec);
    }
    else if (seconds < 24.0F * 60.0F * 60.0F) {
        /* Print time in h, m and s */
        sprintf(s, "%.f h %.f m %.f s", hours, minutes, remainder_sec);
    }
    else {
        /* Print time in d, h, m and s */
        sprintf(s, "%.f d %.f h %.f m %.f s", days, hours, minutes, remainder_sec);
    }


    return s;
}
