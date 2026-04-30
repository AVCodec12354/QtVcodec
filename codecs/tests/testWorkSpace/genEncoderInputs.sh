#!/bin/bash

# Move to script directory first to ensure relative paths are correct
cd "$(dirname "$0")" || exit 1

# Configuration
DECODER="./oapv_app_dec"
BITSTREAM_DIR="./apvBitstreams"
Y4M_OUTPUT_DIR="./Y4MTests/input"
YUV_OUTPUT_DIR="./YUVTests/input"

# Create output directories if they don't exist
mkdir -p "$Y4M_OUTPUT_DIR"
mkdir -p "$YUV_OUTPUT_DIR"

# Check if decoder exists
if [ ! -f "$DECODER" ]; then
    echo "Error: $DECODER not found in current directory."
    exit 1
fi

# Ensure decoder is executable
chmod +x "$DECODER"

# Bitstream Information
# Format: name|colorFormat|resolution|fps|numFrames
BITSTREAMS=(
    "tile_A|yuv422p10le|3840x2160|60|3"
    "tile_B|yuv422p10le|3840x2160|60|3"
    "tile_C|yuv422p10le|7680x4320|30|3"
    "tile_D|yuv422p10le|3840x2160|60|3"
    "tile_E|yuv422p10le|3840x2160|60|3"
    "qp_A|yuv422p10le|3840x2160|60|3"
    "qp_B|yuv422p10le|3840x2160|60|3"
    "qp_C|yuv422p10le|3840x2160|60|3"
    "qp_D|yuv422p10le|3840x2160|60|3"
    "qp_E|yuv422p10le|3840x2160|60|3"
    "syn_A|yuv422p10le|1920x1080|60|2"
    "syn_B|yuv422p10le|1920x1080|60|2"
)

echo "Starting APV decoding to Y4M and YUV..."

for info in "${BITSTREAMS[@]}"; do
    IFS='|' read -r name cfmt res fps frames <<< "$info"

    input_file="${BITSTREAM_DIR}/${name}.apv"

    if [ -f "$input_file" ]; then
        # 1. Decode to Y4M (extension .y4m triggers Y4M output)
        y4m_file="${Y4M_OUTPUT_DIR}/${name}_${cfmt}_${res}_${fps}fps_${frames}.y4m"
        echo "Decoding Y4M: $input_file -> $y4m_file"
        $DECODER -i "$input_file" -o "$y4m_file"

        # 2. Decode to Raw YUV (extension .yuv triggers raw YUV output)
        yuv_file="${YUV_OUTPUT_DIR}/${name}_${cfmt}_${res}_${fps}fps_${frames}.yuv"
        echo "Decoding YUV: $input_file -> $yuv_file"
        $DECODER -i "$input_file" -o "$yuv_file"

        if [ $? -eq 0 ]; then
            echo "Successfully processed $name"
        else
            echo "Failed to process $name"
        fi
    else
        echo "Warning: Input file $input_file not found. Skipping."
    fi
    echo "--------------------------------------"
done

echo "Processing complete."
echo "Y4M files are in: $Y4M_OUTPUT_DIR"
echo "YUV files are in: $YUV_OUTPUT_DIR"
