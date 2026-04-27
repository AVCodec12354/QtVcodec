#!/bin/bash

# Configuration
TEST_DIR="outputs/tests"
TARGET_FOLDERS=("Y4MTests/output" "YUVTests/output")

# ANSI Color Codes
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Check if we are in the right directory or need to move
if [ -d "$TEST_DIR" ]; then
    cd "$TEST_DIR" || exit 1
fi

echo -e "${CYAN}==========================================${NC}"
echo -e "${CYAN}   QtVCodec APV Bitstream Player          ${NC}"
echo -e "${CYAN}==========================================${NC}"
echo -e "${YELLOW}Controls inside player:${NC}"
echo -e "  [SPACE/P] - Pause / Resume"
echo -e "  [S]       - Step forward one frame"
echo -e "  [Q/ESC]   - Quit current file and continue"
echo -e "------------------------------------------"

# Check for ffplay
if ! command -v ffplay &> /dev/null; then
    echo "Error: ffplay is not installed or not in PATH."
    exit 1
fi

# Loop through output folders
for FOLDER in "${TARGET_FOLDERS[@]}"; do
    if [ -d "$FOLDER" ]; then
        echo -e "\nSearching for APV bitstreams in: ${CYAN}$FOLDER${NC}"

        # Find all .apv files
        FILES=$(find "$FOLDER" -name "*.apv" | sort)

        if [ -z "$FILES" ]; then
            echo "  -> No .apv files found."
            continue
        fi

        for FILE in $FILES; do
            # Get file size to check if it's empty
            SIZE=$(wc -c < "$FILE")
            if [ "$SIZE" -le 0 ]; then
                echo "  -> Skipping empty file: $FILE"
                continue
            fi

            while true; do
                echo -e "------------------------------------------"
                echo -e "Playing: ${GREEN}$FILE${NC}"
                echo -e "Format:  ${CYAN}Raw APV Bitstream${NC}"

                ffplay -f apv -loop 33 -autoexit "$FILE"

                echo -e "\n${YELLOW}Playback of current file finished.${NC}"
                echo -n "Action: [Enter] Next file | [r] Replay this file | [q] Quit script: "
                read -r choice

                case "$choice" in
                    [rR] ) continue ;;
                    [qQ] ) echo -e "${CYAN}Exiting...${NC}"; exit 0 ;;
                    * ) break ;; # Continue to next file
                esac
            done
        done
    else
        echo "Directory not found: $FOLDER"
    fi
done

echo -e "\n${CYAN}==========================================${NC}"
echo -e "${CYAN}   Playback sequence completed.           ${NC}"
echo -e "${CYAN}==========================================${NC}"
