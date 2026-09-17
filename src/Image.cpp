#include "Image.h"

#include "rendering/U_gladGlfw.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <spdlog/spdlog.h>
#include "core/util/AppPaths.h"

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_ONLY_TGA
// Internal linkage: rigPlotFinders bundles its own stb_image; both packs can
// link into one app without symbol collisions.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace rig {
namespace image {

namespace {

bool looksLikeUrl(const std::string& href) {
	if (href.find("://") != std::string::npos) {
		return true;
	}
	return href.size() >= 4 && href.compare(0, 4, "www.") == 0;
}

void boxDownsample(const unsigned char* src, int sw, int sh, unsigned char* dst, int dw, int dh) {
	for (int y = 0; y < dh; ++y) {
		const int sy = y * sh / dh;
		for (int x = 0; x < dw; ++x) {
			const int sx = x * sw / dw;
			const unsigned char* p =
				src +
				(static_cast<size_t>(sy) * static_cast<size_t>(sw) + static_cast<size_t>(sx)) * 4;
			unsigned char* q =
				dst +
				(static_cast<size_t>(y) * static_cast<size_t>(dw) + static_cast<size_t>(x)) * 4;
			std::memcpy(q, p, 4);
		}
	}
}

bool rasterFromPixels(unsigned char* pixels, int w, int h, pixel::Raster& out, int maxEdge,
					  const char* label) {
	if (!pixels || w < 1 || h < 1) {
		spdlog::debug("[rigImage] decode failed: {} ({})", label ? label : "",
					  stbi_failure_reason() ? stbi_failure_reason() : "decode");
		if (pixels) {
			stbi_image_free(pixels);
		}
		return false;
	}

	int uw = w;
	int uh = h;
	const unsigned char* src = pixels;
	std::vector<std::uint8_t> shrink;
	if (maxEdge > 0) {
		const int edge = std::max(w, h);
		if (edge > maxEdge) {
			uw = std::max(1, w * maxEdge / edge);
			uh = std::max(1, h * maxEdge / edge);
			shrink.resize(static_cast<size_t>(uw) * static_cast<size_t>(uh) * 4);
			boxDownsample(pixels, w, h, shrink.data(), uw, uh);
			src = shrink.data();
			spdlog::info("[rigImage] downsampled {}x{} -> {}x{} ({})", w, h, uw, uh,
						 label ? label : "");
		}
	}

	out.width = uw;
	out.height = uh;
	out.rgba.assign(src, src + static_cast<size_t>(uw) * static_cast<size_t>(uh) * 4);
	stbi_image_free(pixels);
	return true;
}

} // namespace

std::string resolvePath(const std::string& href, const std::string& basePath) {
	if (href.empty() || looksLikeUrl(href)) {
		return {};
	}

	std::error_code ec;
	const std::filesystem::path asWritten(href);
	if (std::filesystem::exists(asWritten, ec)) {
		return asWritten.string();
	}
	if (!basePath.empty()) {
		const std::filesystem::path underBase = std::filesystem::path(basePath) / href;
		if (std::filesystem::exists(underBase, ec)) {
			return underBase.string();
		}
	}
	const std::filesystem::path underData = std::filesystem::path(AppPaths::getDataDir()) / href;
	if (std::filesystem::exists(underData, ec)) {
		return underData.string();
	}
	return {};
}

bool size(const std::string& path, int& w, int& h) {
	w = 0;
	h = 0;
	int comp = 0;
	if (!stbi_info(path.c_str(), &w, &h, &comp) || w < 1 || h < 1) {
		return false;
	}
	return true;
}

bool load(const std::string& path, pixel::Raster& out, int maxEdge) {
	const auto role = out.role;
	out = {};
	out.role = role;
	int w = 0;
	int h = 0;
	int comp = 0;
	unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &comp, 4);
	return rasterFromPixels(pixels, w, h, out, maxEdge, path.c_str());
}

bool loadMemory(const void* data, int bytes, pixel::Raster& out, int maxEdge) {
	const auto role = out.role;
	out = {};
	out.role = role;
	if (!data || bytes <= 0) {
		return false;
	}
	int w = 0;
	int h = 0;
	int comp = 0;
	unsigned char* pixels =
		stbi_load_from_memory(static_cast<const stbi_uc*>(data), bytes, &w, &h, &comp, 4);
	return rasterFromPixels(pixels, w, h, out, maxEdge, "memory");
}

bool upload(const pixel::Raster& raster, GpuTexture& out) {
	destroy(out);
	if (raster.width < 1 || raster.height < 1 ||
		raster.rgba.size() <
			static_cast<size_t>(raster.width) * static_cast<size_t>(raster.height) * 4) {
		return false;
	}

	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	GLint unpack = 4;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &unpack);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, raster.width, raster.height, 0, GL_RGBA,
				 GL_UNSIGNED_BYTE, raster.rgba.data());
	glPixelStorei(GL_UNPACK_ALIGNMENT, unpack);
	glBindTexture(GL_TEXTURE_2D, 0);

	out.tex = tex;
	out.w = raster.width;
	out.h = raster.height;
	return true;
}

bool loadTexture(const std::string& path, GpuTexture& out, int maxEdge) {
	pixel::Raster raster;
	if (!load(path, raster, maxEdge)) {
		return false;
	}
	return upload(raster, out);
}

void destroy(GpuTexture& tex) {
	if (tex.tex != 0) {
		GLuint name = tex.tex;
		glDeleteTextures(1, &name);
		tex.tex = 0;
	}
	tex.w = 0;
	tex.h = 0;
}

} // namespace image
} // namespace rig
