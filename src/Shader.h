#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

class Shader {
public:
	GLuint program{};

	Shader(const char* shaderCode) {
		const GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &shaderCode, nullptr);
		glCompileShader(shader);

		program = glCreateProgram();
		glAttachShader(program, shader);
		glLinkProgram(program);

		glDeleteShader(shader);
	}

	void use() const {
		glUseProgram(program);
	}

	void deleteShader() const {
		glDeleteProgram(program);
	}
};

#endif