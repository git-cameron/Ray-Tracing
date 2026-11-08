#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>

#include "Object.h"

constexpr float defaultFov = 65.0f;
constexpr float defaultSensitivity = 5.0f;

constexpr int defaultViewWidth = 1280;
constexpr int defaultViewHeight = 720;
constexpr int defaultViewRays = 3;
constexpr int defaultViewBounces = 5;

constexpr int defaultRenderFrames = 1000;
constexpr int defaultRenderRays = 1;
constexpr int defaultRenderBounces = 25;
constexpr int defaultRenderWidth = 1920;
constexpr int defaultRenderHeight = 1080;

constexpr float defaultWindowWidth = 1408;
constexpr float defaultWindowHeight = 792;
constexpr int tileSize = 128;

constexpr const char* toneMapNames[5] = {
    "None", "Reinhard", "Jodie Reinhard", "ACES (fast)", "ACES (fitted)"
};

struct State {
    int accumulationFrame = 0;
    bool renderFlag = false;
    bool lockedMovement = false;
    bool changed = false;
    bool spheresChanged = false;
    bool materialsChanged = false;
    bool modelsChanged = false;
    bool BVHChanged = false;
};

struct RenderSettings {
    int width = defaultViewWidth;
    int height = defaultViewHeight;
    int rayCount = defaultViewRays;
    int maxBounces = defaultViewBounces;
    bool environmentLight = true;
};

struct ToneSettings {
    int tonemap = 0; // None
    float exposure = 1.0f;
    bool grayScale = false;
};

struct Scene {
    std::vector<Material> materials {Material{}};
    std::vector<Sphere> spheres;
    std::vector<Model> models;
    std::vector<ModelExtra> modelExtras;
    std::vector<Triangle> triangles;
    std::vector<BVHNode> nodes;
    std::vector<int> nodeTriIndex;
};


#endif
