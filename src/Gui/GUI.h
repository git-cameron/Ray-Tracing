#ifndef GUI_H
#define GUI_H

#include<stb/stb_image_write.h>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include<glad/glad.h>
#include<glm/glm.hpp>
#include<vector>
#include<string>

#include "imgui/imgui.h"
#include "FileHandler.h"
#include "BVH.h"
#include "Settings.h"
#include "Camera.h"
#include "Object.h"

using namespace ImGui;

class GUI {
public:
	GUI(GLFWwindow* window);
	void mainLoop(const GLuint& texture, float dt, State& state, RenderSettings& render, ToneSettings& tone, Scene& scene, Camera& cam);

	void render();
	void deleteGUI();

private:
	bool renderSettingsFlag = false;
	bool cameraFlag = false;
	bool editSpheresFlag = false;
	bool editModelsFlag = false;
	bool editMaterialsFlag = false;
	bool debugFlag = false;
	bool newMaterialFlag = false;
	bool newSphereFlag = false;
	bool newModelFlag = false;
	bool matchResolutionFlag = true;

	// render settings
	int renderFrames = 100;
	int renderRays = 5;
	int renderBounces = 100;
	int renderWidth = 1920;
	int renderHeight = 1080;
	glm::vec3 renderCamPos = glm::vec3(0);
	glm::vec3 renderCamRot = glm::vec3(0);
	float renderCamFov = 65;
	float renderCamFocusDistance = 1.0f;
	float renderCamAperture = 0.0f;
	bool renderCamFlag = false;

	// backup viewport settings
	int viewRays{};
	int viewBounces{};
	int viewWidth{};
	int viewHeight{};
	glm::vec3 viewCamPos{};
	glm::vec3 viewCamRot{};
	float viewCamFov{};
	float viewCamFocusDistance{};
	float viewCamAperture{};

	float scale = 1;
	Material tempMaterial{};
	Sphere tempSphere{};
	Model tempModel{};
	ModelExtra tempModelExtra{};
	std::string savePath;
	std::string modelPath = "Select Model";

	// Panels
	void renderWindow(const GLuint& texture, RenderSettings& render, State& state, Camera& cam, ImVec2& viewSize) const;
	void mainMenu(State& state, RenderSettings& render, Scene& scene, Camera& cam);
	void viewPort(const GLuint& texture, RenderSettings& render, ImVec2& viewSize, ImGuiWindowFlags windowFlags) const;
	void renderSettingsWindow(RenderSettings& render, ToneSettings& tone, State& state, Camera& cam, const ImVec2& viewSize, ImGuiWindowFlags windowFlags);
	void cameraWindow(Camera& cam, const RenderSettings& render, State& state, ImGuiWindowFlags windowFlags);
	void materialWindows(Scene& scene, State& state, ImGuiWindowFlags windowFlags);
	void sphereWindows(Scene& scene, State& state, ImGuiWindowFlags windowFlags);
	void modelWindows(Scene& scene, State& state, ImGuiWindowFlags windowFlags);
	void debugWindow(float dt, const State& state, const Scene& scene, ImGuiWindowFlags windowFlags) const;

	bool editMaterial(Material* mat);
	bool editSphere(Sphere* sphere, const std::vector<Material>& materials);
	bool editModel(Model* model, ModelExtra* extra, const std::vector<Material>& materials);

	void startRender(State& state, RenderSettings& render, Camera& cam);
};

static int GCD(int x, int y) {
	while (y != 0) {
		const int z = y;
		y = x % y;
		x = z;
	}
	return x;
}

#endif