#include "GUI.h"

void GUI::sphereWindows(Scene& scene, State& state, const ImGuiWindowFlags windowFlags) {
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
			if (!scene.spheres.empty()) {
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
						scene.spheres.clear();
						state.changed = true;
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
			for (int i = 0; i < scene.spheres.size(); i++) {
				if (openAllSphere) SetNextItemOpen(true);
				if (closeAllSphere) SetNextItemOpen(false);

				if (TreeNodeEx(("Sphere " + std::to_string(i)).c_str())) {
					Sphere* sphere = &scene.spheres[i];
					state.changed = editSphere(sphere, scene.materials) || state.changed;

					// delete spheres
					SeparatorText("Delete Sphere");
					if (Button(("Delete##1" + std::to_string(i)).c_str())) {
						scene.spheres.erase(scene.spheres.begin() + i);
						state.changed = true;
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
			editSphere(&tempSphere, scene.materials);
			Separator();
			if (Button("Add Sphere")) {
				scene.spheres.push_back(tempSphere);
				state.changed = true;
			}
		}
		End();
	}
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