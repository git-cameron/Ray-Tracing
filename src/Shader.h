#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

class Shader {
public:
	GLuint program;

	Shader(const char* vertexCode, const char* fragmentCode);
	void use();
	void deleteShader();
};

#endif