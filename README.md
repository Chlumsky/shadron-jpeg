
# Shadron-JPEG extension

This is an official extension for [Shadron](http://www.shadron.info/)
which adds the functionality to export JPEG files via
[libjpeg](http://ijg.org/).

### List of features
 - JPEG file export with configurable quality (0.0 to 1.0)
 - preview image of JPEG compression with its size in MB

## Installation

### Windows

Place `shadron-jpeg.dll` in the `extensions` directory next to Shadron's executable, or better yet,
in `%APPDATA%\Shadron\extensions`. It will be automatically detected by Shadron on next launch.
Requires Shadron 1.1.3 or later.

### Mac

Place `shadron-jpeg.dylib` into `~/.config/Shadron/extensions`.
It will be automatically detected by Shadron on next launch.
Requires Shadron 1.4.2 or later.

## Usage

Before using any objects specific to this extension, it must first be enabled with the following directive:

    #extension jpeg

To export an image as a JPEG file, you may declare a JPEG export like this:

    export jpeg(MyImage, "output.jpg", <quality>);

Quality must be a floating-point value between 0 and 1.

To preview how an image might look with JPEG compression at a given quality level, you may use:

    image CompressedImage = jpeg_preview(RawImage, <quality>);

The image will have an overlay with the resulting size in megabytes. To disable it, add an additional argument `no_overlay` (without quotation marks).
