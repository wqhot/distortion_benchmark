#pragma once

#include <functional>
#include <iostream>
#include <atomic>
#include <vector>
#include <pipelines/thread_manage.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <unistd.h>

#include <common/clock.h>
#include <pipelines/image_producer/timer_producer.h>

class ImageProducer
{
protected:
    int queue_id_;
    thread_queue::ThreadQueue<basic_item> &queue_;

public:
    unsigned int rows;
    unsigned int cols;
    ImageProducer(thread_queue::ThreadQueue<basic_item> &queue) : queue_(queue)
    {
        queue_id_ = -1;
        queue_id_ = queue.add_queue();
    }
    virtual void start() {};
    virtual ~ImageProducer() = default;
    int get_queue_id()
    {
        return queue_id_;
    }
};

