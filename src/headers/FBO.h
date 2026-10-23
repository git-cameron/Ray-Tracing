#ifndef FBO_H
#define FBO_H

#include <glad/glad.h>

class FBO {
public:
	GLuint ID{};
	GLuint texture{};
	GLuint RBO{};

	FBO(int width, int height);

	void resize(int width, int height);
	void bind();
	void unBind();
	void deleteFBO();
	void bindTexture();
	void unBindTexture();
};

#endif