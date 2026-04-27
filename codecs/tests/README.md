# QtVCodec Component Testing Guide

This directory contains the automated test suite for the **QtVCodec** framework. It validates the state machine, parameter configuration, and encoding quality of the OpenAPV components.

## Prerequisites
- **FFmpeg/ffplay**: Ensure you have a recent version of FFmpeg installed (v7.0+ recommended) that supports the APV codec.
- **C++17 Compiler**: For building the test binaries.

## Step-by-Step Verification

Follow these steps in order to build the project, run the tests, and visually verify the output.

### Step 1: Build the Project
Run the main build script from the root directory to compile the core library and the test executable.
```bash
./build.sh
```

### Step 2: Data Preparation (Optional)
If you do not have raw YUV or Y4M files for testing, you can generate them from the provided reference bitstreams inside the `testWorkSpace` directory.
```bash
cd codecs/tests/testWorkSpace
./genEncoderInputs.sh
```
This script will decode reference `.apv` files and place the resulting `.yuv` and `.y4m` files into `YUVTests/input/` and `Y4MTests/input/`.

### Step 3: Run Automated Tests
Execute the test runner script. This script will:
1. Clean old test outputs in `Y4MTests/output` and `YUVTests/output`.
2. Create necessary directories if they don't exist.
3. Run the Google Test suite (`Qv2ComponentTests`).
```bash
./run.sh
```

### Step 4: Verify Output Quality
After the tests finish, use the interactive player script to view the generated `.apv` bitstreams.
```bash
./playOutput.sh
```

**Interactive Controls during playback:**
- **[SPACE] or [P]**: Pause/Resume (useful for inspecting specific frames).
- **[S]**: Step forward frame-by-frame.
- **[Q]**: Exit current video.
- **Terminal Prompt**: After each video, the script will ask if you want to **Continue**, **Replay (r)**, or **Quit (q)**.

---

## Test Architecture

### 1. `Qv2ComponentBaseTests.cpp`
Validates the internal logic of the component:
- **State Machine**: Ensures correct transitions (INITIALIZED -> CONFIGURED -> RUNNING -> STOPPED).
- **Factory**: Verifies component creation by type or name.
- **HDR/Color Metadata**: Validates that complex parameters like HDR Static Metadata and Color Aspects are accepted correctly.

### 2. `Qv2EncoderTests.cpp`
Performs end-to-end encoding tests:
- **YUV Encoding**: Processes raw YUV files and measures PSNR (Peak Signal-to-Noise Ratio).
- **Y4M Encoding**: Parses Y4M headers automatically and encodes the content.
- **Bitstream Integrity**: Saves the encoded output to the `output/` folders for visual inspection.

### 3. `TestUtils.h`
Contains shared helper functions, the `TestListener` class for handling asynchronous callbacks, and data structures for parameterized testing.

---

## Important Folders
- `YUVTests/input/`: Place raw YUV files here for testing.
- `Y4MTests/input/`: Place Y4M files here for testing.
- `*/output/`: Generated `.apv` files will be stored here.
