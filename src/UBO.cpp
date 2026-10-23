#include "headers/UBO.h"

UBO::UBO() {
	glGenBuffers(1, &ID);
}

void UBO::build(const GLuint program, const UBO::Uniforms& uniforms, const std::vector<Sphere>& spheres, const std::vector<Material>& materials, const std::vector<Model>& models) const {
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

void UBO::bind() {
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
}

void UBO::unBind() {
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void UBO::deleteUBO() {
	glDeleteBuffers(1, &ID);
}

