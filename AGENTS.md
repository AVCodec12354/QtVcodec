# QtVCodec AI Agent Guide

## Architecture Overview
QtVCodec uses a component-based architecture for video encoding/decoding:
- **Qv2Component**: Base class with state machine (UNINITIALIZED→CONFIGURED→RUNNING→STOPPED)
- **Qv2Work**: Work items containing input/output buffers and metadata
- **Qv2Buffer**: Container for Qv2Block1D (linear) or Qv2Block2D (graphic) data
- **Qv2ComponentFactory**: Creates APV encoders/decoders by type/name

## Build & Development Workflow
```bash
# Clean rebuild (use this for dependency changes)
./build.sh

# Run component tests
cd outputs/tests && ./Qv2ComponentTests

# Debug crashes
lldb ./build/QtVCodec
```

## Key Patterns & Conventions

### Component Implementation
- Inherit from Qv2Component, implement pure virtual methods (configure, start, queue, etc.)
- Use state machine: configure() → start() → queue(work) → stop() → release()
- Set listener for async callbacks (onWorkDone, onError, onStateChanged)

### Buffer Handling
- **Graphic buffers**: Qv2Block2D for YUV frames, use setPlane() for each plane
- **Linear buffers**: Qv2Block1D for encoded bitstreams
- Convert QV2_CF_* constants to OAPV_CF_* using toOapvFmt() helper

### Testing
- Use Google Test with parameterized fixtures (Qv2EncoderTestP)
- Test data in YUVTests/ and Y4MTests/ subfolders
- Follow naming: `{test_name}_{format}_{resolution}_{fps}_{frames}.{ext}`
- Use TestListener for async result verification

### File Organization
- `inc/Qv2Core/`: Core abstractions (Component, Buffer, Work)
- `codecs/apv/`: APV-specific implementations
- `ui/viewmodels/`: Qt ViewModels for UI binding
- `renderers/`: OpenGL video rendering widgets

## Common Integration Points
- **OpenAPV**: Fetched via CMake FetchContent, headers copied to inc/
- **Qt6**: Widgets + OpenGLWidgets for GUI and rendering
- **GTest**: For unit testing, linked in codecs/tests/

## Debugging Tips
- Enable LOG_DEBUG in test files for verbose output
- Use QV2_LOG* macros for consistent logging
- Check component state with getState() before operations
- PSNR measurement available via measure_psnr() for quality validation</content>
<parameter name="filePath">/Users/khanh12354/WorkPlace/QtVcodec/AGENTS.md
