#pragma once

#include "core/IApp.h"
#include "core/util/CommandLineArgs.h"
#include "Image.h"

class ImageApp : public rigkit::IApp {
  public:
	ImageApp();

	void parseCommandLineArgs(const rigkit::CommandLineArgs& args) override;
	void setup() override;
	void update(float) override {}
	void draw() override {}
	void exit() override;

	const rig::image::GpuTexture& texture() const { return m_tex; }
	const std::string& error() const { return m_error; }

  private:
	void bootstrapPacks();
	void setupUi();
	bool runSmoke();

	bool m_smoke = false;
	rig::image::GpuTexture m_tex;
	std::string m_error;
};
