#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION

#include<iostream>

// created at build
#include "ShaderStrings.h"

// opengl headers
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include "Settings.h"
#include "Gui/GUI.h"
#include "Shader.h"
#include "VAO.h"
#include "VBO.h"
#include "Camera.h"
#include "UBO.h"
#include "FBO.h"
#include "SSBO.h"


// declare functions before use
void input(GLFWwindow* window, State& state, Camera& cam);

int main() {
	State state;
	RenderSettings render;
	ToneSettings tone;
	Scene scene;
	Camera cam(render.width, defaultFov);

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


	GLFWwindow* window = glfwCreateWindow(defaultWindowWidth, defaultWindowHeight, "Raytrace", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	//glfwSwapInterval(0); // disable Vsync

	// mouse mode
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, render.width, render.height);

	// ray shader
	Shader rayShader(Default_vert, Ray_frag);
	rayShader.use();
	glUniform1i(glGetUniformLocation(rayShader.program, "text"), 0);

	// tone shader
	Shader toneShader(Default_vert, Tone_frag);
	toneShader.use();
	glUniform1i(glGetUniformLocation(toneShader.program, "text"), 0);
	GLint exposureLoc = glGetUniformLocation(toneShader.program, "exposure");
	GLint tonemapLoc = glGetUniformLocation(toneShader.program, "tonemap");
	GLint grayscaleLoc = glGetUniformLocation(toneShader.program, "grayscale");

	VAO VAO;
	VBO VBO(vertices, sizeof(vertices));
	VAO.bind();
	VAO.linkVBO(VBO, 0);
	VAO.linkVBO(VBO, 1);

	FBO FBO(render.width, render.height);
	UBO UBO;
	SSBO triSSBO, nodeSSBO, triIndexSSBO;

	// initialize loop variables
	float currentTime, dt;
	float prevTime = glfwGetTime();
	int oldWidth = render.width;
	int oldHeight = render.height;
	int oldTriSize = 0;

	// imgui
    GUI gui(window);

	// main loop
	while (!glfwWindowShouldClose(window)) {
		// input first
		currentTime = glfwGetTime();
		dt = currentTime - prevTime;
		prevTime = currentTime;
		cam.updateDT(dt);
		input(window, state, cam);

		// check for resolution change
		if (render.width != oldWidth || render.height != oldHeight) {
			oldWidth = render.width;
			oldHeight = render.height;
			cam.updateFocus(render.width);

			// resize FBO and texture
			FBO.resize(render.width, render.height);

			// resize viewport
			glViewport(0, 0, render.width, render.height);
			state.changed = true;
		}

		if (state.changed) {
			state.changed = false;
			state.accumulationFrame = 1;
		} else state.accumulationFrame++;

		// scene data
		rayShader.use();
		UBO::Uniforms uni {
			.model = cam.model,
			.camPos = cam.pos,
			.focus = cam.focus,
			.focusDistance = cam.focusDistance,
			.aperture = cam.aperture,
			.width = static_cast<float>(render.width),
			.height = static_cast<float>(render.height),
			.maxBounces = render.maxBounces,
			.rayCount = render.rayCount,
			.spheresLength = static_cast<int>(scene.spheres.size()),
			.modelsLength = static_cast<int>(scene.models.size()),
			.accumulationFrame = state.accumulationFrame,
			.environmentLight = render.environmentLight
		};
		UBO.updateScene(uni, scene.spheres, scene.materials, scene.models);

		// SSBO data
		if (scene.triangles.size() != oldTriSize) {
			oldTriSize = scene.triangles.size();
			triSSBO.updateTriangle(scene.triangles);
			nodeSSBO.updateNodes(scene.nodes);
			triIndexSSBO.updateTriIndex(scene.nodeTriIndex);
		}

		// ray trace pass
		FBO.bindRay();
		FBO.bindRayTexture();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// tone map pass
		FBO.bindDisplay();
		toneShader.use();
		glUniform1f(exposureLoc, tone.exposure);
		glUniform1i(tonemapLoc, tone.tonemap);
		glUniform1i(grayscaleLoc, tone.grayScale);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// gui logic
		gui.mainLoop(FBO.displayTexture, dt, state, render, tone, scene, cam);

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
	toneShader.deleteShader();
	gui.deleteGUI();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


void input(GLFWwindow* window, State& state, Camera& cam) {
	// Mouse
	static float prevX = 0.0f;
	static float prevY = 0.0f;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	const float dx = x - prevX;
	const float dy = y - prevY;
	prevX = x;
	prevY = y;

	if (!state.lockedMovement && (dx != 0 || dy != 0)) {
		cam.mouseInput(dx, dy, true);
		state.changed = true;
	}


	// Keyboard
	static bool escapeHeld = false;
	static bool tabHeld = false;

	// closes program
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		if (!escapeHeld) {
			if (!state.renderFlag) glfwSetWindowShouldClose(window, true);
			escapeHeld = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) escapeHeld = false;

	// disables controls
	if (state.renderFlag) return;

	// enable or disable movement
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		if (!tabHeld) {
			state.lockedMovement = !state.lockedMovement;
			if (state.lockedMovement) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		tabHeld = true;
	} 
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) tabHeld = false;

	if (!state.lockedMovement && cam.input(window)) {
		state.changed = true;
	}
}
