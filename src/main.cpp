
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>

#include <common/tic_toc.h>
#include <common/settings.h>
#include <pipelines/thread_manage.h>
#include <common/yaml.hpp>
#include <utils/image_utils.hpp>
#include <opencv2/opencv.hpp>
#include <pipelines/pipelines.h>


const char *keys =
    {
        "{ @1           |                     | movie file  | }"
        "{ help         | false               | show usage  | }"
        "{ imshow       | false               |   use imshow| }"
        "{ rate         | 1.0                 |     fps rate| }"};
bool global_run_flag;

inline char GetInput()
{
    //fd_set 为long型数组
    //其每个元素都能和打开的文件句柄建立联系
    fd_set rfds;
    struct timeval tv;
    char c = '\0';

    //将　rfds数组清零
    FD_ZERO(&rfds);
    //将rfds的第0位置为１，这样fd=1的文件描述符就添加到了rfds中
    //最初　rfds为00000000,添加后变为10000000
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 10; //设置等待超时时间

    //检测键盘是否有输入
    //由内核根据io状态修改rfds的内容，来判断执行了select的进程哪个句柄可读
    if (select(1, &rfds, NULL, NULL, &tv) > 0)
    {
        c = getchar();
        return c;
    }

    //没有数据返回n
    return '\0';
}

int main(int argc, char **argv)
{
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.get<bool>("help"))
    {
        
        printf("\nUsage: \n"
               "./zntk_ar <path to the movie> -<method to display>"
               "Example of the zntk_ar \n"
               "./zntk_ar rtp:9999 -gtk\n"
               "./zntk_ar ../MOT16-13-raw.webm -gtk\n");
        return 0;
    }

    global_run_flag = true;
    PipeLines pipelines;
    std::string video_name = parser.get<std::string>(0);
    double fps_rate = parser.get<double>("rate");
    DRAW_TYPE show_type = DRAW_BY_FC;
    if (parser.get<bool>("imshow"))
    {
        show_type = DRAW_BY_IMSHOW;
    }
    bool resource = false;
    std::cout << "video_name " << video_name << std::endl;
    if (!video_name.empty())
    {
        resource = pipelines.init_producer(video_name, fps_rate);
    }
    
    if (!resource)
    {
        std::cout << "video error! " << std::endl;
        exit(-1);
    }
    pipelines.init_processor();
    pipelines.init_consumer(show_type);
    pipelines.start();

    auto exit = [](int) {
        global_run_flag = false;
    };
    ::signal(SIGINT, exit);
    ::signal(SIGABRT, exit);
    ::signal(SIGSEGV, exit);
    ::signal(SIGTERM, exit);
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) ||                    \
    defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || \
    defined(WINCE) || defined(_WIN32_WCE)
    ::signal(SIGBREAK, exit);
#else
    ::signal(SIGHUP, exit);
#endif
    while (global_run_flag)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        char ipt = GetInput();
        if (ipt == 'x')
        {
            global_run_flag = false;
        }
    }
    
    pipelines.stop();
    double benchmark = -1;
    ImageUtils::undistortion_benchmark(benchmark);
    std::flush(std::cout);
    std::cout << "----------------------------" << std::endl;
    std::cout << " avg result: " << benchmark << std::endl;
    std::cout << "----------------------------" << std::endl;
    return 0;
}