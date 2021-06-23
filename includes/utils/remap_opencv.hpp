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

    void remap(cv::Mat &src, cv::Mat &dst)
    {
        cv::Mat src_resize;
        resize_uniform(src, src_resize, cv::Size(1920, 720));
        int rows = src_resize.rows;
        int cols = src_resize.cols;
        // 加上黑边
        cv::Mat src_extend(cv::Size(cols, rows + 1), src_resize.type(), cv::Scalar(0));
        src_resize.copyTo(src_extend.rowRange(0, rows));
        cv::remap(src_extend, dst, remap_matrix[1], remap_matrix[0], cv::INTER_NEAREST);
    }

private:
    int load_remap_mat(int rows, int cols)
    {
        cv::Mat offset_matrix(cv::Size(cols, rows), CV_32FC2, offset_mat);
        remap_matrix.clear();
        //分离通道
        cv::split(offset_matrix, remap_matrix);
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
};
