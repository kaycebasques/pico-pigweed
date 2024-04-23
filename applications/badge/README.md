## How to update your name

1. Edit `name_tag.png` with your name, save as a png.

2. Run the following to update `name_tag.h`:

   ```sh
   cd applications/badge
   python -m pw_graphics.png2cc --output-mode rgb565 name_tag.png -W 152 -H 64
   ```

## How to regenerate the sprite header files:

```sh
python -m pw_graphics.png2cc --output-mode rgb565 -W 54 -H 37 kudzu_isometric_text_sprite.png --transparent-color 255,0,255
python -m pw_graphics.png2cc --output-mode rgb565 -W 8 -H 8 heart_8x8.png
python -m pw_graphics.png2cc --output-mode rgb565 -W 5 -H 7 pw_logo5x7.png
python -m pw_graphics.png2cc --output-mode rgb565 -W 46 -H 10 pw_banner46x10.png
python -m pw_graphics.png2cc --output-mode rgb565 hello_my_name_is65x42.png -W 65 -H 42
```

