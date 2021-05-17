#pragma once
#include <opencv2/opencv.hpp>


class ImageUtils
{
public:
    static void undistortion(cv::Mat &src, cv::Mat &dst)
    {
        return;
    }
    static void undistortion_benchmark(double &delay)
    {
        static double sum_delay = 0.0;
        static unsigned long count = 0;
        if (delay > 0)
        {
            count++;
            sum_delay += delay;
        }
        else if (delay < 0)
        {
            delay = sum_delay / count;
        }
        return;
    }
private:
};