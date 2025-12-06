# Simple 3D Rendering Engine in C

A lightweight, software-based 3D rendering engine written in C. This project demonstrates 3D graphics fundamentals by rendering a rotating 3D model (Utah Teapot) without using external graphics libraries like OpenGL or Vulkan for the rendering pipeline. It uses X11 for window management and PulseAudio for sound.

## Features

- **Software Rasterization**: Implements 3D projection, rotation, and triangle rasterization from scratch.
- **Z-Buffering**: Handles depth visibility to correctly render overlapping faces.
- **Lighting**: Basic lighting implementation using face normals.
- **Audio**: Background music playback using PulseAudio and `stb_vorbis`.
- **No Heavy Dependencies**: Built with standard C libraries and minimal system libraries (X11, PulseAudio).

## Dependencies

To build and run this project, you need the following development libraries installed on your Linux system:

- `libX11-dev`
- `libXrandr-dev`
- `libpulse-dev`
- `ffmpeg` (Optional, for converting audio files)

On Debian/Ubuntu based systems, you can install them via:

```bash
sudo apt install libx11-dev libxrandr-dev libpulse-dev ffmpeg
```

## Build and Run

1.  **Build the project**:
    Run the included build script:
    ```bash
    ./build.sh
    ```

2.  **Run the application**:
    ```bash
    ./main
    ```

## Controls

- **`q`**: Quit the application.

## Audio Assets

The project uses OGG format for audio. If you have an MP3 file, you can convert it using `ffmpeg`:

```bash
ffmpeg -i input.mp3 -c:a libvorbis output.ogg
```

**NOTE**: The included `stb_vorbis` library only supports OGG format.

### References
- [stb_vorbis](https://github.com/nothings/stb/blob/master/stb_vorbis.c)
- [Rizz Sound Effect](https://www.myinstants.com/en/instant/rizz-sound-effect-54189/)
