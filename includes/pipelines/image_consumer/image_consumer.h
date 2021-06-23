#pragma once

#include <atomic>
#include <pipelines/thread_manage.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <common/tic_toc.h>
#include <common/clock.h>
#include <common/settings.h>
#include <glog/logging.h>

class ImageConsumer
{
public:
    int screen_rows;
    int screen_cols;
    int screen_rows_second;
    int screen_cols_second;
    int screen_channels;
    ImageConsumer(int queue_id, int hud_queue_id, thread_queue::ThreadQueue<basic_item> &queue) : 
    queue_id_(queue_id), hud_queue_id_(hud_queue_id), queue_(queue)
    {
        double_monitor = false;
        screen_rows = 360;
        screen_cols = 640;
        screen_channels = 4;
        keep = true;
        std::thread roll(roll_consumer, this);
        roll.detach();
    }
    ~ImageConsumer()
    {
        keep = false;
    }
    void pop_test(cv::Mat &image)
    {
        remap(image, image);
        pop(image);
    }

protected:
    const std::string hud_name = "image_hud";
    thread_queue::ThreadQueue<basic_item> &queue_;
    int col_offset;
    int row_offset;
    int output_width;
    int output_height;
    bool output_trans;
    bool output_copy;
    float output_scale;

    int col_offset_second;
    int row_offset_second;
    int output_width_second;
    int output_height_second;
    bool output_trans_second;
    bool output_copy_second;
    float output_scale_second;
    
    int queue_id_;
    int hud_queue_id_;

