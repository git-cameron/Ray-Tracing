#ifndef UBO_H
#define UBO_H

#include <glad/glad.h>
#include <vector>
#include "Object.h"

class UBO {

public:
	GLuint ID{};

	struct Uniforms {
		glm::mat4 model; // 64
		glm::vec3 camPos; // 12
		float focus; // 4
		float focusDistance; // 4
		float aperture; // 4
		float width; // 4
		float height; // 4
		int maxBounces; // 4
		int rayCount; // 4
		int spheresLength; // 4
		int modelsLength; // 4
		int accumulationFrame; // 4
		int environmentLight; // 4
		float pad1 = 0; // 4
		float pad2 = 0; // 4
	};

	UBO() {
		glGenBuffers(1, &ID);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ID);
		glBufferData(GL_UNIFORM_BUFFER, TOTAL_SIZE, nullptr, GL_STATIC_DRAW);
	}

	void updateUniforms(const Uniforms& uniforms) {
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Uniforms), &uniforms);
	}

	void updateScene(const std::vector<Sphere>& spheres, const std::vector<Material>& materials, const std::vector<Model>& models) const {
		const int sphereCount = std::min(spheres.size(), static_cast<size_t>(MAX_SPHERES));
		const int materialCount = std::min(materials.size(), static_cast<size_t>(MAX_MATERIALS));
		const int modelCount = std::min(models.size(), static_cast<size_t>(MAX_MODELS));

		if (sphereCount > 0) glBufferSubData(GL_UNIFORM_BUFFER, SPHERE_OFFSET, sizeof(Sphere) * sphereCount, spheres.data());
		if (materialCount > 0) {
			// convert srgb colour to linear
			std::vector<Material> linearMaterials;
			for (Material mat : materials) linearMaterials.push_back(mat.toLinear());
			glBufferSubData(GL_UNIFORM_BUFFER, MATERIAL_OFFSET, sizeof(Material) * materialCount, linearMaterials.data());
		}
		if (modelCount > 0) glBufferSubData(GL_UNIFORM_BUFFER, MODEL_OFFSET, sizeof(Model) * modelCount, models.data());
	}

	void deleteUBO() const {
		glDeleteBuffers(1, &ID);
	}

private:
	static constexpr int SPHERE_OFFSET = sizeof(Uniforms);
	static constexpr int MATERIAL_OFFSET = SPHERE_OFFSET + sizeof(Sphere) * MAX_SPHERES;
	static constexpr int MODEL_OFFSET = MATERIAL_OFFSET + sizeof(Material) * MAX_MATERIALS;
	static constexpr int TOTAL_SIZE = MODEL_OFFSET + sizeof(Model) * MAX_MODELS;
};


#endif
