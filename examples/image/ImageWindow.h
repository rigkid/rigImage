#pragma once

#include "packs/rigImGui/src/IWindow.h"

class ImageApp;

class ImageWindow : public rigkit::IWindow {
  public:
	explicit ImageWindow(ImageApp* app);

  protected:
	void renderContents() override;

  private:
	ImageApp* m_app = nullptr;
};
