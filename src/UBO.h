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
		bool environmentLight; // 4
		float pad1; // 4
		float pad2; // 4
	};

	UBO() {
		glGenBuffers(1, &ID);
	}

	void build(const GLuint program, const UBO::Uniforms& uniforms, const std::vector<Sphere>& spheres, const std::vector<Material>& materials, const std::vector<Model>& models) const {
		// reserve buffer size
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Uniforms) + sizeof(Sphere) * MAX_SPHERES + sizeof(Material) * MAX_MATERIALS + sizeof(Model) * MAX_MODELS, NULL, GL_STATIC_DRAW);

		// set data
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Uniforms), &uniforms);
		if (!spheres.empty()) glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Uniforms), sizeof(Sphere) * spheres.size(), &spheres[0]);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Uniforms) + sizeof(Sphere) * MAX_SPHERES, sizeof(Material) * materials.size(), &materials[0]);
		if (!models.empty()) glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Uniforms) + sizeof(Sphere) * MAX_SPHERES + sizeof(Material) * MAX_MATERIALS, sizeof(Model) * models.size(), &models[0]);

		// bind
		GLuint location = glGetUniformBlockIndex(program, "bindPoint");
		glUniformBlockBinding(program, location, 0);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ID);
	}

	void bind() {
		glBindBuffer(GL_UNIFORM_BUFFER, ID);
	}

	void unBind() {
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void deleteUBO() {
		glDeleteBuffers(1, &ID);
	}

};


#endif
