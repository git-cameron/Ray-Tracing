#include "GUI.h"

void GUI::renderSettingsWindow(RenderSettings& render, ToneSettings& tone, State& state, Camera& cam, const ImVec2& viewSize, const ImGuiWindowFlags windowFlags) {
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
					Text("Resolution %ix%i", render.width, render.height);
				}
				else {
					if (InputInt("Width##Viewport", &render.width, 0) && render.width < 1) render.width = viewSize.x;
					if (InputInt("Height##Viewport", &render.height, 0) && render.height < 1) render.height = viewSize.y;
					gcd = GCD(render.width, render.height);
					Text("Aspect ratio %i:%i", render.width / gcd, render.height / gcd);
				}
				Separator();

				// parameters
				if (InputInt("Ray Count##Viewport", &render.rayCount, 0)) {
					if (render.rayCount < 1) render.rayCount = 1;
					state.changed = true;
				}
				if (InputInt("Max Bounces##Viewport", &render.maxBounces, 0)) {
					if (render.maxBounces < 0) render.maxBounces = 0;
					state.changed = true;
				}
				Separator();
			}

			if (CollapsingHeader("Render")) {

				// render resolution
				if (InputInt("Width##Render", &renderWidth, 0) && renderWidth < 1) renderWidth = viewSize.x;
				if (InputInt("Height##Render", &renderHeight, 0) && renderHeight < 1) renderHeight = viewSize.y;
				gcd = GCD(renderWidth, renderHeight);
				Text("Aspect ratio %i:%i", renderWidth / gcd, renderHeight / gcd);
				Separator();

				// parameters
				if (InputInt("Frames##Render", &renderFrames, 0) && renderFrames < 1) renderFrames = 1;
				if (InputInt("Ray Count##Render", &renderRays, 0) && renderRays < 1) renderRays = 1;
				if (InputInt("Max Bounces##Render", &renderBounces, 0) && renderBounces < 0) renderBounces = 0;
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
				if (DragFloat("Focus##Render", &renderCamFocusDistance, 0.05, 0) && renderCamFocusDistance < 0.1f) renderCamFocusDistance = 0.1f;
				SliderFloat("Aperture##Render", &renderCamAperture, 0, 2.0f, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
			}

			// tone map settings
			if (CollapsingHeader("Tone Mapping")) {
				SliderFloat("Exposure", &tone.exposure, 0.1f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_AlwaysClamp);
				Combo("Tone Map", &tone.tonemap, toneMapNames, 5);
				Checkbox("Grayscale", &tone.grayScale);
			}

			SeparatorText("Other");

			if (Checkbox("Environment Light", &render.environmentLight)) state.changed = true;

			Checkbox("Render Camera##Checkbox", &renderCamFlag);

			if (Button("Set Camera")) {
				renderCamPos = cam.pos;
				renderCamRot = cam.rotation;
				renderCamFov = cam.fov;
			}

			if (Button("Start Render")) startRender(state, render, cam);
		}
		End();
	}
}