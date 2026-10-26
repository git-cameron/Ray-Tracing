#include "Camera.h"

Camera::Camera(const int width, const float fov) {
	this->sensitivity = 5.0f;
	this->moveSpeed = 1.0f;
	this->dt = 0;
	this->fov = fov;
	this->focusDistance = 1.0f;
	this->aperture = 0.0f;

	focus = width / (2 * glm::tan(glm::radians(fov) / 2));
	pos = glm::vec3(0, 0, -1);
	rotation = glm::vec3(0);
	direction = originalDirection;
	up = originalUp;
	side = originalSide;
	model = glm::mat4(1);
}

void Camera::updateFocus(const int width) {
	focus = width / (2 * glm::tan(glm::radians(fov) / 2));
}


void Camera::updateModel() {
	const glm::mat3 rotateX = glm::rotate(glm::mat4(1.0), glm::radians(rotation.x), originalUp);
	const glm::mat3 rotateY = glm::rotate(glm::mat4(1.0), glm::radians(-rotation.y), originalSide);
	const glm::mat3 rotateZ = glm::rotate(glm::mat4(1.0), glm::radians(rotation.z), originalDirection);

	model = rotateX * rotateY * rotateZ;
	side = model[0];
	up = model[1];
	direction = model[2];
}

bool Camera::input(GLFWwindow* window) {
	float speed = moveSpeed * dt;
	bool moved = false;

	// sprint
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		speed *= 3;
	}

	// movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		pos += speed * direction;
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		pos -= speed * direction;
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		pos += speed * side;
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		pos -= speed * side;
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		pos += speed * up;
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		pos -= speed * up;
		moved = true;
	}

	// camera (low fps movement)
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		mouseInput(-1, 0, false);
		moved = true;
		}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		mouseInput(1, 0, false);
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		mouseInput(0, -1, false);
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		mouseInput(0, 1, false);
		moved = true;
	}

	return moved;
}

void Camera::mouseInput(const float dx, const float dy, const bool mouse) {
	if (mouse) {
		rotation.x += dx * dt * sensitivity;
		rotation.y -= dy * dt * sensitivity;
	}
	else {
		rotation.x += dx;
		rotation.y -= dy;
	}

	// clamp between -180, 180
	rotation = glm::mod(rotation + 180.0f, 360.0f) - 180.0f;

	updateModel();
}


void Camera::updateDT(const float dt) {
	this->dt = dt;
}