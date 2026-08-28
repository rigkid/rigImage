#include "rigImage.h"

#include <memory>
#include <spdlog/spdlog.h>
#include "core/RigKitEngine.h"
#include "core/pack/PackRegistry.h"

namespace rigkit {

rigImage::rigImage() : IPack("rigImage") {}

bool rigImage::init() {
	spdlog::info("[rigImage] init");
	return true;
}

void rigImage::setup() {
	if (!getEngine()) {
		return;
	}
	spdlog::info("[rigImage] setup");
}

} // namespace rigkit

namespace {
struct rigImageRegistrar {
	rigImageRegistrar() {
		rigkit::PackRegistry::instance().addFactory("rigImage", []() {
			return std::shared_ptr<rigkit::IPack>(std::make_shared<rigkit::rigImage>());
		});
	}
};
static rigImageRegistrar rigImage_auto_reg;
} // namespace
