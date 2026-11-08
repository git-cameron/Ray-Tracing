#ifndef UBO_H
#define UBO_H

#include <glad/glad.h>
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
		glCreateBuffers(1, &ID);
		glNamedBufferData(ID, sizeof(Uniforms), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, ID);
	}

	void updateUniforms(const Uniforms& uniforms) const {
		glNamedBufferSubData(ID, 0, sizeof(Uniforms), &uniforms);
	}

	void deleteUBO() const {
		glDeleteBuffers(1, &ID);
	}
};


#endif
