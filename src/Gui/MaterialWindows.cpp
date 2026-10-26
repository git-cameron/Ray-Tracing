#include "GUI.h"

void GUI::materialWindows(Scene& scene, State& state, const ImGuiWindowFlags windowFlags) {
    // material customisation
    if (editMaterialsFlag) {
        if (Begin("Materials", &editMaterialsFlag, windowFlags)) {

            // tree buttons
            bool openAllMat = false, closeAllMat = false;
            if (Button("Expand All")) openAllMat = true;
            SameLine();
            if (Button("Close All")) closeAllMat = true;

            if (!newMaterialFlag) {
                if (Button("New Material")) {
                    tempMaterial = Material{};
                    newMaterialFlag = true;
                }
            }

            // list all materials
            for (int i = 0; i < scene.materials.size(); i++) {
                if (openAllMat) SetNextItemOpen(true);
                if (closeAllMat) SetNextItemOpen(false);

                if (TreeNodeEx(("Material " + std::to_string(i)).c_str())) {
                    Material* mat = &scene.materials[i];
                    state.changed = editMaterial(mat) || state.changed;
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
            if (Button("Add Material")) scene.materials.push_back(tempMaterial);
        }
        End();
    }
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
    if (Checkbox("Glass", &mat->glass)) changed = true;
    if (mat->glass) {
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