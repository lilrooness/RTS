#pragma once

#include <glm.hpp>
#include <fstream>
#include <string>

const std::string CLEAR_COLOR = "clearColor";
const std::string CAMERA_POS = "cameraPos";
const std::string TANK_SPEED = "tankSpeed";
const std::string WINDOW_SIZE = "windowSize";

struct Settings {
	glm::vec4 clearColor;
	glm::vec3 cameraPos;
	float tankSpeed;
	int windowWidth;
	int windowHeight;
};

void load_settings_file(Settings *settings, const char* filename) {
	std::ifstream f(filename);

	std::string keyword;

	while(f >> keyword) {
		if (keyword == CLEAR_COLOR) {
			f >> settings->clearColor.x >> settings->clearColor.y >> settings->clearColor.z >> settings->clearColor.w;
		}
		else if (keyword == CAMERA_POS) {
			f >> settings->cameraPos.x >> settings->cameraPos.y >> settings->cameraPos.z;
		}
		else if (keyword == TANK_SPEED) {
			f >> settings->tankSpeed;
		}
		else if (keyword == WINDOW_SIZE) {
			f >> settings->windowWidth >> settings->windowHeight;
		}
	}
}
