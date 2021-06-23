#pragma once

#include <functional>
#include <iostream>
#include <atomic>
#include <vector>
#include <common/tic_toc.h>
#include <pipelines/thread_manage.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <common/settings.h>
#include <utils/image_utils.hpp>


class CommonProcessor
{
private:
    // 线程
    static int process_image(CommonProcessor *processer);
    thread_queue::ThreadQueue<basic_item> &queue_;
    int queue_id_;
    int get_queue_id_;
    int screen_rows;
    int screen_cols;
    
    bool load_yaml();
    int resize_uniform(cv::Mat &src, cv::Mat &dst, cv::Size dst_size);

    ImageUtils remap;

    int add_alpha(cv::Mat &src, cv::Mat &dst);
public:
    CommonProcessor(thread_queue::ThreadQueue<basic_item> &queue);
    void init(int get_queue);
    int get_queue_id()
    {
        return queue_id_;
    }
    void screen_setting(int screen_rows, int screen_cols);
};