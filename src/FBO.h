#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>

class FBO {
public:
	GLuint rayID{};
	GLuint displayID{};
	GLuint rayTexture{};
	GLuint displayTexture{};

	FBO(const int width, const int height) {
		glGenFramebuffers(1, &rayID);
		glGenFramebuffers(1, &displayID);
		resize(width, height);
	}

	void resize(const int width, const int height) {
		glDeleteTextures(1, &rayTexture);
		glDeleteTextures(1, &displayTexture);

		// raytrace buffer
		glGenTextures(1, &rayTexture);
		glBindTexture(GL_TEXTURE_2D, rayTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, rayID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rayTexture, 0);

		// display buffer
		glGenTextures(1, &displayTexture);
		glBindTexture(GL_TEXTURE_2D, displayTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindFramebuffer(GL_FRAMEBUFFER, displayID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, displayTexture, 0);
	}

	void bindRay() const {
		glBindFramebuffer(GL_FRAMEBUFFER, rayID);
	}

	void bindDisplay() const {
		glBindFramebuffer(GL_FRAMEBUFFER, displayID);

	}

	void unBind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void bindRayTexture() const {
		glBindTexture(GL_TEXTURE_2D, rayTexture);
	}

	void bindDisplayTexture() const {
		glBindTexture(GL_TEXTURE_2D, displayTexture);
	}

	static void unBindTexture() {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void deleteFBO() const {
		glDeleteTextures(1, &rayTexture);
		glDeleteFramebuffers(1, &rayID);
		glDeleteTextures(1, &displayTexture);
		glDeleteFramebuffers(1, &displayID);
	}
};

#endif