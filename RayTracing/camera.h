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

	float aspect_ratio() const { return _aspect_ratio; }
	float fov() const { return _fov; }
	float fov_rad() const { return degrees_to_radians(_fov); }
	vec3 position() const { return _position; }
	float near() const { return _near; }
	view_port get_view_port() const { return _view_port; }

private:

	vec3 _position;
	float _near, _far, _fov, _aspect_ratio;

	view_port _view_port;

};