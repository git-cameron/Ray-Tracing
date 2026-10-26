#include "BVH.h"

BVH::BVH(const Model& model, const std::vector<Triangle>& triangles) {
	this->modelTriangles = triangles;
	BVHNode root;
	root.leftIndex = 0;
	root.triIndex = 0;
	root.triCount = model.endIndex - model.startIndex + 1;
	nodes.push_back(root);
	
	// calculate triangle centres for subdivision
	// BVHTriangle.xyz = centre, w = index
	for (int i = model.startIndex; i <= model.endIndex; i++) {
		Triangle tri = triangles[i];
		BVHTriangles.emplace_back((tri.posA + tri.posB + tri.posC) / 3.0f, i);
	}

	updateAABB(0);
	split(0, 1);
}


void BVH::updateAABB(const int index) {
	nodes[index].min = glm::vec3(9999);
	nodes[index].max = glm::vec3(-9999);

	for (int i = 0; i < nodes[index].triCount; i++) {
		Triangle tri = modelTriangles[BVHTriangles[nodes[index].triIndex + i].w];

		// min bounds
		nodes[index].min = glm::min(nodes[index].min, tri.posA);
		nodes[index].min = glm::min(nodes[index].min, tri.posB);
		nodes[index].min = glm::min(nodes[index].min, tri.posC);

		// max bounds
		nodes[index].max = glm::max(nodes[index].max, tri.posA);
		nodes[index].max = glm::max(nodes[index].max, tri.posB);
		nodes[index].max = glm::max(nodes[index].max, tri.posC);
	}
}


void BVH::split(int index, int depth) {
	// base case
	if (depth >= BVHStackSize) return;

	// split on long axis
	glm::vec3 size = nodes[index].max - nodes[index].min;
	int axis = 0;
	if (size.y > size.x) axis = 1;
	if (size.z > size[axis]) axis = 2;
	float splitPos = nodes[index].min[axis] + size[axis] * 0.5f;

	// sort triangles into sides
	int i = nodes[index].triIndex;
	int j = nodes[index].triIndex + nodes[index].triCount - 1;
	while (i <= j) {
		if (BVHTriangles[i][axis] < splitPos) i++;
		else {
			// swap triangles
			glm::vec4 temp = BVHTriangles[i];
			BVHTriangles[i] = BVHTriangles[j];
			BVHTriangles[j] = temp;
			j--;
		}
	}

	BVHNode left;
	left.triCount = i - nodes[index].triIndex;

	// one sided
	if (left.triCount == 0 || left.triCount == nodes[index].triCount) return;
	left.triIndex = nodes[index].triIndex;
	nodes[index].leftIndex = nodes.size();
	nodes.push_back(left);

	BVHNode right;
	right.triIndex = i;
	right.triCount = nodes[index].triCount - left.triCount;
	nodes.push_back(right);
	nodes[index].triCount = 0;

	updateAABB(nodes[index].leftIndex);
	updateAABB(nodes[index].leftIndex + 1);
	split(nodes[index].leftIndex, depth + 1);
	split(nodes[index].leftIndex + 1, depth + 1);
}