#!/bin/bash

# Define the target directory
CONFIG_DIR="$HOME/.config/devcore"

# Function to prompt for overwrite
prompt_overwrite() {
    local file=$1
    if [ -e "$file" ]; then
        read -p "$file already exists. Overwrite? (y/n): " choice
        case "$choice" in 
            y|Y ) rm -f "$file" ;;
            * ) return 1 ;; # Skip overwrite
        esac
    fi
    return 0
}

# Create the devcore directory if it doesn't exist
mkdir -p "$CONFIG_DIR/templates"

# Check and prompt for overwriting devcore.conf
if prompt_overwrite "$CONFIG_DIR/devcore.conf"; then
    touch "$CONFIG_DIR/devcore.conf"
fi

# Check and prompt for overwriting devmap.json
if prompt_overwrite "$CONFIG_DIR/devmap.json"; then
    touch "$CONFIG_DIR/devmap.json"
fi

# Print success message
echo "Devcore environment set up successfully in $CONFIG_DIR"
