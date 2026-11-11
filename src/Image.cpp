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
#define STBI_ONLY_HDR
// Internal linkage: rigPlotFinders bundles its own stb_image; both packs can
// link into one app without symbol collisions.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

bool loadHdr(const std::string& path, FloatRaster& out, int maxEdge) {
	out = {};
	int w = 0;
	int h = 0;
	int comp = 0;
	float* pixels = stbi_loadf(path.c_str(), &w, &h, &comp, 4);
	if (!pixels || w < 1 || h < 1) {
		if (pixels) {
			stbi_image_free(pixels);
		}
		spdlog::warn("[rigImage] HDR load failed: {} ({})", path,
					 stbi_failure_reason() ? stbi_failure_reason() : "unknown");
		return false;
	}

	int dw = w;
	int dh = h;
	if (maxEdge > 0 && std::max(w, h) > maxEdge) {
		if (w >= h) {
			dw = maxEdge;
			dh = std::max(1, h * maxEdge / w);
		} else {
			dh = maxEdge;
			dw = std::max(1, w * maxEdge / h);
		}
	}

	out.width = dw;
	out.height = dh;
	out.rgba.resize(static_cast<size_t>(dw) * static_cast<size_t>(dh) * 4);
	if (dw == w && dh == h) {
		std::memcpy(out.rgba.data(), pixels, out.rgba.size() * sizeof(float));
	} else {
		for (int y = 0; y < dh; ++y) {
			const int y0 = y * h / dh;
			const int y1 = std::min(h, (y + 1) * h / dh);
			for (int x = 0; x < dw; ++x) {
				const int x0 = x * w / dw;
				const int x1 = std::min(w, (x + 1) * w / dw);
				float acc[4] = {0.f, 0.f, 0.f, 0.f};
				int n = 0;
				for (int sy = y0; sy < y1; ++sy) {
					for (int sx = x0; sx < x1; ++sx) {
						const float* p =
							pixels + (static_cast<size_t>(sy) * static_cast<size_t>(w) +
									  static_cast<size_t>(sx)) *
										 4;
						acc[0] += p[0];
						acc[1] += p[1];
						acc[2] += p[2];
						acc[3] += p[3];
						++n;
					}
				}
				float* d =
					out.rgba.data() +
					(static_cast<size_t>(y) * static_cast<size_t>(dw) + static_cast<size_t>(x)) * 4;
				const float inv = n > 0 ? 1.f / static_cast<float>(n) : 0.f;
				d[0] = acc[0] * inv;
				d[1] = acc[1] * inv;
				d[2] = acc[2] * inv;
				d[3] = acc[3] * inv;
			}
		}
	}
	stbi_image_free(pixels);
	return true;
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

bool savePng(const std::string& path, const pixel::Raster& raster) {
	if (raster.width < 1 || raster.height < 1 ||
		raster.rgba.size() <
			static_cast<size_t>(raster.width) * static_cast<size_t>(raster.height) * 4) {
		return false;
	}
	const int ok = stbi_write_png(path.c_str(), raster.width, raster.height, 4, raster.rgba.data(),
								  raster.width * 4);
	if (!ok) {
		spdlog::warn("[rigImage] PNG write failed: {}", path);
	}
	return ok != 0;
}

} // namespace image
} // namespace rig
