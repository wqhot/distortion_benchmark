#pragma once
#include <opencv2/opencv.hpp>
#include <functional>
#include "remap_opencv.hpp"



class ImageUtils
{
public:
    
    void undistortion(cv::Mat &src, cv::Mat &dst)
    {
        remap.remap_v2(src, dst);
        // remap.remap(src, dst);
        return;
    }
    void undistortion_benchmark(double &delay)
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
    Remap remap;
};