#pragma once

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <time.h>

inline double get_current_time()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz); //该函数在sys/time.h头文件中
    return double(tv.tv_sec) + double(tv.tv_usec) / 1000000.0;
}