    // 双屏
    bool double_monitor;
    std::atomic<bool> keep;
    virtual void pop(cv::Mat &image) = 0;
    virtual void pop_second(cv::Mat &image) = 0;
    static void roll_consumer(ImageConsumer *consumer)
    {
        double fps = 0;
        double last_stamp = get_current_time();
        while (!consumer->queue_.get_try_to_expire(consumer->queue_id_))
        {
            
            basic_item item, hud_item;
            if (!consumer->queue_.get_queue(consumer->queue_id_, item))
            {
                continue;
            }
            double now_stamp = get_current_time();
            // LOG(INFO) << "frame use before display " << get_current_time() - item.second << "s.";
            fps = 0.8*fps + 0.2*1.0 / (now_stamp - last_stamp);
            printf("FPS=%f delta=%f\n", fps, now_stamp - last_stamp);
            last_stamp = now_stamp;
            bool hud_ready = consumer->queue_.try_get_queue(consumer->hud_queue_id_, hud_item);
            consumer->queue_.notify_to_insert(consumer->hud_queue_id_);
            cv::Mat image = item.first;
            // std::cout << "hud_ready " << hud_ready << std::endl;
            // std::cout << "image.size " << image.size << std::endl;
            // std::cout << "hud.size " << hud_item.first.size << std::endl;
            if (image.size == hud_item.first.size && hud_ready)
            {
                cv::addWeighted(image, 1.0, hud_item.first, 1.0, 0.0, image);
            }
            cv::Mat second_image;
            if (consumer->double_monitor)
            {
                consumer->remap_second(image, second_image);
            }
            consumer->remap(image, image);
            consumer->pop(image);
            if (consumer->double_monitor)
            {
                consumer->pop_second(second_image);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            // LOG(INFO) << "frame use " << get_current_time() - item.second << "s.";
        }
        std::cout << "ImageConsumer thread over" << std::endl;
    }
    void remap(cv::Mat &src, cv::Mat &dst)
    {
        int _rows = output_trans ? output_width : output_height; // 1080
        int _cols = output_trans ? output_height : output_width; // 1920
        int scale_rows = output_scale * _rows;                   // 540
        int scale_cols = output_scale * _cols;                   // 960
        cv::Mat left, right, small;
        resize_uniform(src, small, cv::Size(scale_cols, scale_rows));
        if (output_trans)
        {
            cv::rotate(small, left, cv::ROTATE_90_COUNTERCLOCKWISE); // 逆时针
            if (output_copy)
            {
                cv::rotate(small, right, cv::ROTATE_90_CLOCKWISE); // 顺时针
            }
        }
        else
        {
            left = small;
            if (output_copy)
            {
                right = small;
            }
        }
        scale_rows = left.rows;
        scale_cols = left.cols;
        int screen_cols_ = output_copy ? screen_cols / 2 : screen_cols;
        int row_start = (screen_rows - scale_rows) / 2 < 0 ? 0 : (screen_rows - scale_rows) / 2;
        int col_start = (screen_cols_ - scale_cols) / 2 < 0 ? 0 : (screen_cols_ - scale_cols) / 2;
        int row_end = row_start + scale_rows + row_offset > screen_rows ? screen_rows : row_start + scale_rows + row_offset;
        dst = cv::Mat(cv::Size(
                          screen_cols,
                          screen_rows),
                      src.type(), cv::Scalar(0));
        // 防止超范围
        int final_row_start = row_start + row_offset > screen_rows ? screen_rows : row_start + row_offset;
        int final_col_start = col_start + col_offset > screen_cols_ ? screen_cols_ : col_start + col_offset;
        int final_row_end = row_start + row_offset + scale_rows > screen_rows ? screen_rows : row_start + row_offset + scale_rows;
        int final_col_end = col_start + col_offset + scale_cols > screen_cols_ ? screen_cols_ : col_start + col_offset + scale_cols;
        cv::Mat left_scale = dst.rowRange(final_row_start, final_row_end)
                                 .colRange(final_col_start, final_col_end);
        int final_row_start_c = row_offset > left_scale.rows ? scale_rows : 0;
        int final_col_start_c = col_offset > left_scale.cols ? scale_cols : 0;
        int final_row_end_c = row_offset + scale_rows > left_scale.rows ? left_scale.rows : scale_rows;
        int final_col_end_c = col_offset + scale_cols > left_scale.cols ? left_scale.cols : scale_cols;
        left.rowRange(final_row_start_c, final_row_end_c)
            .colRange(final_col_start_c, final_col_end_c)
            .copyTo(left_scale);
        if (output_copy)
        {
            // 防止超范围
            final_row_start = row_start + row_offset > screen_rows ? dst.rows : row_start + row_offset;
            final_col_start = col_start - col_offset > 0 ? col_start - col_offset + screen_cols_ : 0;
            final_row_end = row_start + row_offset + scale_rows > screen_rows ? dst.rows : row_start + row_offset + scale_rows;
            final_col_end = col_start - col_offset + scale_cols > screen_cols_ ? dst.cols : col_start - col_offset + scale_cols + screen_cols_;
            cv::Mat right_scale = dst.rowRange(final_row_start, final_row_end)
                                      .colRange(final_col_start, final_col_end);
            final_row_start_c = row_offset + scale_rows > right_scale.rows ? scale_rows - right_scale.rows : 0;
            final_col_start_c = -col_offset + scale_cols > 0 ? scale_cols - right_scale.cols : 0;

            right.rowRange(final_row_start_c, scale_rows)
                .colRange(final_col_start_c, scale_cols)
                .copyTo(right_scale);
        }
    }

    void remap_second(cv::Mat &src, cv::Mat &dst)
    {
        int _rows = output_trans_second ? output_width_second : output_height_second; // 1080
        int _cols = output_trans_second ? output_height_second : output_width_second; // 1920
        int scale_rows = output_scale_second * _rows;                   // 540
        int scale_cols = output_scale_second * _cols;                   // 960
        cv::Mat left, right, small;
        resize_uniform(src, small, cv::Size(scale_cols, scale_rows));
        if (output_trans_second)
        {
            cv::rotate(small, left, cv::ROTATE_90_COUNTERCLOCKWISE); // 逆时针
            if (output_copy_second)
            {
                cv::rotate(small, right, cv::ROTATE_90_CLOCKWISE); // 顺时针
            }
        }
        else
        {
            left = small;
            if (output_copy_second)
            {
                right = small;
            }
        }
        scale_rows = left.rows;
        scale_cols = left.cols;
        int screen_cols_ = output_copy_second ? screen_cols_second / 2 : screen_cols_second;
        int row_start = (screen_rows_second - scale_rows) / 2 < 0 ? 0 : (screen_rows_second - scale_rows) / 2;
        int col_start = (screen_cols_second - scale_cols) / 2 < 0 ? 0 : (screen_cols_second - scale_cols) / 2;
        int row_end = row_start + scale_rows + row_offset_second > screen_rows_second ? screen_rows_second : row_start + scale_rows + row_offset;
        dst = cv::Mat(cv::Size(
                          screen_cols_second,
                          screen_rows_second),
                      src.type(), cv::Scalar(0));
        // 防止超范围
        int final_row_start = row_start + row_offset_second > screen_rows_second ? screen_rows_second : row_start + row_offset_second;
        int final_col_start = col_start + col_offset_second > screen_cols_ ? screen_cols_ : col_start + col_offset_second;
        int final_row_end = row_start + row_offset_second + scale_rows > screen_rows_second ? screen_rows_second : row_start + row_offset_second + scale_rows;
        int final_col_end = col_start + col_offset_second + scale_cols > screen_cols_ ? screen_cols_ : col_start + col_offset_second + scale_cols;
        cv::Mat left_scale = dst.rowRange(final_row_start, final_row_end)
                                 .colRange(final_col_start, final_col_end);
        int final_row_start_c = row_offset_second > left_scale.rows ? scale_rows : 0;
        int final_col_start_c = col_offset_second > left_scale.cols ? scale_cols : 0;
        int final_row_end_c = row_offset_second + scale_rows > left_scale.rows ? left_scale.rows : scale_rows;
        int final_col_end_c = col_offset_second + scale_cols > left_scale.cols ? left_scale.cols : scale_cols;
        left.rowRange(final_row_start_c, final_row_end_c)
            .colRange(final_col_start_c, final_col_end_c)
            .copyTo(left_scale);
        if (output_copy_second)
        {
            // 防止超范围
            final_row_start = row_start + row_offset_second > screen_rows_second ? dst.rows : row_start + row_offset_second;
            final_col_start = col_start + col_offset_second > screen_cols_ ? dst.cols : col_start + col_offset_second + screen_cols_;
            final_row_end = row_start + row_offset_second + scale_rows > screen_rows_second ? dst.rows : row_start + row_offset_second + scale_rows;
            final_col_end = col_start + col_offset_second + scale_cols > screen_cols_ ? dst.cols : col_start + col_offset_second + scale_cols + screen_cols_;
            cv::Mat right_scale = dst.rowRange(final_row_start, final_row_end)
                                      .colRange(final_col_start, final_col_end);
            final_row_start_c = row_offset_second + scale_rows > right_scale.rows ? scale_rows - right_scale.rows : 0;
            final_col_start_c = col_offset_second + scale_cols > right_scale.cols ? scale_cols - right_scale.cols : 0;

            right.rowRange(final_row_start_c, scale_rows)
                .colRange(final_col_start_c, scale_cols)
                .copyTo(right_scale);
        }
    }

    int resize_uniform(cv::Mat &src, cv::Mat &dst, cv::Size dst_size)
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
            dst = dst_;
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
};