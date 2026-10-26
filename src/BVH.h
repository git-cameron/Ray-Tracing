#ifndef BVH_H
#define BVH_H

#include <vector>

#include "Object.h"

#define BVHStackSize 32

class BVH {

public:
	std::vector<BVHNode> nodes;
	std::vector<glm::vec4> BVHTriangles;

	BVH(const Model& model, const std::vector<Triangle>& triangles);

private:
	std::vector<Triangle> modelTriangles;

	void updateAABB(int index);
	void split(int index, int depth);

};

static void addBVH(Model& model, std::vector<BVHNode>& nodes, std::vector<int>& nodeTriIndex, const std::vector<Triangle>& triangles) {
	model.BVHIndex = nodes.size();
	const int triSize = nodeTriIndex.size();
	BVH bvh(model, triangles);

	for (BVHNode& node : bvh.nodes) {
		node.leftIndex += model.BVHIndex;
		node.triIndex += triSize;
		nodes.push_back(node);
	}
	for (const glm::vec4 & BVHTriangle : bvh.BVHTriangles) {
		nodeTriIndex.push_back(BVHTriangle.w);
	}
}

static void deleteBVH(const int index, const int nextIndex, std::vector<BVHNode>& nodes, std::vector<int>& nodeTriIndex) {
	// last model
	if (nextIndex == -1) {
		nodeTriIndex.erase(nodeTriIndex.begin() + nodes[index].triIndex, nodeTriIndex.end());
		nodes.erase(nodes.begin() + index, nodes.end());
	}
	else {
		const int indexDiff = nextIndex - index;
		const int triDiff = nodes[nextIndex].triIndex - nodes[index].triIndex;

		nodeTriIndex.erase(nodeTriIndex.begin() + nodes[index].triIndex, nodeTriIndex.begin() + nodes[nextIndex].triIndex);
		nodes.erase(nodes.begin() + index, nodes.begin() + nextIndex);

		for (int i = index; i < nodes.size(); i++) {
			nodes[i].leftIndex -= indexDiff;
			nodes[i].triIndex -= triDiff;
		}
	}
}

#endif