#include <sys/time.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

typedef int (*GETTIMEOFDAY) (struct timeval *tv, void *tz);
typedef int (*CLOCK_GETTIME) (clockid_t clk_id, struct timespec *tp);

GETTIMEOFDAY real_gettimeofday = nullptr;
CLOCK_GETTIME real_clock_gettime = nullptr;

float speedmultiplier = 1.5f;

// تم إصلاح المصفوفة وتحديد حجمها [10] بدقة تامة لتخطي حماية المترجم
struct ClockState {
    int result;
    struct timespec initialoffset;
    struct timespec initialtime;
};

struct ClockState initialclock[10];

struct timeval initial_offset_tod_tv;
struct timeval initial_time_tod_tv;

extern "C" int new_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    int r = -1;
    struct timespec currenttp;

    if (real_clock_gettime) {
        r = real_clock_gettime(clk_id, &currenttp);
    } else {
        r = clock_gettime(clk_id, &currenttp);
    }

    if ((clk_id <= 9) && (initialclock[clk_id].result == 0))
    {
        struct timespec temptp;
        temptp.tv_sec = currenttp.tv_sec - initialclock[clk_id].initialtime.tv_sec;
        temptp.tv_nsec = currenttp.tv_nsec - initialclock[clk_id].initialtime.tv_nsec;

        if (temptp.tv_nsec < 0) {
            temptp.tv_nsec += 1000000000;
            temptp.tv_sec--;
        }

        double newsec_double = (double)temptp.tv_sec * speedmultiplier;
        int64_t newnsec = floor((double)temptp.tv_nsec * speedmultiplier);
        int64_t newsec = floor(newsec_double);

        newnsec += floor((newsec_double - floor(newsec_double)) * 1000000000.0f);
        newsec += initialclock[clk_id].initialoffset.tv_sec;
        newnsec += initialclock[clk_id].initialoffset.tv_nsec;

        newsec += newnsec / 1000000000;
        newnsec = newnsec % 1000000000;

        if (newnsec < 0) {
            newnsec += 1000000000;
            newsec--;
        }

        if (tp) {
            tp->tv_sec = newsec;
            tp->tv_nsec = newnsec;
        }
    }
    else if (tp) {
        *tp = currenttp;
    }

    return r;
}

extern "C" int new_gettimeofday(struct timeval *tv, void *tz)
{
    int r = -1;
    struct timeval currenttv;
    struct timeval temptv;

    if (real_gettimeofday) {
        r = real_gettimeofday(&currenttv, tz);
    } else {
        r = gettimeofday(&currenttv, (struct timezone *)tz);
    }

    temptv.tv_sec = currenttv.tv_sec - initial_time_tod_tv.tv_sec;
    temptv.tv_usec = currenttv.tv_usec - initial_time_tod_tv.tv_usec;

    if (temptv.tv_usec < 0) {
        temptv.tv_usec += 1000000;
        temptv.tv_sec--;
    }

    double newsec_double = (double)temptv.tv_sec * speedmultiplier;
    int64_t newusec = floor((double)temptv.tv_usec * speedmultiplier);
    int64_t newsec = floor(newsec_double);

    newusec += floor((newsec_double - floor(newsec_double)) * 1000000.0f);
    newsec += initial_offset_tod_tv.tv_sec;
    newusec += initial_offset_tod_tv.tv_usec;

    newsec += newusec / 1000000;
    newusec = newusec % 1000000;

    if (newusec < 0) {
        newusec += 1000000;
        newsec--;
    }

    if (tv) {
        tv->tv_sec = newsec;
        tv->tv_usec = newusec;
    }

    return r;
}

extern "C" int speedhack_initializeSpeed(float speed)
{
    gettimeofday(&initial_offset_tod_tv, NULL);

    if (real_gettimeofday)
        real_gettimeofday(&initial_time_tod_tv, NULL);
    else
        gettimeofday(&initial_time_tod_tv, NULL);

    for (int i = 0; i <= 9; i++)
    {
        clock_gettime(i, &initialclock[i].initialoffset);

        if (real_clock_gettime)
            initialclock[i].result = real_clock_gettime(i, &initialclock[i].initialtime);
        else
            initialclock[i].result = clock_gettime(i, &initialclock[i].initialtime);
    }

    speedmultiplier = 1.5f;
    return 1;
}
