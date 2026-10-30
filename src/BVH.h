#ifndef BVH_H
#define BVH_H

#include <vector>

#include "Object.h"

#define BVHStackSize 32
#define SplitPanes 16

struct AABB {
	glm::vec3 min = glm::vec3(1e10f);
	glm::vec3 max = glm::vec3(-1e10f);
	int count = 0;

	void grow(const glm::vec3 pos) {
		count++;
		min = glm::min(min, pos);
		max = glm::max(max, pos);
	}

	float cost() const {
		const glm::vec3 e = max - min;
		return count * (e.x * e.y + e.y * e.z + e.z * e.x);
	}
};

class BVH {

public:
	std::vector<BVHNode> nodes;
	std::vector<glm::vec3> BVHTriangles;
	std::vector<int> BVHTriIndexes;

	BVH(const Model& model, const std::vector<Triangle>& triangles);

private:
	std::vector<Triangle> modelTriangles;

	void updateAABB(int index);
	void split(int index, int depth);
};


static void addBVH(Model& model, std::vector<BVHNode>& nodes, std::vector<int>& nodeTriIndex, const std::vector<Triangle>& triangles) {
	const int nodeStart = nodes.size();
	const int triStart = nodeTriIndex.size();

	BVH bvh(model, triangles);
	model.BVHIndex = nodeStart;

	for (BVHNode& node : bvh.nodes) {
		if (node.triCount > 0) node.leftIndex += triStart;
		else node.leftIndex += nodeStart;
		nodes.push_back(node);
	}
	for (int index: bvh.BVHTriIndexes) nodeTriIndex.push_back(index);
}


static void deleteBVH(const int nodeStart, const int nodeCount, const int triStart, const int triCount, std::vector<BVHNode>& nodes, std::vector<int>& nodeTriIndex) {
	nodeTriIndex.erase(nodeTriIndex.begin() + triStart, nodeTriIndex.begin() + triStart + triCount);
	nodes.erase(nodes.begin() + nodeStart, nodes.begin() + nodeStart + nodeCount);

	for (int i = nodeStart; i < nodes.size(); i++) {
		if (nodes[i].triCount > 0) nodes[i].leftIndex -= triCount;
		else nodes[i].leftIndex -= nodeCount;
	}
	for (int i = triStart; i < nodeTriIndex.size(); i++) nodeTriIndex[i] -= triCount;
}

#endif