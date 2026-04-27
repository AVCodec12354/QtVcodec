#!/bin/bash

clear

# Path to tests directory
TEST_DIR="."
cd "$TEST_DIR" || { echo "Error: Directory $TEST_DIR not found"; exit 1; }

# List of target folders to process
TARGET_FOLDERS=("Y4MTests/output" "YUVTests/output")

for FOLDER in "${TARGET_FOLDERS[@]}"; do
    if [ -d "$FOLDER" ]; then
        # If directory exists, delete all contents inside
        echo "Cleaning directory: $FOLDER"
        rm -rf "${FOLDER:?}"/*
    else
        # If directory does not exist, create it (including parents if needed)
        echo "Creating new directory: $FOLDER"
        mkdir -p "$FOLDER"
    fi
done

echo "------------------------------------------"
echo "Starting Qv2ComponentTests..."
echo "------------------------------------------"

# Execute test binary
./Qv2ComponentTests
