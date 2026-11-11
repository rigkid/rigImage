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

## Style

Format first-party `src/` and `examples/` with clang-format before commit (never `third_party/`).

```bash
./tools/format.sh
./tools/install-hooks.sh    # once per clone - pre-commit formats staged C/C++
```

Hooks **fail** if you stage C/C++ and `clang-format` is missing. Put it on PATH:

- **Windows:** install [LLVM](https://github.com/llvm/llvm-project/releases) (enable "Add LLVM to the system PATH"), or `winget install LLVM.LLVM`, then open a new shell. Confirm: `clang-format --version`.
- **macOS:** `brew install clang-format`
- **Linux:** `sudo apt install clang-format`

From a RigKit host checkout, `tools\install-hooks.bat` / `./tools/install-hooks.sh` wires every pack repo. Full policy: [RigKit packs Style](https://github.com/rigkid/RigKit/blob/main/packs/README.md#style).

[API/docs](https://rigkid.github.io/rigImage/)
