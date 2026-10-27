#ifndef OBJECT_H
#define OBJECT_H

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#define MAX_SPHERES 100
#define MAX_MATERIALS 100
#define MAX_MODELS 100

constexpr glm::vec3 originalSide = glm::vec3(1, 0, 0);
constexpr glm::vec3 originalUp = glm::vec3(0, 1, 0);
constexpr glm::vec3 originalDirection = glm::vec3(0, 0, 1);

struct Material {
	glm::vec3 color = glm::vec3(1); // 12 
	float lightStrength = 0.0f; //4
	glm::vec3 lightColor = glm::vec3(0); // 12
	float smoothness = 0.0f; // 4
	float gloss = 0.0f; // 4
	bool glass = false; // 4
	float refraction = 1.0f; // 4
	float pad = 0; // 4

	Material toLinear() const {
		Material mat = *this;
		mat.color = {srgbToLinear(color.r), srgbToLinear(color.g), srgbToLinear(color.b)};
		mat.lightColor = {srgbToLinear(lightColor.r), srgbToLinear(lightColor.g), srgbToLinear(lightColor.b)};
		return mat;
	}

private:
	static float srgbToLinear(const float colour) {
		return colour <= 0.04045f ? colour / 12.92f : std::pow((colour + 0.055f) / 1.055f, 2.4f);
	}
};


struct Sphere {
	glm::vec3 pos = glm::vec3(0); // 12
	float radius = 0.5f; // 4
	int matIndex = 0; // 4
	float pad1 = 0; // 4
	float pad2 = 0; // 4
	float pad3 = 0; // 4
};


struct Triangle {
	glm::vec3 posA; // 12
	float pad1; // 4
	glm::vec3 posB; // 12
	float pad2; // 4
	glm::vec3 posC; // 12
	float pad3; // 4
	glm::vec3 normA; // 12
	float pad4; // 4
	glm::vec3 normB; // 12
	float pad5; // 4
	glm::vec3 normC; // 12
	float pad6; // 4
};


// transform = translate * rotate (x * y * z) * scale

struct ModelExtra {
	glm::vec3 pos = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);
};

struct Model {
	glm::mat4 transform = glm::mat4(1); // 64
	glm::mat4 invTransform = glm::mat4(1); // 64
	int matIndex = 0; // 4
	int startIndex{}; // 4
	int endIndex{}; // 4
	int BVHIndex{}; // 4

	void setTransform(const ModelExtra& data) {
		const glm::mat3 rotateX = glm::rotate(glm::mat4(1.0), glm::radians(-data.rotation.x), originalUp);
		const glm::mat3 rotateY = glm::rotate(glm::mat4(1.0), glm::radians(-data.rotation.y), originalSide);
		const glm::mat3 rotateZ = glm::rotate(glm::mat4(1.0), glm::radians(data.rotation.z), originalDirection);

		const glm::mat4 translate = glm::translate(glm::mat4(1), data.pos);
		const glm::mat4 rotate = rotateX * rotateY * rotateZ;
		const glm::mat4 scale = glm::scale(glm::mat4(1), data.scale);
		transform = translate * rotate * scale;
		invTransform = glm::inverse(transform);
	}

	void setTransform(const ModelExtra* data) {
		const glm::mat3 rotateX = glm::rotate(glm::mat4(1.0), glm::radians(-data->rotation.x), originalUp);
		const glm::mat3 rotateY = glm::rotate(glm::mat4(1.0), glm::radians(-data->rotation.y), originalSide);
		const glm::mat3 rotateZ = glm::rotate(glm::mat4(1.0), glm::radians(data->rotation.z), originalDirection);

		const glm::mat4 translate = glm::translate(glm::mat4(1), data->pos);
		const glm::mat4 rotate = rotateX * rotateY * rotateZ;
		const glm::mat4 scale = glm::scale(glm::mat4(1), data->scale);
		transform = translate * rotate * scale;
		invTransform = glm::inverse(transform);
	}
};

struct BVHNode {
	glm::vec3 min; // 12
	int leftIndex = 0; // 4 (index of left child if triCount is 0, index of first triangle otherwise)
	glm::vec3 max; // 12
	int triCount; // 4
};

#endif