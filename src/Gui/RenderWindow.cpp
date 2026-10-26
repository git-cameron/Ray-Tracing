#include "GUI.h"

void GUI::renderWindow(const GLuint& texture, RenderSettings& render, State& state, Camera& cam, ImVec2& viewSize) const {
    // render finished
    if (state.accumulationFrame >= renderFrames || IsKeyPressed(ImGuiKey_Escape)) {
        saveImage(savePath, render.width, render.height);
        state.renderFlag = false;

        // restore viewport settings
        render.width = viewWidth;
        render.height = viewHeight;
        render.rayCount = viewRays;
        render.maxBounces = viewBounces;

        if (renderCamFlag) {
            cam.pos = viewCamPos;
            cam.rotation = viewCamRot;
            cam.fov = viewCamFov;
            cam.focusDistance = viewCamFocusDistance;
            cam.aperture = viewCamAperture;
            cam.updateModel();
            cam.updateFocus(render.width);
        }

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

    // backup viewport settings
    viewWidth = render.width;
    viewHeight = render.height;
    viewRays = render.rayCount;
    viewBounces = render.maxBounces;

    // apply render settings
    state.renderFlag = true;
    state.lockedMovement = true;
    state.changed = true;
    render.width = renderWidth;
    render.height = renderHeight;
    render.rayCount = renderRays;
    render.maxBounces = renderBounces;

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
        cam.updateFocus(render.width);
    }
}