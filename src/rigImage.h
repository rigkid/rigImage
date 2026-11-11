#pragma once

#include "core/pack/IPack.h"
#include "Image.h"

namespace rigkit {

/**
 * @brief Raster decode + GLES upload.
 * @details CPU buffer is `rig::pixel::Raster` (`rig.pixel.raster`). Apps call
 * `rig::image::load` / `loadTexture`. Path identity stays on `CAssetRef` in
 * **rigComponent**. This pack does not own document POD.
 */
class rigImage : public IPack {
  public:
	rigImage();
	bool init() override;
	void setup() override;
};

} // namespace rigkit
