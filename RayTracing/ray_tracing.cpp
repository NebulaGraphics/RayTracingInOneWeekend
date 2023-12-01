#include "./opencv2/opencv.hpp"
#include "./opencv2/core/opengl.hpp"
#include "./opencv2/highgui/highgui_c.h"
#include "rt_weekend.h"
#include <cmath>
#include "vec3.h"
#include "ray.h"
#include "interval.h"

#pragma comment(lib,"opencv_world470d.lib")

using color = vec3;

void ray_tracing(cv::Mat* buffer, bool* isRunning, int sampler_per_pixel, int bounce);
void init_world();

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

    init_world();

    std::thread tracing_thread(ray_tracing, buffer, isRunning, 1, 50);

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

    cv::destroyAllWindows();

    delete isRunning;
    delete buffer;

    return 0;
}

struct float2
{
    float u, v;
};

struct resolution
{
    float width, height;
};

struct varying
{
    float2 uv;
    resolution resolution;
    int bounce;
};


float linear_to_gamma(float value);
color pixel_shader(varying varying);

interval color_intensity(0.000, 0.999);

void ray_tracing(cv::Mat* buffer, bool* isRunning, int sample_per_pixel = 1, int bounce = 50)
{
    auto target_height = buffer->rows;
    auto target_width = buffer->cols;
    auto channels = buffer->channels();
    

    while (*isRunning)
    {
        for (int j = 0; j < target_height; ++j) {
            auto horizonal = buffer->ptr<uchar>(j);
            for (int i = 0; i < target_width * channels; i += channels) {

                color final_color;
                for (int sample = 0; sample < sample_per_pixel; ++sample)
                {
                    auto color = pixel_shader(varying
                        {
                            // uv
                            float2{ (float)((float)i / channels / target_width + random_double() / (target_width * channels)), 1.0f - (float)((float)j / target_height + random_double() / target_height)},
                            // resolution
                            resolution { (float)target_width, (float)target_height},
                            // bounce
                            bounce
                        }
                    );

                    final_color += color;
                }

                final_color /= sample_per_pixel;
               
                //BGR£¨À¶¡¢ÂÌ¡¢ºì£©
                horizonal[i]      =   (uchar)(linear_to_gamma(color_intensity.clamp(final_color.z())) * 255);
                horizonal[i + 1]  =   (uchar)(linear_to_gamma(color_intensity.clamp(final_color.y())) * 255);
                horizonal[i + 2]  =   (uchar)(linear_to_gamma(color_intensity.clamp(final_color.x())) * 255);
;            }
        }

    }

    std::cout << " rendering end " << std::endl;
}

/**
* 
*   ray tracing in one weekend
* 
* 
* 
**/

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "hittable_list.h"
#include "sphere.h"

camera main_camera(
    vec3 { 0, 0, 3},               // position
    view_port{ 0, 0, 1, 1},        // view port 
    0.1f,                          // near
    10000.f,                       // far
    60.f,                          // fov
    1280.f / 720.f                 // aspect ratio
);

color sky_color(const ray& r)
{
    vec3 unit_direction = normalize(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

float linear_to_gamma(float value)
{
    return pow(value, 1 / 2.2);
}

/// <summary>
/// view port is default as (0, 1)
/// </summary>
/// <param name="uv"></param>
/// <returns></returns>

ray get_ray(varying varying)
{
    auto view_height = tan(main_camera.fov_rad() / 2) * main_camera.near() * 2;
    auto view_width = view_height * main_camera.aspect_ratio();

    auto pixel_world_pos = vec3
    {
        (varying.uv.u - 0.5) * view_width ,
        (varying.uv.v - 0.5) * view_height,
        // camera is look to -z
        (float)main_camera.position().z() - main_camera.near()
    };

    vec3 ray_direction{ pixel_world_pos - main_camera.position() };
    ray r(main_camera.position(), ray_direction);
    return r;
}

color ray_cast(const ray& r, const hittable& world, const int bounce) {
    
    if (bounce <= 0) 
    {
        return color(0, 0, 0);
    }

    hit_record rec;

    if (world.hit(r, interval(0.001, infinity), rec)) 
    {
        vec3 direction = rec.normal + random_unit_vector();
        return 0.5 * ray_cast(ray(rec.p, direction), world, bounce - 1);
    }

    return sky_color(r);
}


// World

hittable_list world;

void init_world()
{
    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));
}

color pixel_shader(varying varying) {

    auto r = get_ray(varying);

    return ray_cast(r, world, varying.bounce);
}

