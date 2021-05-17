#include <pipelines/image_producer/image_producer.h>

class SequenceProducer : public ImageProducer
{
public:
    SequenceProducer(std::string file_name, double fps, thread_queue::ThreadQueue<basic_item> &queue);
    ~SequenceProducer();

    void start();

private:
    cv::VideoCapture capture_;
    Timer_zy<SequenceProducer>timer_;
    double frame_delay_;
    static bool get_frame(SequenceProducer *producer);
};