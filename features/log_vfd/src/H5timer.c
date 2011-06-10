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



static herr_t
H5_timer_get_timevals(H5_timevals_t *times /*in,out*/)
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
        times->elapsed_ps   = -1;
        times->system_ps    = -1;
        times->user_ps      = -1;
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
    times->system_ps = ((double)res.ru_stime.tv_sec * 1.0E9F) + ((double)res.ru_stime.tv_usec * 1.0E3F);
    times->user_ps   = ((double)res.ru_utime.tv_sec * 1.0E9F) + ((double)res.ru_utime.tv_usec * 1.0E3F);

#else

    /* No suitable way to get system/user times */
    /* This is not an error condition, they just won't be available */
    times->system_ps = -1.0;
    times->user_ps   = -1.0;

#endif


    /****************
     * Elapsed time *
     ****************/

    /* NOTE: Not having a way to get elapsed time IS an error, unlike
     * the system and user times.
     */

#if defined(H5_HAVE_MACH_MACH_TIME_H)

    /* Mac OS X */
    times->elapsed_ps = H5_get_mach_time_ps();
    if(times->elapsed_ps < 0.0)
        return -1;

#elif defined(H5_HAVE_CLOCK_GETTIME)

    err = clock_gettime(CLOCK_MONOTONIC, &ts);
    if(err != 0)
        return -1;
    times->elapsed_ps = ((double)ts.tv_sec * 1.0E12F) + ((double)ts.tv_nsec * 1.0E3F);

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

    timer->initial.elapsed_ps = 0.0F;
    timer->initial.system_ps  = 0.0F;
    timer->initial.user_ps    = 0.0F;

    timer->final_interval.elapsed_ps = 0.0F;
    timer->final_interval.system_ps  = 0.0F;
    timer->final_interval.user_ps    = 0.0F;

    timer->total.elapsed_ps = 0.0F;
    timer->total.system_ps  = 0.0F;
    timer->total.user_ps    = 0.0F;

    timer->is_running       = 0;

#if !defined(H5_HAVE_GETRUSAGE) && !defined(_WIN32)
    timer->has_user_system_times = 0;
#else
    timer->has_user_system_times = 1;
#endif

    return 0;
}



herr_t
H5_timer_start(H5_timer_t *timer /*in,out*/)
{
    herr_t  err = -1;

    assert(timer);

    /* Start the timer
     * This sets the "initial" times to the system-defined start times.
     */
    err = H5_timer_get_timevals(&(timer->initial));
    if(err < 0)
        return -1;

    timer->is_running = 1;

    return 0;
}



herr_t
H5_timer_stop(H5_timer_t *timer /*in,out*/)
{
    herr_t          err = -1;

    assert(timer);

    /* Stop the timer */
    err = H5_timer_get_timevals(&(timer->final_interval));
    if(err < 0)
        return -1;

    /* The "final" times are stored as intervals (final - initial)
     * for more useful reporting to the user.
     */
    timer->final_interval.elapsed_ps = timer->final_interval.elapsed_ps - timer->initial.elapsed_ps;
    timer->final_interval.system_ps  = timer->final_interval.system_ps  - timer->initial.system_ps;
    timer->final_interval.user_ps    = timer->final_interval.user_ps    - timer->initial.user_ps;

    /* Add the intervals to the elapsed time */
    timer->total.elapsed_ps += timer->final_interval.elapsed_ps;
    timer->total.system_ps  += timer->final_interval.system_ps;
    timer->total.user_ps    += timer->final_interval.user_ps;

    timer->is_running = 0;

    return 0;
}

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
        err = H5_timer_get_timevals(&now);
        if(err < 0)
            return -1;

        times->elapsed_ps = now.elapsed_ps - timer.initial.elapsed_ps;
        times->system_ps  = now.system_ps  - timer.initial.system_ps;
        times->user_ps    = now.user_ps    - timer.initial.user_ps;

    }
    else {
        times->elapsed_ps = timer.final_interval.elapsed_ps;
        times->system_ps  = timer.final_interval.system_ps;
        times->user_ps    = timer.final_interval.user_ps;
    }

    return 0;
}

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
        err = H5_timer_get_timevals(&now);
        if(err < 0)
            return -1;

        times->elapsed_ps = timer.total.elapsed_ps + (now.elapsed_ps - timer.initial.elapsed_ps);
        times->system_ps  = timer.total.system_ps  + (now.system_ps  - timer.initial.system_ps);
        times->user_ps    = timer.total.user_ps    + (now.user_ps    - timer.initial.user_ps);

    }
    else {
        times->elapsed_ps = timer.total.elapsed_ps;
        times->system_ps  = timer.total.system_ps;
        times->user_ps    = timer.total.user_ps;
    }

    return 0;
}




/*-------------------------------------------------------------------------
 * Function:    H5_timer_get_time_string
 *
 * Purpose:     Converts a time (in picoseconds) into a human-readable
 *              string suitable for log messages.
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
 * safe size.  Allocating the correct size would be painful.
 */
#define H5TIMER_TIME_STRING_LEN 256

char *
H5_timer_get_time_string(double ps)
{

    double hours    = 0.0F;
    double minutes  = 0.0F;
    double seconds  = 0.0F;

    double display_hours    = 0.0F;
    double display_minutes  = 0.0F;
    double display_seconds  = 0.0F;

    double fake_intpart;

    char *s;                /* output string */

    /* Initialize */
    s = (char *)calloc(H5TIMER_TIME_STRING_LEN, sizeof(char));
    if(NULL == s)
        return NULL;

    if(ps > 0.0F) {

        seconds = ps / 1.0E12F;

        hours   = seconds / 3600.0F;
        display_hours = floor(hours);

        minutes = modf(hours, &fake_intpart) * 60.0F;
        display_minutes = floor(minutes);

        display_seconds = modf(minutes, &fake_intpart) * 60.0F;
    }

    /* Do we need a format string? Some people might like a certain 
     * number of milliseconds or s before spilling to the next highest
     * time unit.  Perhaps this could be passed as an integer.
     * (name? round_up_size? ?)
     */
    if(ps < 0.0F) {
        sprintf(s, "N/A");
    }else if(ps < 1.0E6F) {
        /* t < 1 us, Print time in ns */
        sprintf(s, "%.f ns", ps / 1.0E3F);
    } else if (ps < 1.0E9F) {
        /* t < 1 ms, Print time in us */
        sprintf(s, "%.1f us", ps / 1.0E6F);
    } else if (ps < 1.0E12F) {
        /* t < 1 s, Print time in ms */
        sprintf(s, "%.1f ms", ps / 1.0E9F);
    } else if (ps < 1.0E12F * 60) {
        /* t < 1 m, Print time in s */
        sprintf(s, "%.2f s", display_seconds);
    } else if (ps < 1.0E12F * 60 * 60) {
        /* t < 1 h, Print time in m and s */
        sprintf(s, "%.f m %.f s", display_minutes, display_seconds);
    } else {
        /* Print time in h, m and s */
        sprintf(s, "%.f h %.f m %.f s", display_hours, display_minutes, display_seconds);
    }

    return s;
}
