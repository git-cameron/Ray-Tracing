#include "GUI.h"

GUI::GUI(GLFWwindow* window) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	CreateContext();
	ImGuiIO& io = GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
}


void GUI::mainLoop(GLuint texture, int& width, int& height, bool& lockedMovement, bool& renderFlag, bool& changed, Camera& cam, float dt, int accumulationFrame, 
	std::vector<Material>& materials, std::vector<Sphere>& spheres, std::vector<Model>& models, std::vector<ModelExtra>& modelExtras, std::vector<Triangle>& triangles,
	std::vector<BVHNode>& nodes, std::vector<int>& nodeTriIndex, int& rayCount, int& maxBounces, bool& environmentLight) {

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();
	ImGuiIO& io = GetIO();
	//ShowDemoWindow();


	ImGuiWindowFlags windowFlags = lockedMovement ? 0 : ImGuiWindowFlags_NoMove;
	ImVec2 viewSize;


	// rendering
	if (renderFlag) {
		// render finished
		if (accumulationFrame >= renderFrames || IsKeyPressed(ImGuiKey_Escape)) {
			saveImage(savePath, width, height);
			renderFlag = false;

			// restore viewport settings
			width = viewWidth;
			height = viewHeight;
			rayCount = viewRays;
			maxBounces = viewBounces;

			if (renderCamFlag) {
				cam.pos = viewCamPos;
				cam.rotation = viewCamRot;
				cam.fov = viewCamFov;
				cam.focusDistance = viewCamFocusDistance;
				cam.aperture = viewCamAperture;
				cam.updateModel();
				cam.updateFov();
			}

			return;
		}

		PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		SetNextWindowSize(io.DisplaySize);
		SetNextWindowPos(ImVec2(0, 0));
		if (Begin("Render", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
			viewSize = GetContentRegionAvail();
			Image(texture, ImVec2(viewSize[0], viewSize[1] - GetFrameHeightWithSpacing()), ImVec2(0, 1), ImVec2(1, 0));
			ProgressBar((float) accumulationFrame / renderFrames, ImVec2(-1.0f, 0.0f), (std::to_string(accumulationFrame) + "/" + std::to_string(renderFrames)).c_str());
		}
		End();

		PopStyleVar();
		return;
	}


	// main menu
	BeginMainMenuBar();
	if (BeginMenu("File")) {
		if (MenuItem("Save")) {
			std::string scenePath = getSavePath("scene.ray", "scene", "ray");
			if (!scenePath.empty()) saveScene(scenePath, materials, spheres);
		}
		if (MenuItem("Open")) {
			std::string scenePath = getLoadPath("scene", "ray");
			if (!scenePath.empty()) {
				if (loadScene(scenePath, materials, spheres)) {
					changed = true;
				}
			}
		}
		ImGui::EndMenu();
	}


	if (BeginMenu("Render")) {
		if (MenuItem("Start##Render")) {
			startRender(width, height, lockedMovement, renderFlag, changed, cam, rayCount, maxBounces);
			ImGui::EndMenu();
			EndMainMenuBar();
			return;
		}
		if (MenuItem("Settings##Render")) renderSettingsFlag = !renderSettingsFlag;
		ImGui::EndMenu();
	}

	if (MenuItem("Camera")) cameraFlag = !cameraFlag;

	if (BeginMenu("Objects")) {
		if (MenuItem("New Sphere##Objects")) newSphereFlag = !newSphereFlag;
		if (MenuItem("Edit Spheres##Objects")) editSpheresFlag = !editSpheresFlag;
		if (MenuItem("New Model##Objects")) newModelFlag = !newModelFlag;
		if (MenuItem("Edit Models##Objects")) editModelsFlag = !editModelsFlag;
		ImGui::EndMenu();
	}

	if (BeginMenu("Materials")) {
		if (MenuItem("New Material##Materials")) newMaterialFlag = !newMaterialFlag;
		if (MenuItem("Edit Materials##Materials")) editMaterialsFlag = !editMaterialsFlag;
		ImGui::EndMenu();
	}	

	if (MenuItem("Debug")) debugFlag = !debugFlag;
	EndMainMenuBar();

	ImVec2 windowSize = io.DisplaySize;
	windowSize = ImVec2(windowSize.x, windowSize.y - GetFrameHeight());

	// dock for nodes 
	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	SetNextWindowSize(windowSize);
	SetNextWindowPos(ImVec2(0, GetFrameHeight()));
	Begin("Dock", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGuiID dockID = GetID("Dock");
	DockSpace(dockID, ImVec2(0.0f, 0.0f));

	End();

	// viewport
	if (Begin("View", nullptr, windowFlags)) {
		viewSize = GetContentRegionAvail();
		if (matchResolutionFlag) {
			width = viewSize.x * scale;
			height = viewSize.y * scale;
		}
		Image(texture, ImVec2(viewSize[0], viewSize[1]), ImVec2(0, 1), ImVec2(1, 0));


		// mouse position in viewport
		/*
		glm::vec2 viewportMouse = glm::vec2(-1);
		if (io.MouseClicked[0]) {
			viewportMouse = glm::vec2(io.MousePos.x - GetCursorScreenPos().x, -io.MousePos.y + GetCursorScreenPos().y);
			viewportMouse.x = viewportMouse.x <= viewSize.x ? viewportMouse.x : -1;
			viewportMouse.y = viewportMouse.y <= viewSize.y ? viewportMouse.y : -1;
			if (viewportMouse.x >= 0 && viewportMouse.y >= 0) puts("in");
			else puts("out");
		}
		*/

	}
	End();

	// render settings
	if (renderSettingsFlag) {
		if (Begin("Render Settings", &renderSettingsFlag, windowFlags)) {
			SeparatorText("Window");

			// window resolution
			Text("Resolution %.0fx%.0f", viewSize.x, viewSize.y);
			int gcd = GCD(viewSize.x, viewSize.y);
			Text("Aspect ratio %.0f:%.0f", viewSize.x / gcd, viewSize.y / gcd);
			

			if (CollapsingHeader("Viewport")) {
				// preview resolution
				Checkbox("Match Resolution", &matchResolutionFlag);
				if (matchResolutionFlag) {
					float slideScale = scale * scale * 100;
					if (SliderFloat("Resolution", &slideScale, 1, 400, "%.0f%%", ImGuiSliderFlags_AlwaysClamp)) {
						scale = slideScale / 100;
						scale = glm::sqrt(scale);
					}
					Text("Resolution %ix%i", width, height);
				}
				else {
					if (InputInt("Width##Viewport", &width, 0)) {
						if (width < 1) width = viewSize.x;
					}
					if (InputInt("Height##Viewport", &height, 0)) {
						if (height < 1) height = viewSize.y;
					}
					gcd = GCD(width, height);
					Text("Aspect ratio %i:%i", width / gcd, height / gcd);
				}
				Separator();

				// parameters
				if (InputInt("Ray Count##Viewport", &rayCount, 0)) {
					if (rayCount < 1) rayCount = 1;
					changed = true;
				}
				if (InputInt("Max Bounces##Viewport", &maxBounces, 0)) {
					if (maxBounces < 0) maxBounces = 0;
					changed = true;
				}
				Separator();
			}


			if (CollapsingHeader("Render")) {

				// render resolution
				if (InputInt("Width##Render", &renderWidth, 0)) {
					if (renderWidth < 1) renderWidth = viewSize.x;
				}
				if (InputInt("Height##Render", &renderHeight, 0)) {
					if (renderHeight < 1) renderHeight = viewSize.y;
				}
				gcd = GCD(renderWidth, renderHeight);
				Text("Aspect ratio %i:%i", renderWidth / gcd, renderHeight / gcd);
				Separator();

				// parameters
				if (InputInt("Frames##Render", &renderFrames, 0)) {
					if (renderFrames < 1) renderFrames = 1;
				}
				if (InputInt("Ray Count##Render", &renderRays, 0)) {
					if (renderRays < 1) renderRays = 1;
				}
				if (InputInt("Max Bounces##Render", &renderBounces, 0)) {
					if (renderBounces < 0) renderBounces = 0;
				}
				Separator();
			}


			// camera settings
			if (CollapsingHeader("Render Camera")) {
				SeparatorText("Camera Position");
				DragFloat("Pos X##Render", &renderCamPos.x, 0.05);
				DragFloat("Pos Y##Render", &renderCamPos.y, 0.05);
				DragFloat("Pos Z##Render", &renderCamPos.z, 0.05);

				SeparatorText("Camera Direction");
				SliderFloat("Pitch##Render", &renderCamRot.y, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp);
				SliderFloat("Yaw##Render", &renderCamRot.x, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp);
				SliderFloat("Roll##Render", &renderCamRot.z, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp);

				SeparatorText("Camera Other");
				SliderFloat("Fov##Render", &renderCamFov, 1, 180, "%.0f", ImGuiSliderFlags_AlwaysClamp);
				if (DragFloat("Focus##Render", &renderCamFocusDistance, 0.05, 0)) {
					if (renderCamFocusDistance < 0.1f) renderCamFocusDistance = 0.1f;
				}
				SliderFloat("Aperture##Render", &renderCamAperture, 0, 2.0f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
			}


			SeparatorText("Other");

			if (Checkbox("Environment Light", &environmentLight)) changed = true;

			Checkbox("Render Camera##Checkbox", &renderCamFlag);

			if (Button("Set Camera")) {
				renderCamPos = cam.pos;
				renderCamRot = cam.rotation;
				renderCamFov = cam.fov;
			}

			if (Button("Start Render")) {
				startRender(width, height, lockedMovement, renderFlag, changed, cam, rayCount, maxBounces);
			}
		}
		End();
	}

	// camera controls
	if (cameraFlag) {
		if (Begin("Camera", &cameraFlag, windowFlags)) {

			SeparatorText("Camera Position##Camera");
			if (DragFloat("Pos X##Camera", &cam.pos.x, 0.05)) {
				changed = true;
			}
			if (DragFloat("Pos Y##Camera", &cam.pos.y, 0.05)) {
				changed = true;
			}
			if (DragFloat("Pos Z##Camera", &cam.pos.z, 0.05)) {
				changed = true;
			}

			SeparatorText("Camera Direction##Camera");
			if (SliderFloat("Pitch##Camera", &cam.rotation.y, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
				cam.updateModel();
				changed = true;
			}
			if (SliderFloat("Yaw##Camera", &cam.rotation.x, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
				cam.updateModel();
				changed = true;
			}
			if (SliderFloat("Roll##Camera", &cam.rotation.z, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
				cam.updateModel();
				changed = true;
			}

			SeparatorText("Other##Camera");
			if (SliderFloat("Fov##Camera", &cam.fov, 1, 180, "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
				cam.updateFov();
				changed = true;
			}
			if (DragFloat("Focus##Camera", &cam.focusDistance, 0.05, 0)) {
				if (cam.focusDistance < 0.1f) cam.focusDistance = 0.1f;
				changed = true;
			}
			if (SliderFloat("Aperture##Camera", &cam.aperture, 0, 2.0f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp)) {
				changed = true;
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

	// material customisation
	if (editMaterialsFlag) {
		if (Begin("Materials", &editMaterialsFlag, windowFlags)) {

			// tree buttons
			bool openAllMat = false, closeAllMat = false;
			if (Button("Expand All")) {
				openAllMat = true;
			}
			SameLine();
			if (Button("Close All")) {
				closeAllMat = true;
			}

			if (!newMaterialFlag) {
				if (Button("New Material")) {
					tempMaterial = Material{};
					newMaterialFlag = true;
				}
			}

			// list all materials
			for (int i = 0; i < materials.size(); i++) {
				if (openAllMat) SetNextItemOpen(true);
				if (closeAllMat) SetNextItemOpen(false);

				if (TreeNodeEx(("Material " + std::to_string(i)).c_str())) {
					Material* mat = &materials[i];
					changed = editMaterial(mat) || changed;
					TreePop();
				}
			}
		}
		End();
	}

	// add new material
	if (newMaterialFlag) {
		if (Begin("New Material", &newMaterialFlag, windowFlags | ImGuiWindowFlags_NoDocking)) {
			editMaterial(&tempMaterial);
			Separator();
			if (Button("Add Material")) {
				materials.push_back(tempMaterial);
			}
		}
		End();
	}

	// sphere customisation
	if (editSpheresFlag) {
		if (Begin("Spheres", &editSpheresFlag, windowFlags)) {

			// tree buttons
			bool openAllSphere = false, closeAllSphere = false;
			if (Button("Expand All##1")) {
				openAllSphere = true;
			}
			SameLine();
			if (Button("Close All##1")) {
				closeAllSphere = true;
			}

			if (!newSphereFlag) {
				if (Button("New Sphere")) {
					tempSphere = Sphere{};
					newSphereFlag = true;
				}
				SameLine();
			}

			// delete all spheres
			if (!spheres.empty()) {
				if (Button("Delete All")) {
					OpenPopup("Delete?");
				}

				// pop up
				SetNextWindowPos(GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				if (BeginPopupModal("Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					Text("Delete All Spheres?");
					Separator();
					if (Button("Confirm")) {
						spheres.clear();
						changed = true;
						CloseCurrentPopup();
					}
					SameLine();
					if (Button("Cancel")) {
						CloseCurrentPopup();
					}
					EndPopup();
				}
			}


			// list all spheres
			for (int i = 0; i < spheres.size(); i++) {
				if (openAllSphere) SetNextItemOpen(true);
				if (closeAllSphere) SetNextItemOpen(false);

				if (TreeNodeEx(("Sphere " + std::to_string(i)).c_str())) {
					Sphere* sphere = &spheres[i];
					changed = editSphere(sphere, materials) || changed;

					// delete spheres
					SeparatorText("Delete Sphere");
					if (Button(("Delete##1" + std::to_string(i)).c_str())) {
						spheres.erase(spheres.begin() + i);
						changed = true;
					}
					TreePop();
				}
			}
		}
		End();
	}

	// add new sphere
	if (newSphereFlag) {
		if (Begin("New Sphere", &newSphereFlag, windowFlags | ImGuiWindowFlags_NoDocking)) {
			editSphere(&tempSphere, materials);
			Separator();
			if (Button("Add Sphere")) {
				spheres.push_back(tempSphere);
				changed = true;
			}
		}
		End();
	}

	// model customisation
	if (editModelsFlag) {
		if (Begin("Models", &editModelsFlag, windowFlags)) {
			// tree buttons
			bool openAllModels = false, closeAllModels = false;
			if (Button("Expand All##2")) {
				openAllModels = true;
			}
			SameLine();
			if (Button("Close All##2")) {
				closeAllModels = true;
			}

			if (!newModelFlag) {
				if (Button("New Model")) {
					modelPath = "Select Model";
					tempModel = Model{};
					tempModelExtra = ModelExtra{};
					newModelFlag = true;
				}
				SameLine();
			}

			// delete all models
			if (!models.empty()) {
				if (Button("Delete All##1")) {
					OpenPopup("Delete?");
				}

				// pop up
				SetNextWindowPos(GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				if (BeginPopupModal("Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					Text("Delete All Models?");
					Separator();
					if (Button("Confirm")) {
						models.clear();
						modelExtras.clear();
						triangles.clear();
						nodes.clear();
						nodeTriIndex.clear();
						changed = true;
						CloseCurrentPopup();
					}
					SameLine();
					if (Button("Cancel")) {
						CloseCurrentPopup();
					}
					EndPopup();
				}
			}


			// list all models
			for (int i = 0; i < models.size(); i++) {
				if (openAllModels) SetNextItemOpen(true);
				if (closeAllModels) SetNextItemOpen(false);

				if (TreeNodeEx(("Model " + std::to_string(i)).c_str())) {
					// edit model
					Model* model = &models[i];
					ModelExtra* extra = &modelExtras[i];
					changed = editModel(model, extra, materials) || changed;

					// delete models
					SeparatorText("Delete Model");
					if (Button(("Delete##2" + std::to_string(i)).c_str())) {
						int bvhDiff = i == models.size() - 1 ? 0 : models[i + 1].BVHIndex - models[i].BVHIndex;
						deleteBVH(models[i].BVHIndex, bvhDiff == 0 ? -1 : models[i + 1].BVHIndex, nodes, nodeTriIndex);

						int modelSize = models[i].endIndex - models[i].startIndex + 1;
						triangles.erase(triangles.begin() + models[i].startIndex, triangles.begin() + models[i].endIndex + 1);
						models.erase(models.begin() + i);
						modelExtras.erase(modelExtras.begin() + i);
						for (int j = i; j < models.size(); j++) {
							models[j].startIndex -= modelSize;
							models[j].endIndex -= modelSize;
							models[j].BVHIndex -= bvhDiff;
						}
						changed = true;
					}
					TreePop();
				}
			}

		}
		End();
	}

	// add new model
	if (newModelFlag) {
		if (Begin("New Model", &newModelFlag, windowFlags | ImGuiWindowFlags_NoDocking)) {
			editModel(&tempModel, &tempModelExtra, materials);
			Separator();

			// gets model path
			if (Button((modelPath + "##NewModel").c_str())) {
				modelPath = getLoadPath("model", "obj");
				if (modelPath.empty()) modelPath = "Select Model";
			}
			SetItemTooltip("Model Path (.obj)");

			if (Button("Add Model")) {
				if (loadModel(modelPath, models, modelExtras, triangles)) {
					addBVH(models[models.size() - 1], nodes, nodeTriIndex, triangles);
					modelExtras[modelExtras.size() - 1] = tempModelExtra;
					models[models.size() - 1].transform = tempModel.transform;
					models[models.size() - 1].invTransform = tempModel.invTransform;
					models[models.size() - 1].matIndex = tempModel.matIndex;
					changed = true;
				}
			}
		}
		End();
	}

	// debug display
	if (debugFlag) {
		SetNextWindowBgAlpha(0.35f);
		Begin("Debug", nullptr, windowFlags | ImGuiWindowFlags_NoTitleBar);
		Text(" %.0f fps", 1.0f / dt);
		Text(" %.2f ms", dt * 1000);
		Text(" %i frames", accumulationFrame);
		Text(" %i faces", triangles.size());
		Text(" %i nodes", nodes.size());
		End();
	}

	PopStyleVar();
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


bool GUI::editMaterial(Material* mat) {
	bool changed = false;

	// object color
	float color[]{ mat->color.x, mat->color.y, mat->color.z };
	if (ColorEdit3("Colour", color)) {
		mat->color = glm::vec3(color[0], color[1], color[2]);
		changed = true;
	}

	// light
	float lightColor[]{ mat->lightColor.x, mat->lightColor.y, mat->lightColor.z };
	if (ColorEdit3("Light Colour", lightColor)) {
		mat->lightColor = glm::vec3(lightColor[0], lightColor[1], lightColor[2]);
		changed = true;
	}

	if (DragFloat("Light Strength", &mat->lightStrength, 0.05, 0)) {
		if (mat->lightStrength < 0) mat->lightStrength = 0;
		changed = true;
	}

	// glass
	bool glass = mat->glass;
	if (Checkbox("Glass", &glass)) {
		mat->glass = glass;
		changed = true;
	}
	if (glass) {
		if (DragFloat("Refraction", &mat->refraction, 0.001, 0)) {
			if (mat->refraction < 0) mat->refraction = 0;
			changed = true;
		}
	}

	// other properties
	else {
		if (DragFloat("Gloss", &mat->gloss, 0.001, 0, 1, "%f", ImGuiSliderFlags_AlwaysClamp)) {
			if (mat->gloss < 0) mat->gloss = 0;
			changed = true;
		}
	}

	if (DragFloat("Smoothness", &mat->smoothness, 0.001, 0, 1, "%f", ImGuiSliderFlags_AlwaysClamp)) {
		if (mat->smoothness < 0) mat->smoothness = 0;
		changed = true;
	}

	return changed;
}


bool GUI::editSphere(Sphere* sphere, const std::vector<Material>& materials) {
	bool changed = false;

	SeparatorText("Sphere Position");
	if (DragFloat("Pos X", &sphere->pos.x, 0.05)) {
		changed = true;
	}
	if (DragFloat("Pos Y", &sphere->pos.y, 0.05)) {
		changed = true;
	}
	if (DragFloat("Pos Z", &sphere->pos.z, 0.05)) {
		changed = true;
	}

	SeparatorText("Other");
	if (DragFloat("Radius", &sphere->radius, 0.01, 0.01)) {
		if (sphere->radius < 0.01) sphere->radius = 0.01;
		changed = true;
	}

	if (SliderInt("Material", &sphere->matIndex, 0, materials.size() - 1, "%i", ImGuiSliderFlags_AlwaysClamp)) {
		changed = true;
	}

	return changed;
}


bool GUI::editModel(Model* model, ModelExtra* extra, const std::vector<Material>& materials) {
	bool changed = false;

	SeparatorText("Model Position##EditModel");
	if (DragFloat("Pos X##EditModel", &extra->pos.x, 0.05)) {
		model->setTransform(extra);
		changed = true;
	}
	if (DragFloat("Pos Y##EditModel", &extra->pos.y, 0.05)) {
		model->setTransform(extra);
		changed = true;
	}
	if (DragFloat("Pos Z##EditModel", &extra->pos.z, 0.05)) {
		model->setTransform(extra);
		changed = true;
	}

	SeparatorText("Model Rotation##EditModel");
	if (SliderFloat("Rotation X##EditModel", &extra->rotation.x, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
		model->setTransform(extra);
		changed = true;
	}
	if (SliderFloat("Rotation Y##EditModel", &extra->rotation.y, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
		model->setTransform(extra);
		changed = true;
	}
	if (SliderFloat("Rotation Z##EditModel", &extra->rotation.z, -180, 180, "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
		model->setTransform(extra);
		changed = true;
	}

	SeparatorText("Other##EditModel");
	float modelScale = extra->scale.x;

	if (DragFloat("Scale##EditModel", &modelScale, 0.05, 0.1)) {
		if (modelScale < 0) modelScale = 0;
		extra->scale = glm::vec3(modelScale);
		model->setTransform(extra);
		changed = true;
	}

	if (SliderInt("Material##EditModel", &model->matIndex, 0, materials.size() - 1, "%i", ImGuiSliderFlags_AlwaysClamp)) {
		changed = true;
	}

	return changed;
}


void GUI::startRender(int& width, int& height, bool& lockedMovement, bool& renderFlag, bool& changed, Camera& cam, int& rayCount, int& maxBounces) {
	savePath = getSavePath("render.png", "image", "png");
	if (savePath.empty()) return;

	// backup viewport settings
	viewWidth = width;
	viewHeight = height;
	viewRays = rayCount;
	viewBounces = maxBounces;

	// apply render settings
	renderFlag = true;
	lockedMovement = true;
	changed = true;
	width = renderWidth;
	height = renderHeight;
	rayCount = renderRays;
	maxBounces = renderBounces;

	if (renderCamFlag) {
		// backup viewport cam settings
		viewCamPos = cam.pos;
		viewCamRot = cam.rotation;
		viewCamFov = cam.fov;
		viewCamFocusDistance = cam.focusDistance;
		viewCamAperture = cam.aperture;

		// apply render cam settings
		cam.pos = renderCamPos;
		cam.rotation = renderCamRot;
		cam.fov = renderCamFov;
		cam.focusDistance = renderCamFocusDistance;
		cam.aperture = renderCamAperture;
		cam.updateModel();
		cam.updateFov();
	}
}