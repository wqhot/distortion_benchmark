#pragma once

#include <pipelines/image_consumer/image_consumer.h>

class ImshowConsumer : public ImageConsumer
{
public:
    ImshowConsumer(int queue_id, int hud_queue_id, thread_queue::ThreadQueue<basic_item> &queue) : ImageConsumer(queue_id, hud_queue_id, queue)
    {
        col_offset = 0;
        row_offset = 0;
        output_width = 1280;
        output_height = 720;
        output_trans = false;
        output_copy = false;
        output_scale = 1;
        screen_cols = 1280;
        screen_rows = 720;
        // cv::startWindowThread();
    };

private:
    void pop(cv::Mat &image);
    void pop_second(cv::Mat &image)
    {

    }
};