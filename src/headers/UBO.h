#ifndef UBO_H
#define UBO_H

#include <glad/glad.h>
#include <vector>
#include "Object.h"

class UBO {

public:
	GLuint ID;

	struct Uniforms {
		glm::mat4 model; // 64
		glm::vec3 camPos; // 12
		float focus; // 4
		float width; // 4
		float height; // 4
		int maxBounces; // 4
		int rayCount; // 4
		int spheresLength; // 4
		int modelsLength; // 4
		int accumulationFrame; // 4
		float environmentLight; // 4
	};

	UBO();
	void build(GLuint program, const UBO::Uniforms& uniforms, const std::vector<Sphere>& spheres, const std::vector<Material>& materials, const std::vector<Model>& models) const;
	void bind();
	void unBind();
	void deleteUBO();

};


#endif
