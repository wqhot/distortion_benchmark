#pragma once
#include <pipelines/image_producer/sequence_producer.h>

#include <pipelines/image_processor/common_processor.h>

#include <pipelines/image_consumer/imshow_consumer.h>

#include <memory>
#include <string>

// __asm__(".symver stat,__xstat@GLIBC_2.2.5");

enum DRAW_TYPE
{
    DRAW_BY_FC,
    DRAW_BY_IMSHOW,
    DRAW_BY_GTK,
    DRAW_BY_FULLSCREEN_IMSHOW
};

class PipeLines
{
public:
    PipeLines() : common_processor_(queue_)
    {
    }
    bool init_producer(std::string, double fps);

    bool init_processor();

    bool init_consumer(DRAW_TYPE consumer_type);

    bool start();

    void stop()
    {
        queue_.try_to_expire();
    }


private:
    thread_queue::ThreadQueue<basic_item> queue_;
    std::unique_ptr<ImageProducer> producer_;
    CommonProcessor common_processor_;
    std::unique_ptr<ImageConsumer> consumer_;
};