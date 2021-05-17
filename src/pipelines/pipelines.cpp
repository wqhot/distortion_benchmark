#include <pipelines/pipelines.h>
#include <functional>
#include <cctype>
#include <sys/stat.h>
#include <regex>

bool PipeLines::init_producer(std::string file_name, double fps)
{
    const std::vector<std::string> video = {".webm", ".mp4", ".avi"};
    const std::vector<std::string> pic = {".bmp", ".jpeg", ".jpg", ".png"};
    struct stat s_buf;
    stat(file_name.c_str(), &s_buf);
    bool is_video = false;
    bool is_usb = false;
    bool is_picture = false;

    // // 如果是路径
    if (S_ISDIR(s_buf.st_mode))
    {
        producer_ = std::make_unique<SequenceProducer>(file_name, fps, queue_);
        return true;
    }
    return false;
}


bool PipeLines::init_processor()
{
    // init common processor
    int producer_queue_id = producer_->get_queue_id();
    if (producer_queue_id < 0)
    {
        return false;
    }
    common_processor_.init(producer_queue_id);
    common_processor_.screen_setting(
        producer_->rows,
        producer_->cols);

    return true;
}

bool PipeLines::init_consumer(DRAW_TYPE consumer_type)
{
    switch (consumer_type)
    {
 
    case DRAW_BY_IMSHOW:
        consumer_ = std::make_unique<ImshowConsumer>(
            common_processor_.get_queue_id(),
            -1,
            queue_);
        break;
    // case DRAW_BY_FULLSCREEN_IMSHOW:
    //     consumer_ = std::make_unique<FullscreenImshowConsumer>(
    //         common_processor_.get_queue_id(),
    //         hud_processor_.get_queue_id(),
    //         queue_);
    //     break;
    default:
        return false;
        break;
    }

    return true;
}

bool PipeLines::start()
{
    producer_->start();
    return true;
}
