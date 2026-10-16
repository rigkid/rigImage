#pragma once

/**
 * @file
 * @brief CPU brush dabs into a `Raster` - the pixel half of an editor Brush tool.
 * @details Pure pixel writes, no GL and no UI. The caller owns the raster,
 * re-uploads the texture, and persists the file.
 */

#include <glm/glm.hpp>
#include "Raster.h"

namespace rig {
namespace image {

/**
 * @brief Stamp one soft round dab, alpha-blended over the pixels.
 * @param center Dab centre in raster pixels.
 * @param radiusPx Dab radius in pixels (>= 0.5 to mark anything).
 * @param rgba Brush colour; `a` scales dab opacity.
 * @return Pixels touched. 0 when the dab misses the raster.
 */
int paintDab(pixel::Raster& raster, const glm::vec2& center, float radiusPx, const glm::vec4& rgba);

/**
 * @brief Stamp dabs along a segment so a drag reads as one stroke.
 * @details Dab spacing is a quarter radius - close enough that round caps
 * fuse into a band. Endpoints included.
 * @return Pixels touched over all dabs.
 */
int paintStroke(pixel::Raster& raster, const glm::vec2& from, const glm::vec2& to, float radiusPx,
				const glm::vec4& rgba);

} // namespace image
} // namespace rig
