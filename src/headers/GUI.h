#ifndef GUI_H
#define GUI_H

#include<stb/stb_image_write.h>
#include<glad/glad.h>
#include<glm/glm.hpp>
#include<vector>
#include<string>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "FileHandler.h"
#include "headers/Camera.h"
#include "headers/Object.h"
#include "headers/BVH.h"

using namespace ImGui;

class GUI {
public:
	GUI(GLFWwindow* window);
	void mainLoop(GLuint texture, int& width, int& height, bool& lockedMovement, bool& renderFlag, bool& changed, Camera& cam, float dt, int accumulationFrame, 
		std::vector<Material>& materials, std::vector<Sphere>& spheres, std::vector<Model>& models, std::vector<ModelExtra>& modelExtras, std::vector<Triangle>& triangles, 
		std::vector<BVHNode>& nodes, std::vector<int>& nodeTriIndex, int& rayCount, int& maxBounces, float& environmentLight);

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
	bool renderCamFlag = false;

	// backup viewport settings
	int viewRays{};
	int viewBounces{};
	int viewWidth{};
	int viewHeight{};
	glm::vec3 viewCamPos{};
	glm::vec3 viewCamRot{};
	float viewCamFov{};

	float scale = 1;
	Material tempMaterial{};
	Sphere tempSphere{};
	Model tempModel{};
	ModelExtra tempModelExtra{};
	std::string savePath;
	std::string modelPath = "Select Model";

	bool editMaterial(Material* mat);
	bool editSphere(Sphere* sphere, const std::vector<Material>& materials);
	bool editModel(Model* model, ModelExtra* Extra, const std::vector<Material>& materials);

	void startRender(int& width, int& height, bool& lockedMovement, bool& renderFlag, bool& changed, Camera& cam, int& rayCount, int& maxBounces);
};

static int GCD(int x, int y) {
	int z;
	while (y != 0) {
		z = y;
		y = x % y;
		x = z;
	}
	return x;
}

#endif