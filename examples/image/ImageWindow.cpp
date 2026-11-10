#include "ImageWindow.h"

#include <imgui.h>
#include "app.h"

ImageWindow::ImageWindow(ImageApp* app) : rigkit::ImGuiPanel("Image", 0), m_app(app) {}

void ImageWindow::renderContents() {
	if (!m_app) {
		return;
	}
	if (!m_app->error().empty()) {
		ImGui::TextColored(ImVec4(1.f, 0.4f, 0.3f, 1.f), "%s", m_app->error().c_str());
		return;
	}
	const auto& tex = m_app->texture();
	if (tex.tex == 0) {
		ImGui::TextDisabled("No texture.");
		return;
	}
	ImGui::Text("dot.png  %d x %d", tex.w, tex.h);
	const float scale = 64.f;
	ImGui::Image(ImTextureRef(static_cast<ImTextureID>(tex.tex)), ImVec2(scale, scale));
}
