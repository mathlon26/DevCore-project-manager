#!/bin/bash
set -e

echo "Cloning the DevCore repository..."
git clone https://github.com/mathlon26/DevCore-project-manager.git

cd DevCore-project-manager

echo "Building DevCore (running run.sh)..."
chmod +x run.sh
./run.sh

# Verify that the devcore binary was built
if [ ! -f devcore ]; then
    echo "Error: devcore binary not found. Build may have failed."
    exit 1
fi

# Move the devcore binary to a directory in the user's PATH (using $HOME/.local/bin)
echo "Moving the devcore binary to \$HOME/.local/bin..."
mkdir -p "$HOME/.local/bin"
mv devcore "$HOME/.local/bin/"
echo "Done. Make sure \$HOME/.local/bin is in your PATH."

# Create necessary config directories
echo "Setting up configuration directories..."
mkdir -p "$HOME/.config/devcore/templates"
mkdir -p "$HOME/.config/devcore"

# Copy configuration files from the repository
echo "Copying configuration files to \$HOME/.config/devcore..."
if [ -f devcore.conf ]; then
    cp devcore.conf "$HOME/.config/devcore/devcore.conf"
else
    echo "Warning: devcore.conf not found in repository."
fi

if [ -f devmap.json ]; then
    cp devmap.json "$HOME/.config/devcore/devmap.json"
else
    echo "Warning: devmap.json not found in repository."
fi

# Ask the user if they want preinstalled templates
read -p "Do you want to install preinstalled templates? (y/n): " install_templates
if [[ "$install_templates" =~ ^[Yy] ]]; then
    echo "Installing preinstalled templates..."
    # Define languages to install
    for lang in "C++" "C" "Python" "Java"; do
        echo "Creating language: $lang"
        "$HOME/.local/bin/devcore" create-lang "$lang"
        # Check if a templates directory exists for this language in the repository.
        if [ -d "templates/$lang" ]; then
            echo "Copying templates for $lang..."
            mkdir -p "$HOME/.config/devcore/templates/$lang"
            cp -r "templates/$lang/"* "$HOME/.config/devcore/templates/$lang/"
        else
            echo "No templates found for $lang in repository."
        fi
    done
else
    echo "Skipping preinstalled templates."
fi

# Remove the cloned repository
cd ..
rm -rf DevCore-project-manager

echo "Installation complete!"
