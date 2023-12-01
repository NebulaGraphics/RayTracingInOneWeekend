#include "./opencv2/opencv.hpp"
#include "./opencv2/core/opengl.hpp"
#include "./opencv2/highgui/highgui_c.h"
#include <cmath>

#pragma comment(lib,"opencv_world470d.lib")

void ray_tracing(cv::Mat* buffer, bool* isRunning);

int main(int argc, char* argv[])
{

#if _DEBUG

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#endif

    cv::String window_name = "Raytracing in one weekend";

    cv::namedWindow(window_name, cv:: WINDOW_AUTOSIZE);

    auto buffer = new cv::Mat(720, 1280, CV_8UC3);

    auto keyCode = 0;

    bool* isRunning = new bool { true };

    std::thread tracing_thread(ray_tracing, buffer, isRunning);

    tracing_thread.detach();

    while (keyCode != 27)
    {
        cv::imshow(window_name, *buffer);
        keyCode = cv::pollKey();
    }
   
    *isRunning = false;
    
    if (tracing_thread.joinable())
    {
        tracing_thread.join();
    }

    cv::waitKey();
    cv::destroyAllWindows();

    delete isRunning;
    delete buffer;

    return 0;
}

struct color
{
    uchar* r, *b, *g;
};

void pixel_shader(color* color);

void ray_tracing(cv::Mat* buffer, bool* isRunning)
{
    auto target_height = buffer->rows;
    auto target_width = buffer->cols;
    auto channels = buffer->channels();

    while (*isRunning)
    {
        for (int j = 0; j < target_height; ++j) {
            auto horizonal = buffer->ptr<uchar>(j);
            for (int i = 0; i < target_width * channels; i += channels) {
                horizonal[i] = rand() * 255;
                horizonal[i + 1] = rand() * 255;
                horizonal[i + 2] = rand() * 255;
;            }
        }

    }
}

