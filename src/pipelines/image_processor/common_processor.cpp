#include <pipelines/image_processor/common_processor.h>
#include <common/settings.h>
#include <common/tic_toc.h>
#include <math.h>
#include <cmath>
#include <common/yaml.hpp>
#include <common/clock.h>
#include <glog/logging.h>


image_process_settings_ settings_;

CommonProcessor::CommonProcessor(thread_queue::ThreadQueue<basic_item> &queue) : queue_(queue)
{
    load_yaml();
}

void CommonProcessor::init(int queue_id)
{
    get_queue_id_ = queue_id;
    queue_id_ = queue_.add_queue();
    std::thread processor_thread(process_image, this);
    processor_thread.detach();
}

int CommonProcessor::process_image(CommonProcessor *processer)
{
    const std::string ai_name = "ai_image";
    while (!processer->queue_.get_try_to_expire(processer->get_queue_id_))
    {
        basic_item item;
        if (!processer->queue_.get_queue(processer->get_queue_id_, item))
        {
            continue;
        }

        TicToc tic;
        cv::Mat image = item.first;

        processer->add_alpha(image, image);


        TicToc undistortion_toc;
        processer->remap.undistortion(image, image);
        double delay = undistortion_toc.toc();
        processer->remap.undistortion_benchmark(delay);
        undistortion_toc.toc_print("undistortion");


        basic_item insert_item(image, item.second);
        processer->queue_.insert_queue(
            processer->queue_id_,
            insert_item);
        // tic.toc_print("process queue");
        // LOG(INFO) << "frame use " << get_current_time() - item.second << "s.";
        // break;
    }
    double benchmark = -1;
    processer->remap.undistortion_benchmark(benchmark);
    std::flush(std::cout);
    std::cout << "----------------------------" << std::endl;
    std::cout << " avg result: " << benchmark << std::endl;
    std::cout << "----------------------------" << std::endl;
    return 0;
}

bool CommonProcessor::load_yaml()
{
    Yaml::Node root;
    try
    {
        Yaml::Parse(root, "../config/zntk_ar.yaml");
    }
    catch (const Yaml::Exception e)
    {
        std::cout << "Exception " << e.Type() << ": " << e.what() << std::endl;
        return false;
    }
    try
    {
        settings_.output_height = root["output_height"].As<int>(-1);
        settings_.output_width = root["output_width"].As<int>(-1);
        settings_.output_col_offset = root["output_col_offset"].As<int>(0);
        settings_.output_row_offset = root["output_row_offset"].As<int>(0);
        settings_.output_trans = root["output_trans"].As<bool>(false);
        settings_.output_copy = root["output_copy"].As<bool>(false);
        settings_.output_scale = root["output_scale"].As<double>(1.0);
    }
    catch (const Yaml::Exception e)
    {
        std::cout << "Exception " << e.Type() << ": " << e.what() << std::endl;
        return false;
    }
    return true;
}

void CommonProcessor::screen_setting(int screen_rows, int screen_cols)
{
    CommonProcessor::screen_rows = screen_rows;
    CommonProcessor::screen_cols = screen_cols;
}


int CommonProcessor::add_alpha(cv::Mat &src, cv::Mat &dst)
{
    if (src.channels() == 4)
    {
        dst = src.clone();
        return 0;
    }
    cv::cvtColor(src, dst, cv::COLOR_RGB2RGBA);
    return 0;
}
