#ifndef VBO_H
#define VBO_H

#include <glad/glad.h>

class VBO {
public:
	GLuint ID{};
	
	VBO(const GLfloat* vertices, const GLsizeiptr size) {
		glGenBuffers(1, &ID);
		glBindBuffer(GL_ARRAY_BUFFER, ID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	void bind() const {
		glBindBuffer(GL_ARRAY_BUFFER, ID);
	}

	void unBind() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void deleteVBO() const {
		glDeleteBuffers(1, &ID);
	}
};

#endif