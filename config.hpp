#pragma once

namespace config
{
	bool exit = false;

	float scale = 1.0f;
	float fov = 1.570796371; // dynamic, so should ideally be different
	float weaponFov = 0.8726646304;
	float speed = 1.f;

	void reset()
	{
		scale = 1.f;
		fov = 1.570796371;
		weaponFov = 0.8726646304;
		speed = 1.f;
	}
};