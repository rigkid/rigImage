# image

![preview](img/preview.png)

Thin **rigImage** demo: decode `<exeDir>/data/dot.png` and show it.

```bash
cmake -S packs/rigImage/examples/image -B packs/rigImage/examples/image/build
cmake --build packs/rigImage/examples/image/build --target image
```

`--smoke-image` decodes the PNG and exits 0.
