#ifndef SSBO_H
#define SSBO_H

#include <glad/glad.h>
#include <vector>
#include "Object.h"
#include "BVH.h"

class SSBO {

public:
	GLuint ID{};

	SSBO() {
		glGenBuffers(1, &ID);
	}

	void buildTriangle(std::vector<Triangle> triangles) {
		if (triangles.empty()) return;
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Triangle) * triangles.size(), &triangles[0], GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ID);
	}

	void buildNodes(std::vector<BVHNode> nodes) {
		if (nodes.empty()) return;
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVHNode) * nodes.size(), &nodes[0], GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ID);
	}

	void buildTriIndex(std::vector<int> triIndex) {
		if (triIndex.empty()) return;
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * triIndex.size(), &triIndex[0], GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ID);
	}

	void bind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID);
	}

	void unBind() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void deleteSSBO() {
		glDeleteBuffers(1, &ID);
	}

};


#endif