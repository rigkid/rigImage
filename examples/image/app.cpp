#include "app.h"

#include "rendering/U_gladGlfw.h"
#include <cstdlib>
#include <iostream>
#include <spdlog/spdlog.h>
#include "core/RigKitEngine.h"
#include "core/pack/MPack.h"
#include "core/util/AppPaths.h"
#include "packs/rigComponent/src/rigComponent.h"
#include "packs/rigImGui/src/MWindow.h"
#include "packs/rigImGui/src/Mui.h"
#include "packs/rigImGui/src/rigImGui.h"
#include "packs/rigProject/src/rigProject.h"
#include "packs/rigSystems/src/rigSystems.h"
#include "ImageWindow.h"
#include "rigImage.h"

ImageApp::ImageApp() {
	window().width = 720;
	window().height = 480;
	window().title = "rigImage — image";
	settings().appName = "image";
}

void ImageApp::parseCommandLineArgs(const rigkit::CommandLineArgs& args) {
	rigkit::IApp::parseCommandLineArgs(args);
	if (args.hasFlag("smoke-image")) {
		m_smoke = true;
	}
	if (args.hasFlag("help") || args.hasFlag("h")) {
		std::cout << "image — rigImage decode demo\n\n"
				  << "  --smoke-image    Decode data/dot.png, exit\n"
				  << "  --help\n";
		std::exit(0);
	}
}

void ImageApp::bootstrapPacks() {
	auto* packs = getEngine()->getPackManager();
	if (!packs) {
		return;
	}
	packs->registerPack<rigkit::rigComponent>();
	packs->registerPack<rigkit::rigSystems>();
	packs->registerPack<rigkit::rigProject>();
	packs->registerPack<rigkit::rigImGui>();
	packs->registerPack<rigkit::rigImage>();
	packs->initAll();
	packs->setupAll();
}

bool ImageApp::runSmoke() {
	const std::string path = AppPaths::joinPath(AppPaths::getDataDir(), "dot.png");
	if (!rig::image::resolvePath("https://example.com/a.png").empty()) {
		return false;
	}
	rig::pixel::Raster pixels;
	if (!rig::image::load(path, pixels)) {
		return false;
	}
	return pixels.width == 1 && pixels.height == 1 && pixels.rgba.size() == 4;
}

void ImageApp::setupUi() {
	auto* ui = getEngine()->getUiManager();
	if (!ui) {
		spdlog::warn("image: no IMui — rigImGui failed to attach");
		return;
	}
	auto* mui = dynamic_cast<rigkit::Mui*>(ui);
	auto* wm = mui ? mui->getWindowManager() : nullptr;
	if (!wm) {
		return;
	}
	wm->createWindow<ImageWindow>(this);
	wm->showWindow("Image");
	if (mui) {
		mui->setFirstRunHostDockLayout({"Image"});
	}
}

void ImageApp::setup() {
	spdlog::info("image setup");
	m_engine->setClearColor(0.12f, 0.14f, 0.18f, 1.0f);
	bootstrapPacks();

	if (m_smoke) {
		const bool ok = runSmoke();
		if (!ok) {
			spdlog::error("image --smoke-image failed");
		} else {
			spdlog::info("image --smoke-image OK");
		}
		if (auto* win = getEngine()->getWindow()) {
			glfwSetWindowShouldClose(win, GLFW_TRUE);
		}
		if (!ok) {
			std::exit(1);
		}
		return;
	}

	const std::string path = AppPaths::joinPath(AppPaths::getDataDir(), "dot.png");
	if (!rig::image::loadTexture(path, m_tex)) {
		m_error = "failed to load " + path;
		spdlog::error("image: {}", m_error);
	}
	setupUi();
}

void ImageApp::exit() {
	rig::image::destroy(m_tex);
	spdlog::info("image exit");
}
