#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>

#include "Object.h"

constexpr float defaultFov = 65.0f;
constexpr float defaultWindowWidth = 1408;
constexpr float defaultWindowHeight = 792;
constexpr int tileSize = 128;

constexpr const char* toneMapNames[5] = {
    "None", "Reinhard", "Jodie Reinhard", "ACES (fast)", "ACES (fitted)"
};

struct State {
    int accumulationFrame = 0;
    bool renderFlag = false;
    bool changed = false;
    bool lockedMovement = false;
};

struct RenderSettings {
    int width = 1280;
    int height = 720;
    int rayCount = 3;
    int maxBounces = 5;
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
