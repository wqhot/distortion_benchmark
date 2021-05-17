#include <pipelines/image_producer/sequence_producer.h>
#include <time.h>
#include <opencv2/highgui/highgui.hpp>

SequenceProducer::SequenceProducer(std::string file_name, double fps, thread_queue::ThreadQueue<basic_item> &queue) : ImageProducer(queue)
{
    std::string first_file = file_name + "/%04d.bmp";

    capture_.open(first_file);
    if (!capture_.isOpened())
    {
        return;
    }
    rows = capture_.get(cv::CAP_PROP_FRAME_HEIGHT);
    cols = capture_.get(cv::CAP_PROP_FRAME_WIDTH);
    double rate = fps * 30 > 0 ? fps * 30 : 10.0;
    frame_delay_ = 1000 / rate;
}

SequenceProducer::~SequenceProducer()
{
}

void SequenceProducer::start()
{
    timer_.start_test_zy(frame_delay_, get_frame, this);
}

bool SequenceProducer::get_frame(SequenceProducer *producer)
{
    cv::Mat frame;
    producer->capture_ >> frame;
    int frame_id = producer->capture_.get(cv::CAP_PROP_POS_FRAMES);
    std::cout << "frame id = " << frame_id << std::endl;
    if (!frame.data)
    {
        return false;
        // producer->capture_.set(cv::CAP_PROP_POS_FRAMES, 0);
        // return true;
    }

    double stamp = get_current_time();
    basic_item insert_item(frame, stamp);
    producer->queue_.insert_queue(
        producer->queue_id_,
        insert_item);
    return true;
}