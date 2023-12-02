#include "./opencv2/opencv.hpp"
#include "./opencv2/core/opengl.hpp"
#include "./opencv2/highgui/highgui.hpp"
#include "./opencv2/highgui/highgui_c.h"
#include "rt_weekend.h"
#include <cmath>
#include "vec3.h"
#include "ray.h"
#include "interval.h"
#include "material.h"

#if _DEBUG
#pragma comment(lib,"opencv_world481d.lib")
#else
#pragma comment(lib,"opencv_world481.lib")
#endif


using color = vec3;

void ray_tracing(cv::Mat* buffer, int sampler_per_pixel, int bounce);
void init_world();
void move_camera(float x, float y, float z);

std::atomic<bool> isRunning = true;

int main(int argc, char* argv[])
{

#if _DEBUG

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#endif

	cv::String window_name = "Raytracing in one weekend";

	cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);

	auto buffer = new cv::Mat(1080, 1920, CV_8UC3);

	int keyCode = 0;

	init_world();

	std::thread tracing_thread(ray_tracing, buffer, 10, 50);

	while (keyCode != 27)
	{
		cv::imshow(window_name, *buffer);
		keyCode = cv::pollKey();

		if (keyCode == 119)
		{
			// forward 
			move_camera(0, 0, -0.01);
		}
		else if (keyCode == 97) // A
		{
			// left
			move_camera(-0.01, 0, 0);
		}
		else if (keyCode == 115)
		{
			// backward
			move_camera(0, 0, 0.01);
		}
		else if (keyCode == 100)
		{
			// right
			move_camera(0.01, 0, 0);
		}

		
	}

	isRunning = false;

	if (tracing_thread.joinable())
	{
		tracing_thread.join();
	}

	cv::destroyAllWindows();

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

void ray_tracing(cv::Mat* buffer, int sample_per_pixel = 1, int bounce = 50)
{
	auto target_height = buffer->rows;
	auto target_width = buffer->cols;
	auto channels = buffer->channels();


	while (isRunning)
	{
		auto current_clock = clock();
		for (int j = 0; j < target_height; ++j) {
			auto horizonal = buffer->ptr<uchar>(j);
			for (int i = 0; i < target_width * channels; i += channels) {

				color final_color;
				for (int sample = 0; sample < sample_per_pixel; ++sample)
				{
					if (!isRunning)
					{
						break;
					}

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
				horizonal[i] = (uchar)(linear_to_gamma(color_intensity.clamp(final_color.z())) * 255);
				horizonal[i + 1] = (uchar)(linear_to_gamma(color_intensity.clamp(final_color.y())) * 255);
				horizonal[i + 2] = (uchar)(linear_to_gamma(color_intensity.clamp(final_color.x())) * 255);
				
			}
		}

		std::cout << "time cost per frame:" << (clock() - current_clock) / 1000.f << " s" << std::endl;
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
	vec3{ -2, 2, 1 },               // position
	view_port{ 0, 0, 1, 1 },        // view port 
	0.1f,                          // near
	10000.f,                       // far
	90.f,                          // fov
	1280.f / 720.f                 // aspect ratio
);

void move_camera(float x, float y, float z)
{
	main_camera.move(vec3(x, y, z));
}

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

	auto camera_up = main_camera.up();
	auto camera_right = main_camera.right();
	auto camera_forward = -main_camera.forward();

	assert(approx_equal(0, dot(camera_up, camera_right)));
	assert(approx_equal(0, dot(camera_up, camera_forward)));
	assert(approx_equal(0, dot(camera_right, camera_forward)));

	auto sign_u = varying.uv.u > 0.5 ? 1 : -1;
	auto sign_v = varying.uv.v > 0.5 ? 1 : -1;
	auto pixel_view_pos = vec3
	{
		(varying.uv.u - 0.5) * view_width ,
		(varying.uv.v - 0.5) * view_height ,
		// camera is look to -z
		-main_camera.near()
	};

	vec3 ray_direction_view{ normalize(pixel_view_pos)};
	vec3 ray_direction(
		dot(vec3(camera_right.x(), camera_up.x(), camera_forward.x()),
			ray_direction_view),
		dot(vec3(camera_right.y(), camera_up.y(), camera_forward.y()),
			ray_direction_view),
		dot(vec3(camera_right.z(), camera_up.z(), camera_forward.z()),
			ray_direction_view)
	);
	ray r(main_camera.position(), normalize(ray_direction));
	return r;
}

color ray_cast(const ray& r, const hittable& world, const int bounce) {

	if (bounce <= 0 || !isRunning)
	{
		return color(0, 0, 0);
	}

	hit_record rec;

	if (world.hit(r, interval(0.001, infinity), rec))
	{
		ray scattered;
		color attenuation;
		if (rec.mat->scatter(r, rec, attenuation, scattered))
		{
			return attenuation * ray_cast(scattered, world, bounce - 1);
		}

		return color(0, 0, 0);
	}

	return sky_color(r);
}


// World

hittable_list world;

void init_world()
{
	auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
	auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
	auto material_left = make_shared<dielectric>(1.5);
	auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

	world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
	world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
	world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.4, material_left));
	world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));
	main_camera.set_fov(20);
	main_camera.set_position(vec3(-2, 2, 1));
	main_camera.look_at(vec3(0, 0, -1));
}

color pixel_shader(varying varying) {

	auto r = get_ray(varying);

	return ray_cast(r, world, varying.bounce);
}

