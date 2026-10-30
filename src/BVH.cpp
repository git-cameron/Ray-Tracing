#include "BVH.h"

BVH::BVH(const Model& model, const std::vector<Triangle>& triangles) {
	this->modelTriangles = triangles;
	BVHNode root;
	root.leftIndex = 0;
	root.triCount = model.endIndex - model.startIndex + 1;
	nodes.push_back(root);
	
	// calculate triangle centres for subdivision
	// BVHTriangle.xyz = centre, w = index
	for (int i = model.startIndex; i <= model.endIndex; i++) {
		Triangle tri = triangles[i];
		BVHTriangles.emplace_back((tri.posA + tri.posB + tri.posC) / 3.0f);
		BVHTriIndexes.push_back(i);
	}

	updateAABB(0);
	split(0, 1);
}


void BVH::updateAABB(const int index) {
	nodes[index].min = glm::vec3(1e10);
	nodes[index].max = glm::vec3(-1e10);

	const int firstIndex = nodes[index].leftIndex;

	for (int i = 0; i < nodes[index].triCount; i++) {
		const Triangle& tri = modelTriangles[BVHTriIndexes[firstIndex + i]];

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

	// save before leftIndex overwritten by child index
	const int firstIndex = nodes[index].leftIndex;
	const int triCount = nodes[index].triCount;
	AABB parent;
	for (int i = firstIndex; i < firstIndex + triCount; i++) parent.grow(glm::vec3(BVHTriangles[i]));

	int bestAxis = 0;
	float bestPos = 0;
	float bestCost = 1e10;
	for (int axis = 0; axis < 3; axis++) {
		float boundsMin = parent.min[axis];
		float boundsMax = parent.max[axis];
		if (boundsMin == boundsMax) continue;
		float scale = (boundsMax - boundsMin) / SplitPanes;

		for (int i = 0; i < SplitPanes; i++) {
			float candidate = boundsMin + i * scale;

			// Surface Area Heuristic
			AABB left, right;
			for (int tri = firstIndex; tri < firstIndex + triCount; tri++) {
				glm::vec3 centre = BVHTriangles[tri];
				if (centre[axis] < candidate) left.grow(centre);
				else right.grow(centre);
			}
			float cost = left.cost() + right.cost();
			if (cost < bestCost) {
				bestCost = cost;
				bestAxis = axis;
				bestPos = candidate;
			}
		}
	}

	// Not worth splitting
	if (parent.cost() <= bestCost) return;

	// sort triangles into sides
	int i = firstIndex;
	int j = firstIndex + triCount - 1;
	while (i <= j) {
		if (BVHTriangles[i][bestAxis] < bestPos) i++;
		else {
			// swap triangles
			glm::vec3 tempTri = BVHTriangles[i];
			int tempIndex = BVHTriIndexes[i];
			BVHTriangles[i] = BVHTriangles[j];
			BVHTriIndexes[i] = BVHTriIndexes[j];
			BVHTriangles[j] = tempTri;
			BVHTriIndexes[j] = tempIndex;
			j--;
		}
	}

	int leftCount = i - firstIndex;

	// one sided
	if (leftCount == 0 || leftCount == triCount) return;

	BVHNode left, right;
	left.leftIndex = firstIndex;
	left.triCount = leftCount;
	right.leftIndex = i;
	right.triCount = triCount - leftCount;

	int childIndex = nodes.size();
	nodes.push_back(left);
	nodes.push_back(right);

	nodes[index].leftIndex = childIndex;
	nodes[index].triCount = 0;

	updateAABB(childIndex);
	updateAABB(childIndex + 1);
	split(childIndex, depth + 1);
	split(childIndex + 1, depth + 1);
}