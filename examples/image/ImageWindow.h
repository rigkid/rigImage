#pragma once

#include "packs/rigImGui/src/ImGuiPanel.h"

class ImageApp;

class ImageWindow : public rigkit::ImGuiPanel {
  public:
	explicit ImageWindow(ImageApp* app);

  protected:
	void renderContents() override;

  private:
	ImageApp* m_app = nullptr;
};
