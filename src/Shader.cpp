#include "Shader.h"

Shader::Shader(const char* vertexCode, const char* fragmentCode) {
	GLuint vertex, fragment;
		
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexCode, 0);
	glCompileShader(vertex);

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentCode, 0);
	glCompileShader(fragment);

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
		
	}

void Shader::use() {
	glUseProgram(program);
}

void Shader::deleteShader() {
	glDeleteProgram(program);
}
