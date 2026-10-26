#include "GUI.h"

void GUI::modelWindows(Scene& scene, State& state, const ImGuiWindowFlags windowFlags) {
	// model customisation
	if (editModelsFlag) {
		if (Begin("Models", &editModelsFlag, windowFlags)) {
			// tree buttons
			bool openAllModels = false, closeAllModels = false;
			if (Button("Expand All##2")) openAllModels = true;
			SameLine();
			if (Button("Close All##2")) closeAllModels = true;

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
			if (!scene.models.empty()) {
				if (Button("Delete All##1")) OpenPopup("Delete?");

				// pop up
				SetNextWindowPos(GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				if (BeginPopupModal("Delete?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					Text("Delete All Models?");
					Separator();
					if (Button("Confirm")) {
						scene.models.clear();
						scene.modelExtras.clear();
						scene.triangles.clear();
						scene.nodes.clear();
						scene.nodeTriIndex.clear();
						state.changed = true;
						CloseCurrentPopup();
					}
					SameLine();
					if (Button("Cancel")) CloseCurrentPopup();
					EndPopup();
				}
			}

			// list all models
			for (int i = 0; i < scene.models.size(); i++) {
				if (openAllModels) SetNextItemOpen(true);
				if (closeAllModels) SetNextItemOpen(false);

				if (TreeNodeEx(("Model " + std::to_string(i)).c_str())) {
					// edit model
					Model* model = &scene.models[i];
					ModelExtra* extra = &scene.modelExtras[i];
					state.changed = editModel(model, extra, scene.materials) || state.changed;

					// delete models
					SeparatorText("Delete Model");
					if (Button(("Delete##2" + std::to_string(i)).c_str())) {
						const int bvhDiff = i == scene.models.size() - 1 ? 0 : scene.models[i + 1].BVHIndex - scene.models[i].BVHIndex;
						deleteBVH(scene.models[i].BVHIndex, bvhDiff == 0 ? -1 : scene.models[i + 1].BVHIndex, scene.nodes, scene.nodeTriIndex);

						const int modelSize = scene.models[i].endIndex - scene.models[i].startIndex + 1;
						scene.triangles.erase(scene.triangles.begin() + scene.models[i].startIndex, scene.triangles.begin() + scene.models[i].endIndex + 1);
						scene.models.erase(scene.models.begin() + i);
						scene.modelExtras.erase(scene.modelExtras.begin() + i);
						for (int j = i; j < scene.models.size(); j++) {
							scene.models[j].startIndex -= modelSize;
							scene.models[j].endIndex -= modelSize;
							scene.models[j].BVHIndex -= bvhDiff;
						}
						state.changed = true;
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
			editModel(&tempModel, &tempModelExtra, scene.materials);
			Separator();

			// gets model path
			if (Button((modelPath + "##NewModel").c_str())) {
				modelPath = getLoadPath("model", "obj");
				if (modelPath.empty()) modelPath = "Select Model";
			}
			SetItemTooltip("Model Path (.obj)");

			if (Button("Add Model")) {
				if (loadModel(modelPath, scene.models, scene.modelExtras, scene.triangles)) {
					addBVH(scene.models[scene.models.size() - 1], scene.nodes, scene.nodeTriIndex, scene.triangles);
					scene.modelExtras[scene.modelExtras.size() - 1] = tempModelExtra;
					scene.models[scene.models.size() - 1].transform = tempModel.transform;
					scene.models[scene.models.size() - 1].invTransform = tempModel.invTransform;
					scene.models[scene.models.size() - 1].matIndex = tempModel.matIndex;
					state.changed = true;
				}
			}
		}
		End();
	}
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

    if (SliderInt("Material##EditModel", &model->matIndex, 0, materials.size() - 1, "%i", ImGuiSliderFlags_AlwaysClamp)) changed = true;

    return changed;
}