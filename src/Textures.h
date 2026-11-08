#ifndef TEXTURES_H
#define TEXTURES_H

#include <glad/glad.h>

class Textures {
public:
	GLuint rayTexture{};
	GLuint displayTexture{};

	Textures(const int width, const int height) {
		resize(width, height);
	}

	void resize(const int width, const int height) {
		glDeleteTextures(1, &rayTexture);
		glDeleteTextures(1, &displayTexture);

		// raytrace texture
		glCreateTextures(GL_TEXTURE_2D, 1, &rayTexture);
		glTextureStorage2D(rayTexture, 1, GL_RGBA32F, width, height);
		glBindImageTexture(0, rayTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		// display texture
		glCreateTextures(GL_TEXTURE_2D, 1, &displayTexture);
		glTextureStorage2D(displayTexture, 1, GL_RGBA8, width, height);
		glTextureParameteri(displayTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(displayTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindImageTexture(1, displayTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	}

	void deleteTextures() const {
		glDeleteTextures(1, &rayTexture);
		glDeleteTextures(1, &displayTexture);
	}
};

#endif