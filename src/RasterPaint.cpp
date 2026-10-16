#include "RasterPaint.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace rig {
namespace image {

int paintDab(pixel::Raster& raster, const glm::vec2& center, float radiusPx,
			 const glm::vec4& rgba) {
	if (raster.width < 1 || raster.height < 1 || radiusPx < 0.5f || rgba.a <= 0.f) {
		return 0;
	}
	const int x0 = std::max(0, static_cast<int>(std::floor(center.x - radiusPx)));
	const int y0 = std::max(0, static_cast<int>(std::floor(center.y - radiusPx)));
	const int x1 = std::min(raster.width - 1, static_cast<int>(std::ceil(center.x + radiusPx)));
	const int y1 = std::min(raster.height - 1, static_cast<int>(std::ceil(center.y + radiusPx)));
	const float srcR = rgba.r * 255.f;
	const float srcG = rgba.g * 255.f;
	const float srcB = rgba.b * 255.f;
	int touched = 0;
	for (int y = y0; y <= y1; ++y) {
		for (int x = x0; x <= x1; ++x) {
			const float dx = static_cast<float>(x) - center.x;
			const float dy = static_cast<float>(y) - center.y;
			const float d = std::sqrt(dx * dx + dy * dy);
			// One-pixel anti-aliased rim; solid core.
			const float cover = std::clamp(radiusPx - d + 0.5f, 0.f, 1.f);
			if (cover <= 0.f) {
				continue;
			}
			const float a = cover * rgba.a;
			std::uint8_t* p = raster.rgba.data() + (static_cast<size_t>(y) * raster.width + x) * 4;
			p[0] = static_cast<std::uint8_t>(p[0] + (srcR - p[0]) * a + 0.5f);
			p[1] = static_cast<std::uint8_t>(p[1] + (srcG - p[1]) * a + 0.5f);
			p[2] = static_cast<std::uint8_t>(p[2] + (srcB - p[2]) * a + 0.5f);
			p[3] = static_cast<std::uint8_t>(std::min(255.f, p[3] + (255.f - p[3]) * a + 0.5f));
			++touched;
		}
	}
	return touched;
}

int paintStroke(pixel::Raster& raster, const glm::vec2& from, const glm::vec2& to, float radiusPx,
				const glm::vec4& rgba) {
	const float len = glm::length(to - from);
	const float spacing = std::max(0.5f, radiusPx * 0.25f);
	const int steps = std::max(1, static_cast<int>(std::ceil(len / spacing)));
	int touched = 0;
	for (int i = 0; i <= steps; ++i) {
		const float t = static_cast<float>(i) / static_cast<float>(steps);
		touched += paintDab(raster, from + (to - from) * t, radiusPx, rgba);
	}
	return touched;
}

} // namespace image
} // namespace rig
