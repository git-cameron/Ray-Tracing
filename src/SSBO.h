#ifndef SSBO_H
#define SSBO_H

#include <glad/glad.h>
#include <vector>
#include "Object.h"

class SSBO {

public:
	GLuint ID{};

	SSBO() {
		glGenBuffers(1, &ID);
	}

	void updateTriangle(const std::vector<Triangle>& triangles) const {
		if (triangles.empty()) return;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Triangle) * triangles.size(), triangles.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ID);
	}

	void updateNodes(const std::vector<BVHNode>& nodes) const {
		if (nodes.empty()) return;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVHNode) * nodes.size(), nodes.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ID);
	}

	void updateTriIndex(const std::vector<int>& triIndex) const {
		if (triIndex.empty()) return;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * triIndex.size(), triIndex.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ID);
	}

	void deleteSSBO() const {
		glDeleteBuffers(1, &ID);
	}
};


#endif