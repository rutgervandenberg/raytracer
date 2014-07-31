#pragma once
#include <cmath>
#include "vector.hpp"
#include "config.hpp"

const float pi = 3.14159265f;

/**
*	Perform camera transformations
*	(z axis: positive is up, zero is floor)
*/
struct Camera {
	vec4 pos;
	vec4 dir;
	vec4 rotation[4];
	vec4 movespeed;
	float turnspeed;

	float straight;
	float strafe;
	float vert;

	float xturn;
	float yturn;

	float xdir, ydir;

	Camera(Config& config) {
	}

	Camera(vec4 pos, float xdir, float ydir) {
		update(pos, xdir, ydir);
		movespeed = vec4s(0.1f);
		turnspeed = (0.01f);
		straight = 0;
		strafe = 0;
		xturn = 0;
		yturn = 0;
	}

	void move() {
		float xf = straight * cos(xdir + 0.5f * pi) + strafe * cos(xdir);
		float yf = straight * sin(xdir + 0.5f * pi) + strafe * sin(xdir);
		float zf = vert;

		xdir += xturn * turnspeed;
		ydir += yturn * turnspeed;

		pos = pos + vec4(xf, yf, zf, 0) * movespeed;

		//rotation[0] = vec4(1, 0, 0, 0);
		//rotation[1] = vec4(0, cos(ydir), -sin(ydir), 0);
		//rotation[2] = vec4(0, sin(ydir), cos(ydir), 0);
		//rotation[3] = vec4(0, 0, 0, 1);
	}

	void update(vec4 pos, float xdir, float ydir) {
		this->pos = pos;

		rotation[0] = vec4(1, 0, 0, 0);
		rotation[1] = vec4(0, cos(ydir), -sin(ydir), 0);
		rotation[2] = vec4(0, sin(ydir), cos(ydir), 0);
		rotation[3] = vec4(0, 0, 0, 1);

		//rotation[0] = vec4(cos(xdir), -sin(xdir), 0, 0);
		//rotation[1] = vec4(sin(xdir), cos(ydir) + cos(xdir), -sin(ydir), 0);
		//rotation[2] = vec4(0, sin(ydir), cos(ydir), 0);
		//rotation[3] = vec4(0, 0, 0, 1);
	}
};