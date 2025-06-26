#pragma once
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "config.hpp"

inline void ScaleCommand(const char* preset)
{
	if (strcmp(preset, "normal") == 0) {
		config::reset();
		return;
	}

	if (strcmp(preset, "small") == 0) {
		config::scale = 0.3f;
		config::speed = 0.6f;
		config::fov = 1.2f;
		return;
	}

	if (strcmp(preset, "giant") == 0) {
		config::scale = 1.5f;
		config::speed = 1.1f;
		config::fov = 1.65f;
		return;
	}

	printf("Invalid arguments, valid arguments: small | normal | giant\n");
}

inline void CommandHandler(const char* command, const char* args)
{
	if (strcmp(command, "exit") == 0) {
		config::exit = true;
	}
	else if (strcmp(command, "reset") == 0) {
		config::reset();
	}
	else if (strcmp(command, "set-size") == 0) {
		ScaleCommand(args);
	}
	else if (strcmp(command, "customscale") == 0) {
		config::scale = atof(args);
	}
	else {
		printf("Invalid command, valid commands: set-size | exit\n");
	}
}