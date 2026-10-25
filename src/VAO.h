#ifndef VAO_H
#define VAO_H

#include <glad/glad.h>
#include "VBO.h"

class VAO {
public:
	GLuint ID{};

	VAO() {
		glGenVertexArrays(1, &ID);
	}

	void linkVBO(VBO VBO, GLuint layout) {
		VBO.bind();
		glVertexAttribPointer(layout, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(layout * 2 * sizeof(float)));
		glEnableVertexAttribArray(layout);
		VBO.unBind();
	}

	void bind() {
		glBindVertexArray(ID);
	}

	void unBind() {
		glBindVertexArray(0);
	}

	void deleteVAO() {
		glDeleteVertexArrays(1, &ID);
	}
};

#endif