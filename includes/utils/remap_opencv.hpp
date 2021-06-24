#pragma once

#include <common/settings.h>
#include <opencv2/opencv.hpp>
#include <vector>

// libjibianv2.so中保存的反向映射矩阵
// offset_mat的格式
// 映射结果图像(2160*1920)中第y行，第x列像素值为原图(1920x721)中第y*2160+x+1行，第y*2160+x列的像素值
// 原图本应为1920x720,最后一行加了一条黑边变成了1920x721，这只是为了方便在映射时取到黑色的像素
extern float offset_mat[2 * FINAL_COLS * FINAL_ROWS + 1];

// 加载反向映射矩阵
class Remap
{
public:
    Remap()
    {
        load_remap_mat(FINAL_ROWS, FINAL_COLS);
    }

    void remap_v2(cv::Mat &src, cv::Mat &dst)
    {
        cv::Mat dst_(cv::Size(FINAL_COLS, FINAL_ROWS), src.type(), cv::Scalar(0));
        int rows = src.rows;
        int cols = src.cols;
        int elm_size = dst.elemSize();
        for (auto &offset : remap_offset)
        {
            int dst_x = offset.first.first;
            int dst_y = offset.first.second;
            int src_x = offset.second.first;
            int src_y = offset.second.second;
            memcpy(
                dst_.data + (dst_x + dst_y * FINAL_COLS) * elm_size,
                src.data + (src_x + src_y * cols) * elm_size,
                elm_size);
        }
        dst = dst_;
        cv::flip(dst.colRange(0, FINAL_COLS / 2), dst.colRange(FINAL_COLS / 2, FINAL_COLS), -1);
    }

    void remap(cv::Mat &src, cv::Mat &dst)
    {
        // cv::Mat src_resize;
        // resize_uniform(src, src_resize, cv::Size(1920, 720));
        int rows = src.rows;
        int cols = src.cols;
        // 加上黑边
        dst = cv::Mat(cv::Size(FINAL_COLS, FINAL_ROWS), src.type(), cv::Scalar(0));
        cv::Mat src_extend(cv::Size(cols, rows + 1), src.type(), cv::Scalar(0));
        src.copyTo(src_extend.rowRange(0, rows));
        cv::Mat dst_roi = dst(dest_roi_range);
        cv::remap(src_extend, dst_roi, remap_matrix[1](dest_roi_range), remap_matrix[0](dest_roi_range), cv::INTER_NEAREST);
        cv::flip(dst_roi, dst(half_dest_roi_range), -1);
    }

private:
    int load_remap_mat(int rows, int cols)
    {
        cv::Mat offset_matrix(cv::Size(cols, rows), CV_32FC2, offset_mat);
        cv::Mat offset_matrix_half = offset_matrix.colRange(0, cols / 2);
        remap_matrix.clear();
        //分离通道
        cv::split(offset_matrix_half, remap_matrix);
        int min_i = FINAL_ROWS, min_j = FINAL_COLS;
        int max_i = 0, max_j = 0;
        for (int i = 0; i < FINAL_ROWS; ++i)
        {
            for (int j = 0; j < FINAL_COLS / 2; ++j)
            {
                float y = offset_mat[2*  (i * FINAL_COLS + j)];
                float x = offset_mat[2*  (i * FINAL_COLS + j) + 1];
                if (y != 720.0 && x != 0.0)
                {
                    if (min_i > i)
                    {
                        min_i = i;
                    }
                    if (min_j > j)
                    {
                        min_j = j;
                    }
                    if (max_i < i)
                    {
                        max_i = i;
                    }
                    if (max_j < j)
                    {
                        max_j = j;
                    }
                    remap_offset.push_back(
                        std::make_pair(
                            std::make_pair(j, i),
                            std::make_pair(x, y)));
                }
            }
        }
        dest_roi_range = cv::Rect(cv::Point(min_j, min_i), cv::Point(max_j, max_i));
        half_dest_roi_range = cv::Rect(cv::Point(FINAL_COLS - max_j - 1, min_i), cv::Point(FINAL_COLS - min_j - 1, max_i));
        return 0;
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

private:
    std::vector<cv::Mat> remap_matrix;
    std::vector<std::pair<std::pair<int, int>, std::pair<float, float>>> remap_offset;
    cv::Rect dest_roi_range, half_dest_roi_range;
};
