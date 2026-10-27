#include "GUI.h"

GUI::GUI(GLFWwindow* window) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	CreateContext();
	GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
}


void GUI::mainLoop(const GLuint& texture, const float dt, State& state, RenderSettings& render, ToneSettings& tone, Scene& scene, Camera& cam) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();
	const ImGuiWindowFlags windowFlags = state.lockedMovement ? 0 : ImGuiWindowFlags_NoMove;
	ImVec2 viewSize;
	ImVec2 windowSize = GetIO().DisplaySize;
	windowSize = ImVec2(windowSize.x, windowSize.y - GetFrameHeight());

	if (state.renderFlag) {
		renderWindow(texture, render, state, cam, viewSize);
		return;
	}

	mainMenu(state, render, scene, cam);

	// exit early if render started from main menu, before overwritten by viewport
	if (state.renderFlag) return;

	// dock for nodes 
	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	SetNextWindowSize(windowSize);
	SetNextWindowPos(ImVec2(0, GetFrameHeight()));
	Begin("Dock", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	const ImGuiID dockID = GetID("Dock");
	DockSpace(dockID, ImVec2(0.0f, 0.0f));
	End();

	viewPort(texture, render, viewSize, windowFlags);
	renderSettingsWindow(render, tone, state, cam, viewSize, windowFlags);
	cameraWindow(cam, render, state, windowFlags);
	materialWindows(scene, state, windowFlags);
	sphereWindows(scene, state, windowFlags);
	modelWindows(scene, state, windowFlags);
	debugWindow(dt, state, scene, windowFlags);

	PopStyleVar();
}


void GUI::mainMenu(State& state, RenderSettings& render, Scene& scene, Camera& cam) {
	BeginMainMenuBar();
	if (BeginMenu("File")) {
		if (MenuItem("Save")) {
			std::string scenePath = getSavePath("scene.ray", "scene", "ray");
			if (!scenePath.empty()) saveScene(scenePath, scene.materials, scene.spheres);
		}
		if (MenuItem("Open")) {
			std::string scenePath = getLoadPath("scene", "ray");
			if (!scenePath.empty() && loadScene(scenePath, scene.materials, scene.spheres)) state.changed = true;
		}
		EndMenu();
	}

	if (BeginMenu("Render")) {
		if (MenuItem("Start##Render")) {
			startRender(state, render, cam);
			EndMenu();
			EndMainMenuBar();
			return;
		}
		if (MenuItem("Settings##Render")) renderSettingsFlag = !renderSettingsFlag;
		EndMenu();
	}

	if (MenuItem("Camera")) cameraFlag = !cameraFlag;

	if (BeginMenu("Objects")) {
		if (MenuItem("New Sphere##Objects")) newSphereFlag = !newSphereFlag;
		if (MenuItem("Edit Spheres##Objects")) editSpheresFlag = !editSpheresFlag;
		if (MenuItem("New Model##Objects")) newModelFlag = !newModelFlag;
		if (MenuItem("Edit Models##Objects")) editModelsFlag = !editModelsFlag;
		EndMenu();
	}

	if (BeginMenu("Materials")) {
		if (MenuItem("New Material##Materials")) newMaterialFlag = !newMaterialFlag;
		if (MenuItem("Edit Materials##Materials")) editMaterialsFlag = !editMaterialsFlag;
		EndMenu();
	}

	if (MenuItem("Debug")) debugFlag = !debugFlag;
	EndMainMenuBar();
}


void GUI::viewPort(const GLuint& texture, RenderSettings& render, ImVec2& viewSize, const ImGuiWindowFlags windowFlags) const {
	if (Begin("View", nullptr, windowFlags)) {
		viewSize = GetContentRegionAvail();
		if (matchResolutionFlag) {
			render.width = viewSize.x * scale;
			render.height = viewSize.y * scale;
		}
		Image(texture, ImVec2(viewSize[0], viewSize[1]), ImVec2(0, 1), ImVec2(1, 0));
	}
	End();
}


void GUI::cameraWindow(Camera& cam, const RenderSettings& render, State& state, const ImGuiWindowFlags windowFlags) {
	if (cameraFlag) {
		if (Begin("Camera", &cameraFlag, windowFlags)) {

			SeparatorText("Camera Position##Camera");
			if (DragFloat("Pos X##Camera", &cam.pos.x, 0.05)) state.changed = true;
			if (DragFloat("Pos Y##Camera", &cam.pos.y, 0.05)) state.changed = true;
			if (DragFloat("Pos Z##Camera", &cam.pos.z, 0.05)) state.changed = true;

			SeparatorText("Camera Direction##Camera");
			if (SliderFloat("Pitch##Camera", &cam.rotation.y, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
				cam.updateModel();
				state.changed = true;
			}
			if (SliderFloat("Yaw##Camera", &cam.rotation.x, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
				cam.updateModel();
				state.changed = true;
			}
			if (SliderFloat("Roll##Camera", &cam.rotation.z, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
				cam.updateModel();
				state.changed = true;
			}

			SeparatorText("Other##Camera");
			if (SliderFloat("Fov##Camera", &cam.fov, 1, 180, "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
				cam.updateFocus(render.width);
				state.changed = true;
			}
			if (DragFloat("Focus##Camera", &cam.focusDistance, 0.05, 0)) {
				if (cam.focusDistance < 0.1f) cam.focusDistance = 0.1f;
				state.changed = true;
			}
			if (SliderFloat("Aperture##Camera", &cam.aperture, 0, 2.0f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp)) {
				state.changed = true;
			}
			if (DragFloat("Speed##Camera", &cam.moveSpeed, 0.05, 0.1)) {
				if (cam.moveSpeed < 0.1f) cam.moveSpeed = 0.1f;
			}
			if (DragFloat("Sensitivity##Camera", &cam.sensitivity, 0.05, 0.1)) {
				if (cam.sensitivity < 0.1f) cam.sensitivity = 0.1f;
			}
		}
		End();
	}
}


void GUI::debugWindow(const float dt, const State& state, const Scene& scene, const ImGuiWindowFlags windowFlags) const {
	if (debugFlag) {
		SetNextWindowBgAlpha(0.35f);
		Begin("Debug", nullptr, windowFlags | ImGuiWindowFlags_NoTitleBar);
		Text(" %.0f fps", 1.0f / dt);
		Text(" %.2f ms", dt * 1000);
		Text(" %i frames", state.accumulationFrame);
		Text(" %i faces", static_cast<int>(scene.triangles.size()));
		Text(" %i nodes", static_cast<int>(scene.nodes.size()));
		End();
	}
}


void GUI::render() {
	Render();
	ImGui_ImplOpenGL3_RenderDrawData(GetDrawData());
}


void GUI::deleteGUI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	DestroyContext();
}

