#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include<iostream>

// created at build
#include "ShaderStrings.h"

// opengl headers
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include "headers/GUI.h"
#include "headers/Shader.h"
#include "headers/VAO.h"
#include "headers/VBO.h"
#include "headers/Camera.h"
#include "headers/Object.h"
#include "headers/UBO.h"
#include "headers/FBO.h"
#include "headers/SSBO.h"
#include "headers/BVH.h"


// globals
int windowWidth = 1408;
int windowHeight = 792;
int width = 1280;
int height = 720;
int oldWidth = width;
int oldHeight = height;
int rayCount = 3;
int maxBounces = 5;
int oldTriSize = 0;
float environmentLight = true;
float prevX = 0.0f;
float prevY = 0.0f;
bool renderFlag = false;
bool changed = false;
bool lockedMovement = false;
Camera cam(width, height, 65.0f);

// detects for keys just pressed
bool escapeHeld = false;
bool tabHeld = false;


// declare functions before use
void input(GLFWwindow* window, float dt);
void mouse_position_callback(GLFWwindow* window, double xPos, double yPos);
void framebuffer_size_callback(GLFWwindow* window, int x, int y);


int main() {
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLfloat vertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};


	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Raytrace", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	//glfwSwapInterval(0); // disable Vsync

	// call backs
	glfwSetCursorPosCallback(window, mouse_position_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// mouse mode
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, width, height);


	// create shaders
	Shader rayShader(Default_vert, Ray_frag);
	rayShader.use();
	glUniform1i(glGetUniformLocation(rayShader.program, "text"), 0);

	VAO VAO;
	VBO VBO(vertices, sizeof(vertices));

	VAO.bind();
	VAO.linkVBO(VBO, 0);
	VAO.linkVBO(VBO, 1);

	FBO FBO(width, height);
	FBO.unBind();

	UBO UBO;
	SSBO triSSBO;
	triSSBO.unBind();
	SSBO nodeSSBO;
	nodeSSBO.unBind();
	SSBO triIndexSSBO;
	triIndexSSBO.unBind();


	VAO.unBind();
	VBO.unBind();
	UBO.unBind();


	// contains scene data
	std::vector<Material> materials;
	materials.push_back(Material{});
	std::vector<Sphere> spheres;
	std::vector<Model> models;
	std::vector<ModelExtra> modelExtras;
	std::vector<Triangle> triangles;
	std::vector<BVHNode> nodes;
	std::vector<int> nodeTriIndex;

	// initialize loop variables
	float currentTime, dt;
	float prevTime = glfwGetTime();
	int accumulationFrame = 0;

	FBO.bindTexture();

	// imgui
    GUI gui(window);

	// main loop
	while (!glfwWindowShouldClose(window)) {
		// input first
		currentTime = glfwGetTime();
		dt = currentTime - prevTime;
		prevTime = currentTime;
		cam.updateDT(dt);
		input(window, dt);

		// check for resolution change
		if (width != oldWidth || height != oldHeight) {
			oldWidth = width;
			oldHeight = height;
			cam.updateRes(width, height);

			// resize FBO and texture
			FBO.bind();
			FBO.resize(width, height);
			FBO.unBind();
			FBO.bindTexture();

			// resize viewport
			glViewport(0, 0, width, height);

			changed = true;
		}

		if (changed) {
			changed = false;
			accumulationFrame = 1;
		}
		else {
			accumulationFrame++;
		}

		// render scene to FBO
		FBO.bind();
		rayShader.use();

		// GPU data
		UBO::Uniforms uni {cam.model, cam.pos, cam.focus, width, height, maxBounces, rayCount, spheres.size(), models.size(), accumulationFrame, environmentLight};
		UBO.bind();
		UBO.build(rayShader.program, uni, spheres, materials, models);

		// SSBO data
		if (triangles.size() != oldTriSize) {
			oldTriSize = triangles.size();
			triSSBO.bind();
			triSSBO.buildTriangle(triangles);
			nodeSSBO.bind();
			nodeSSBO.buildNodes(nodes);
			triIndexSSBO.bind();
			triIndexSSBO.buildTriIndex(nodeTriIndex);
			triIndexSSBO.unBind();
		}
		


		VAO.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// gui logic
		gui.mainLoop(FBO.texture, width, height, lockedMovement, renderFlag, changed, cam, dt, accumulationFrame, materials, spheres, models, modelExtras, 
			triangles, nodes, nodeTriIndex, rayCount, maxBounces, environmentLight);

		FBO.unBind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		gui.render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	VAO.deleteVAO();
	VBO.deleteVBO();
	FBO.deleteFBO();
	UBO.deleteUBO();
	triSSBO.deleteSSBO();
	nodeSSBO.deleteSSBO();
	triIndexSSBO.deleteSSBO();
	rayShader.deleteShader();
	gui.deleteGUI();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


// resize window
void framebuffer_size_callback(GLFWwindow* window, int x, int y) {
	windowWidth = x;
	windowHeight = y;
}

// mouse movement
void mouse_position_callback(GLFWwindow* window, double xPos, double yPos) {
	float x = static_cast<float>(xPos);
	float y = static_cast<float>(yPos);

	float dx = x - prevX;
	float dy = y - prevY; 
	prevX = x;
	prevY = y;

	if (!lockedMovement) {
		cam.mouseInput(dx, dy, true);
		changed = true;
	}
}

// keyboard
void input(GLFWwindow* window, float dt) {

	// closes program
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		if (!escapeHeld) {
			if (!renderFlag) glfwSetWindowShouldClose(window, true);
			escapeHeld = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) escapeHeld = false;

	// disables controls
	if (renderFlag) return;

	// enable or disable movement
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		if (!tabHeld) {
			lockedMovement = !lockedMovement;
			if (lockedMovement) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		tabHeld = true;
	} 
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) tabHeld = false;

	if (!lockedMovement && cam.input(window)) {
		changed = true;
	}
}
