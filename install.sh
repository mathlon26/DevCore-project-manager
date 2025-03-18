#!/bin/bash
set -e

# ANSI Colors
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
CYAN="\033[36m"
WHITE="\033[37m"
RESET="\033[0m"

export HOME="$HOME_TEMP"

# Banner
echo -e "${CYAN}==========================================${RESET}"
echo -e "${BLUE} DevCore Installer - Automated Setup ${RESET}"
echo -e "${CYAN}==========================================${RESET}"

echo -e "${YELLOW}MAKE SURE YOU HAVE THE FOLLOWING DIRECTORY: ${WHITE}~/Coding/Projects${RESET}"
read -p "Did you create this directory? (y/n): " continueInstall
if [[ "$continueInstall" =~ ^[Yy] ]]; then

    echo -e "${CYAN}------------------------------------------${RESET}"
    echo -e "${GREEN}🚀 Cleaning up old installations...${RESET}"
    
    # Remove any existing temporary repository
    if [ -d "DevCore-project-manager-temp" ]; then
        echo -e "${YELLOW}🧹 Removing existing temporary repository...${RESET}"
        rm -rf DevCore-project-manager-temp
    fi

    echo -e "${CYAN}------------------------------------------${RESET}"
    echo -e "${GREEN}📥 Cloning the DevCore repository...${RESET}"
    git clone https://github.com/mathlon26/DevCore-project-manager.git DevCore-project-manager-temp

    cd DevCore-project-manager-temp

    # Check and remove previous clone if exists
    if [ -d "DevCore-project-manager" ]; then
        echo -e "${YELLOW}🗑️  Removing existing DevCore directory...${RESET}"
        rm -rf DevCore-project-manager
    fi

    echo -e "${GREEN}🔄 Cloning DevCore repository again...${RESET}"
    git clone https://github.com/mathlon26/DevCore-project-manager.git

    cd DevCore-project-manager

    echo -e "${CYAN}------------------------------------------${RESET}"
    echo -e "${GREEN}⚙️  Building DevCore (running run.sh)...${RESET}"
    chmod +x run.sh
    ./run.sh

    # Verify that devcore binary was built
    if [ ! -f devcore ]; then
        echo -e "${RED}❌ Error: devcore binary not found. Build may have failed.${RESET}"
        exit 1
    fi

    echo -e "${GREEN}📂 Moving the devcore binary to ${WHITE}$HOME/.local/bin${RESET}"
    mkdir -p "$HOME/.local/bin"
    mv devcore "$HOME/.local/bin/"
    echo -e "${YELLOW}✅ Done. Make sure $HOME/.local/bin is in your PATH.${RESET}"

    echo -e "${CYAN}------------------------------------------${RESET}"
    echo -e "${GREEN}🔧 Setting up configuration directories...${RESET}"
    mkdir -p "$HOME/.config/devcore/templates"
    mkdir -p "$HOME/.config/devcore"

    echo -e "${GREEN}📋 Copying configuration files...${RESET}"
    [ -f devcore.conf ] && cp devcore.conf "$HOME/.config/devcore/devcore.conf" || echo -e "${YELLOW}⚠️  Warning: devcore.conf not found.${RESET}"
    [ -f devmap.json ] && cp devmap.json "$HOME/.config/devcore/devmap.json" || echo -e "${YELLOW}⚠️  Warning: devmap.json not found.${RESET}"

    echo -e "${CYAN}------------------------------------------${RESET}"
    echo -e "${GREEN}📂 Ensuring the projects directory exists...${RESET}"
    mkdir -p "$HOME/Coding/Projects"

    read -p "Do you want to install preinstalled templates? (y/n): " install_templates
    if [[ "$install_templates" =~ ^[Yy] ]]; then
        echo -e "${GREEN}📦 Installing preinstalled templates...${RESET}"
        for lang in "C++" "C" "Python" "Java"; do
            echo -e "${BLUE}🌟 Creating language: $lang${RESET}"
            env HOME="$HOME_TEMP" "$HOME/.local/bin/devcore" create-lang "$lang"
            if [ -d "templates/$lang" ]; then
                echo -e "${GREEN}📂 Copying templates for $lang...${RESET}"
                mkdir -p "$HOME_TEMP/.config/devcore/templates/$lang"
                for template_dir in "templates/$lang"/*; do
                    if [ -d "$template_dir" ]; then
                        pwd
                        echo "$template_dir" "$HOME_TEMP/.config/devcore/templates/$lang/"
                        cp -r "$template_dir" "$HOME_TEMP/.config/devcore/templates/$lang/"
                    fi
                done
            else
                echo -e "${YELLOW}⚠️  No templates found for $lang.${RESET}"
            fi
        done
    else
        echo -e "${YELLOW}🚀 Skipping preinstalled templates.${RESET}"
    fi

    echo -e "${CYAN}------------------------------------------${RESET}"
    echo -e "${RED}🧹 Cleaning up installation files...${RESET}"
    cd ../..
    rm -rf DevCore-project-manager-temp

    echo -e "${CYAN}------------------------------------------${RESET}"
    echo -e "${GREEN}📌 Replacing the devcore binary...${RESET}"

    # Define paths
    NEW_BINARY="$HOME/.local/bin/devcore"
    TEMP_BINARY="/tmp/devcore.tmp"
    FINAL_BINARY="/usr/bin/devcore"

    # Move the new binary to a temporary location first
    sudo mv "$NEW_BINARY" "$TEMP_BINARY"

    # Create a script to update the binary after the process exits
    UPDATE_SCRIPT="/tmp/update_devcore.sh"
    echo "#!/bin/bash" | sudo tee $UPDATE_SCRIPT > /dev/null
    echo "sleep 2" | sudo tee -a $UPDATE_SCRIPT > /dev/null  # Ensure the process exits
    echo "sudo mv $TEMP_BINARY $FINAL_BINARY" | sudo tee -a $UPDATE_SCRIPT > /dev/null
    echo "sudo rm -- \"$0\"" | sudo tee -a $UPDATE_SCRIPT > /dev/null  # Delete script after execution

    # Make the update script executable
    sudo chmod +x $UPDATE_SCRIPT

    # Schedule the update script to run in the background
    nohup sudo bash $UPDATE_SCRIPT >/dev/null 2>&1 &

    echo -e "${CYAN}==========================================${RESET}"
    echo -e "${GREEN}🎉 Installation complete! 🎉${RESET}"
    echo -e "${YELLOW}🔄 Restart your terminal first!${RESET}"
    echo -e "${YELLOW}🔗 Then ensure that $HOME/.local/bin is in your PATH.${RESET}"
    echo -e "${YELLOW}🧹 If not, rerun the devcore install.${RESET}"
    echo -e "${CYAN}==========================================${RESET}"

else
    echo -e "${RED}❌ Exiting install. Please create ~/Coding/Projects before running this!"
fi 
