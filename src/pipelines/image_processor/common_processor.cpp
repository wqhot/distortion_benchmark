#include <pipelines/image_processor/common_processor.h>
#include <common/settings.h>
#include <common/tic_toc.h>
#include <math.h>
#include <cmath>
#include <common/yaml.hpp>
#include <common/clock.h>
#include <glog/logging.h>
#include <utils/image_utils.hpp>


reflock::RefLockManager<cv::Mat> global_reflock;
reflock::RefLockManager<std::pair<cv::Mat, unsigned long>> cambrion_reflock;
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
        // 发送到引用存储区
        global_reflock.try_set(ai_name, image);

        processer->add_alpha(image, image);


        TicToc undistortion_toc;
        ImageUtils::undistortion(image, image);
        double delay = undistortion_toc.toc();
        ImageUtils::undistortion_benchmark(delay);
        undistortion_toc.toc_print("undistortion");


        basic_item insert_item(image, item.second);
        processer->queue_.insert_queue(
            processer->queue_id_,
            insert_item);
        // tic.toc_print("process queue");
        // LOG(INFO) << "frame use " << get_current_time() - item.second << "s.";
        // break;
    }
    std::cout << "CommonProcessor thread over" << std::endl;
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

int CommonProcessor::resize_uniform(cv::Mat &src, cv::Mat &dst, cv::Size dst_size)
{
    int w = src.cols;
    int h = src.rows;
    int dst_w = dst_size.width;
    int dst_h = dst_size.height;
    // std::cout << "src: (" << h << ", " << w << ")" << std::endl;
    cv::Mat dst_ = cv::Mat(cv::Size(dst_w, dst_h), src.type(), cv::Scalar(0));

    float ratio_src = w * 1.0 / h;
    float ratio_dst = dst_w * 1.0 / dst_h;

    int tmp_w = 0;
    int tmp_h = 0;
    if (ratio_src > ratio_dst)
    {
        tmp_w = dst_w;
        tmp_h = floor((dst_w * 1.0 / w) * h);
    }
    else if (ratio_src < ratio_dst)
    {
        tmp_h = dst_h;
        tmp_w = floor((dst_h * 1.0 / h) * w);
    }
    else
    {
        resize(src, dst_, dst_size);
        return 0;
    }

    cv::Mat tmp;
    resize(src, tmp, cv::Size(tmp_w, tmp_h));

    if (tmp_w != dst_w)
    { //高对齐，宽没对齐
        int index_w = floor((dst_w - tmp_w) / 2.0);
        tmp.copyTo(dst_.colRange(index_w, tmp_w + index_w));
    }
    else if (tmp_h != dst_h)
    { //宽对齐， 高没有对齐
        int index_h = floor((dst_h - tmp_h) / 2.0);
        tmp.copyTo(dst_.rowRange(index_h, tmp_h + index_h));
    }
    else
    {
        printf("error\n");
    }

    dst = dst_;
    return 0;
}

void CommonProcessor::trans_remap(cv::Mat &src, cv::Mat &dst)
{
    TicToc tic;
    const int col_offset = 0;
    const int row_offset = 0;
    int _rows = settings_.output_height; // 1080
    int _cols = settings_.output_width; // 1920
    int scale_rows = settings_.output_scale * _rows;                                       // 540
    int scale_cols = settings_.output_scale * _cols;                                       // 960
    cv::Mat left, right, small;
    resize_uniform(src, dst, cv::Size(scale_cols, scale_rows));
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
