# rigImage

Raster decode + GLES upload for **RigKit**. Vendors [stb_image](https://github.com/nothings/stb)
(PNG / JPEG / GIF / BMP / TGA). Path identity stays on `CAssetRef` in
**rigComponent** — this pack does not own document POD.

```cpp
#include "Image.h"

rig::pixel::Raster pixels;
rig::image::load(path, pixels);

rig::image::GpuTexture tex;
rig::image::loadTexture(path, tex);
// ... ImGui::Image(ImTextureRef((ImTextureID)tex.tex), ...)
rig::image::destroy(tex);
```

`resolvePath(href, base)` tries the path as written, then `base`, then
`<exeDir>/data`. URLs return empty — this pack does not fetch.

Long edge over 2048 is downsampled before the raster is kept (Pi). Pass
`maxEdge = 0` to skip the cap.

## Example

```bash
cmake -S packs/rigImage/examples/image -B packs/rigImage/examples/image/build
cmake --build packs/rigImage/examples/image/build --target image
# packs/rigImage/examples/image/build/bin/image --smoke-image
```

[API/docs](https://rigkid.github.io/rigImage/)
