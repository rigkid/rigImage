#pragma once

/**
 * @file
 * @brief Load image files into a CPU raster and upload them as GLES textures.
 * @details Texture names live in `GpuTexture`, not in a component, and must be destroyed while
 * the GL context is alive. The long edge is capped before upload so Pi memory stays sane.
 */

#include <string>
#include <vector>

#include "Raster.h"

namespace rig {
namespace image {

/// GLES texture name. Destroy while the GL context is alive.
struct GpuTexture {
	unsigned int tex = 0;
	int w = 0;
	int h = 0;
};

/// Long-edge cap before upload. 0 = no cap.
constexpr int kDefaultMaxEdge = 2048;

/// Long-edge cap for a float HDR decode. Spherical harmonics do not need the source size.
constexpr int kDefaultHdrMaxEdge = 256;

/**
 * @brief Linear RGBA float image. Not a component and not uploaded by itself.
 * @details Radiance `.hdr` lands here. Values are linear, not display-encoded.
 */
struct FloatRaster {
	int width = 0;
	int height = 0;
	std::vector<float> rgba;
};

/// Resolve a local href: as written, then @p basePath, then `<exeDir>/data`.
/// Empty when the href is a URL or no file exists.
std::string resolvePath(const std::string& href, const std::string& basePath = {});

/** @brief Pixel width/height without a full decode. False when the file is not an image. */
bool size(const std::string& path, int& w, int& h);
bool load(const std::string& path, pixel::Raster& out, int maxEdge = kDefaultMaxEdge);
/** @brief Decode a Radiance HDR (or any stb float image) into @p out. */
bool loadHdr(const std::string& path, FloatRaster& out, int maxEdge = kDefaultHdrMaxEdge);
bool loadMemory(const void* data, int bytes, pixel::Raster& out, int maxEdge = kDefaultMaxEdge);
bool upload(const pixel::Raster& raster, GpuTexture& out);
bool loadTexture(const std::string& path, GpuTexture& out, int maxEdge = kDefaultMaxEdge);
void destroy(GpuTexture& tex);

/** @brief Write @p raster as PNG. Parent folder must exist. */
bool savePng(const std::string& path, const pixel::Raster& raster);

} // namespace image
} // namespace rig
