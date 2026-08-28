#pragma once

/**
 * @file
 * @brief CPU RGBA pixel buffer shared by decode, FBO readback, and upload paths.
 */

#include <cstdint>
#include <vector>

namespace rig {
namespace pixel {

/**
 * @brief `rig.pixel.raster` role. Schema literals: working, output,
 * layerPixels, mask, composite.
 */
enum class RasterRole {
	Working,
	Output,
	LayerPixels,
	Mask,
	Composite
};

/**
 * @brief CPU RGBA buffer. Speaks `rig.pixel.raster`.
 * @details Not a property datatype and not an ECS component. Decode, FBO
 * readback, and upload share this bag. No texture or FBO ids. Decode leaves
 * `role` as the caller set it (default `Working`).
 */
struct Raster {
	RasterRole role = RasterRole::Working;
	int width = 0;
	int height = 0;
	std::vector<std::uint8_t> rgba;
};

} // namespace pixel
} // namespace rig
