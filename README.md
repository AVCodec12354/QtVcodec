# QtVCodec

A Qt-based video codec application specializing in OpenAPV (Advanced Professional Video) support.

## Features
- Video encoding and decoding using OpenAPV.
- High-performance video rendering with OpenGL.
- User-friendly GUI built with Qt6.
- Real-time logging system.

## Prerequisites
- **CMake** (3.16 or higher)
- **Qt6** (Widgets and OpenGLWidgets components)
- **C++17** compatible compiler
- **Git** (for fetching dependencies)

## Build & Run

You can use the provided `build.sh` script or follow these manual steps:

### Using script
```bash
chmod +x build.sh
./build.sh
```

### Manual build
```bash
mkdir build
cd build
cmake ..
cmake --build .
./QtVCodec
```

## Project Structure
- `codecs/`: Implementation of video codecs (APV).
- `renderers/`: OpenGL-based video rendering widgets.
- `ui/`: GUI components including View, ViewModels, and helpers.
- `src/`: Main entry point of the application.
- `inc/`: Header files for dependencies (e.g., OpenAPV).

## Dependencies
- [OpenAPV](https://github.com/AcademySoftwareFoundation/openapv) - Automatically managed via CMake's `FetchContent`.
