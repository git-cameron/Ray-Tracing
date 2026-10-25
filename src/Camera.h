#ifndef CAMERA_H
#define CAMERA_H

#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

class Camera {
public:
	float fov;
	float focus;
	float focusDistance;
	float aperture;
	float sensitivity;
	float moveSpeed;
	glm::mat4 model{};
	glm::vec3 pos{};
	glm::vec3 rotation{};

	Camera(int width, int height, float fov);
	void updateRes(int width, int height);
	void updateFov();
	bool input(GLFWwindow* window);
	void mouseInput(float dx, float dy, bool mouse);
	void updateDT(float dt);
	void updateModel();

private:
	const glm::vec3 originalSide = glm::vec3(1, 0, 0);
	const glm::vec3 originalUp = glm::vec3(0, 1, 0);
	const glm::vec3 originalDirection = glm::vec3(0, 0, 1);

	float dt;
	int width;
	int height;
	glm::vec3 side{};
	glm::vec3 up{};
	glm::vec3 direction{};
};

#endif
