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

	}

private:

	vec3 _position;
	float _near, _far, _fov, _aspect_ratio;

	vec3 _forward = vec3(0, 0, -1);
	vec3 _right = vec3(1, 0, 0);
	vec3 _up = vec3(0, 1, 0);

	view_port _view_port;

};