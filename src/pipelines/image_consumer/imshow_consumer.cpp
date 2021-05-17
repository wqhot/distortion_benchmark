#include <pipelines/image_consumer/imshow_consumer.h>

void ImshowConsumer::pop(cv::Mat &image)
{
    cv::imshow("consumer", image);
    cv::waitKey(1);
}
