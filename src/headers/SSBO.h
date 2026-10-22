#ifndef SSBO_H
#define SSBO_H

#include <glad/glad.h>
#include <vector>
#include "Object.h"
#include "BVH.h"

class SSBO {

public:
	GLuint ID;

	SSBO();
	void buildTriangle(std::vector<Triangle> triangles);
	void buildNodes(std::vector<BVHNode> nodes);
	void buildTriIndex(std::vector<int> triIndex);

	void bind();
	void unBind();
	void deleteSSBO();

};


#endif