#include "GUI.h"

void GUI::renderWindow(const GLuint& texture, RenderSettings& render, State& state, Camera& cam, ImVec2& viewSize) const {
    // render finished
    if (state.accumulationFrame >= renderFrames || IsKeyPressed(ImGuiKey_Escape)) {
        saveImage(savePath, render.width, render.height);
        state.renderFlag = false;

        render = backupRenderSettings;
        if (renderCamFlag) cam = backupCam;
        state.changed = true;
        return;
    }

    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    SetNextWindowSize(GetIO().DisplaySize);
    SetNextWindowPos(ImVec2(0, 0));
    if (Begin("Render", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        viewSize = GetContentRegionAvail();
        Image(texture, ImVec2(viewSize[0], viewSize[1] - GetFrameHeightWithSpacing()), ImVec2(0, 1), ImVec2(1, 0));
        ProgressBar(static_cast<float>(state.accumulationFrame) / renderFrames, ImVec2(-1.0f, 0.0f), (std::to_string(state.accumulationFrame) + "/" + std::to_string(renderFrames)).c_str());
    }
    End();

    PopStyleVar();
}


void GUI::startRender(State& state, RenderSettings& render, Camera& cam) {
    savePath = getSavePath("render.png", "image", "png");
    if (savePath.empty()) return;

    backupRenderSettings = render;

    // apply render settings
    state.renderFlag = true;
    state.lockedMovement = true;
    state.changed = true;
    render = renderSettings;

    if (renderCamFlag) {
        backupCam = cam;
        cam = renderCam;
        cam.updateModel();
        cam.updateFocus(renderSettings.width);
    }
}