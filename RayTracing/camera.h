#pragma once
#include "ray.h"
#include "vec3.h"
#include <math.h>
#include "rt_weekend.h"

struct view_port
{
	float x, y, width, height;
};

class camera {

public :
	camera() = delete;
	camera(vec3 position, view_port view_port, float near, float far, float fov, float aspect_ratio):
		_position(position),
		_view_port(view_port),
		_fov(fov),
		_near(near),
		_far(far),
		_aspect_ratio(aspect_ratio)
	{

	}

	void move(vec3 offset)
	{
		_position += offset;
	}

	float aspect_ratio() const { return _aspect_ratio; }
	float fov() const { return _fov; }
	void set_fov(float fov) { _fov = fov; }
	float fov_rad() const { return degrees_to_radians(_fov); }
	vec3 position() const { return _position; }
	void set_position(const vec3& pos) { _position = pos; }
	float near() const { return _near; }
	double focus_distance() const { return focus_dist; }
	view_port get_view_port() const { return _view_port; }

	vec3 forward() const { return _forward; }
	vec3 right() const { return _right; }
	vec3 up() const { return _up; }

	void look_at(vec3 target, vec3 world_up = vec3(0, 1, 0))
	{
		// always look to -z direction in view space;
		_forward = normalize(target - _position);
		
		// use right-hand coordinate
		_right = normalize(cross(world_up, -_forward));

		_up = normalize(cross(-_forward, _right));

		//focus_dist = (target - _position).length();
	}

	void set_defocus_angle(double angle)
	{
		// TODO: fix this bug
		defocus_angle = angle * 0.06;
	}

	void set_focus_dist(double dist)
	{
		focus_dist = dist;
	}

	double get_defocus_angle() const
	{
		return defocus_angle;
	}

	point3 defocus_disk_sample() const {

		
		// Calculate the camera defocus disk basis vectors.
		auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
		auto defocus_disk_u = _right * defocus_radius;
		auto defocus_disk_v = _up * defocus_radius;

		// Returns a random point in the camera defocus disk.
		auto p = random_in_unit_disk();
		return _position + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
	}

private:

	vec3 _position;
	float _near, _far, _fov, _aspect_ratio;

	vec3 _forward = vec3(0, 0, -1);
	vec3 _right = vec3(1, 0, 0);
	vec3 _up = vec3(0, 1, 0);

	view_port _view_port;

	double defocus_angle = 0;  // Variation angle of rays through each pixel
	double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

};