#pragma once
#include "./includes.h"

class ColorRGBA {
public:
	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;
private:

	bool initWithString(const std::string& hexColor) {
		if (hexColor.length() != 8) return false;

		for (char c : hexColor)
			if (!std::isxdigit(c))
				return false;

		try {
			r = static_cast<GLubyte>(std::stoul(hexColor.substr(0, 2), nullptr, 16));
			g = static_cast<GLubyte>(std::stoul(hexColor.substr(2, 2), nullptr, 16));
			b = static_cast<GLubyte>(std::stoul(hexColor.substr(4, 2), nullptr, 16));
			a = static_cast<GLubyte>(std::stoul(hexColor.substr(6, 2), nullptr, 16));
		}
		catch (const std::exception& e) {
			return false;
		}

		return true;
	}

	ColorRGBA() {}
public:
	static ColorRGBA* create(const std::string& hexColor) {
		auto ret = new ColorRGBA();
		if (ret && ret->initWithString(hexColor))
			return ret;
		delete ret;
		return nullptr;
	}
};