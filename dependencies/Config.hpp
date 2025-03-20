#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Canvas.hpp"
#include <set>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <vector>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace Config {

// Inline variable for storing configuration key-value pairs (ordered map).
inline std::map<std::string, std::string> configMap;
// Inline variable to store the name of the loaded configuration file.
inline std::string configFilename;

inline std::string github = "https://github.com/mathlon26/DevCore-project-manager.git";

const std::vector<std::string> validKeys{
    "projects_path",
    "editor"
};

// Utility function to trim whitespace from both ends of a string.
inline std::string trim(const std::string &s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        ++start;
    }
    auto end = s.end();
    do {
        --end;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

// Write the current configuration map back to the config file.
// Write the current configuration map back to the config file without removing comments.
inline void updateFile() {
    if (configFilename.empty()) {
        Canvas::PrintErrorExit("No configuration file loaded to update.");
    }
    
    // Read all existing lines from the file.
    std::vector<std::string> lines;
    {
        std::ifstream infile(configFilename);
        if (infile.is_open()) {
            std::string line;
            while (std::getline(infile, line)) {
                lines.push_back(line);
            }
        }
    }
    
    // Keep track of which keys have been updated in the file.
    std::set<std::string> keysUpdated;
    
    // Process each line.
    for (auto &line : lines) {
        std::string trimmedLine = trim(line);
        // If the line is empty or a full-line comment, skip updating it.
        if (trimmedLine.empty() || trimmedLine[0] == '#') {
            continue;
        }
        
        // Find the start of any inline comment to preserve it.
        size_t posComment = line.find('#');
        std::string assignmentPart = (posComment != std::string::npos) ? line.substr(0, posComment) : line;
        std::string trailingComment = (posComment != std::string::npos) ? line.substr(posComment) : "";
        
        // Look for the '=' delimiter in the assignment part.
        size_t posEq = assignmentPart.find('=');
        if (posEq == std::string::npos) {
            continue;
        }
        
        // Extract and trim the key.
        std::string key = trim(assignmentPart.substr(0, posEq));
        
        // Only update the line if the key is valid.
        if (std::find(validKeys.begin(), validKeys.end(), key) != validKeys.end()) {
            if (configMap.find(key) != configMap.end()) {
                std::string newValue = configMap[key];
                std::ostringstream oss;
                oss << key << " = " << newValue;
                // Rebuild the line while preserving any trailing inline comment.
                line = oss.str() + trailingComment;
                keysUpdated.insert(key);
            }
        }
    }
    
    // For any keys in the configMap that weren't found in the file, append them.
    for (const auto &pair : configMap) {
        if (keysUpdated.find(pair.first) == keysUpdated.end()) {
            std::ostringstream oss;
            oss << pair.first << " = " << pair.second;
            lines.push_back(oss.str());
        }
    }
    
    // Write the updated lines back to the configuration file.
    std::ofstream outfile(configFilename);
    if (!outfile.is_open()) {
        Canvas::PrintErrorExit("Unable to open configuration file for writing: " + configFilename);
    }
    for (const auto &l : lines) {
        outfile << l << "\n";
    }
    outfile.close();
}


// Load configuration from a .conf file with lines in the format "key = value".
// Lines starting with '#' or empty lines are ignored.
inline bool load(const std::string &filename, bool install=false) {
    if (install) {
        Canvas::PrintInfo("Checking for required directories");
        fs::path configPath = filename;
        fs::path configDir = configPath.parent_path();
    
        if (!fs::exists(configDir)) {
            fs::create_directories(configDir);
        }
    
        // Always clone and overwrite the configuration file
        Canvas::PrintInfo("Cloning the DevCore repository to retrieve the default config.");
        std::string cloneCommand = "git clone " + github + " /tmp/devcore_repo";
        if (std::system(cloneCommand.c_str()) != 0) {
            Canvas::PrintErrorExit("Failed to clone repository from " + Canvas::LinkText(github));
        }
        
        fs::path sourceConfig = "/tmp/devcore_repo/devcore.conf";
        if (!fs::exists(sourceConfig)) {
            Canvas::PrintErrorExit("Default configuration file not found in the cloned repository.");
        }
        Canvas::PrintInfo("Copying the new config to '" + Canvas::LinkText(filename, Canvas::Color::CYAN) + "'");
        // Overwrite the configuration file regardless of its existence.
        fs::copy_file(sourceConfig, configPath, fs::copy_options::overwrite_existing);
        Canvas::PrintInfo("Removing the temporary cloned repository.");
        fs::remove_all("/tmp/devcore_repo");
        Canvas::PrintSuccess(Canvas::BoldText("Done installing the default config.") + Canvas::ColorToAnsi(Canvas::Color::GREEN) + "\n    You can edit the config by running `devcore config set <key> <value>`. \n    Or editing the config file manually at '" + Canvas::LinkText(filename, Canvas::Color::GREEN) + "'");
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Save the filename for future updates.
    configFilename = filename;
    
    std::string line;
    while (std::getline(file, line)) {
        // Remove comments (anything after '#' is considered a comment).
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        line = trim(line);
        if (line.empty())
            continue;
        
        // Find the '=' delimiter.
        size_t delimPos = line.find('=');
        if (delimPos == std::string::npos)
            continue; // Skip lines without '='.
        
        std::string key = trim(line.substr(0, delimPos));
        std::string value = trim(line.substr(delimPos + 1));
        configMap[key] = value;
    }
    file.close();

    return true;
}

inline int setup(const std::string &filename)
{
    Canvas::ClearConsole();
    Canvas::PrintTitle("DevCore | Setup Zone");
    Canvas::PrintWarning("It seems like you do not yet have a config file. Would you like to install a default config? \n    If not check out '" + Canvas::LinkText(filename, Canvas::Color::YELLOW) + "' to configure one manually.");
    if(Canvas::GetBoolInput("    ")) {
        load(filename, true);
        return 0;
    }
    return 0;
}

void validate()
{
    if (configMap.empty())
    {
        setup(configFilename);
        exit(0);
    }
}

// Retrieve a configuration value by key.
// If the key is not among the validKeys, print an error and exit.
inline std::string get(const std::string &key) {
    validate();
    if (std::find(validKeys.begin(), validKeys.end(), key) == validKeys.end()) {        
        Canvas::PrintErrorExit("Invalid key '" + key + "' should not even be in the configuration. Why are you looking for it?");
    }
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        return it->second;
    }
    Canvas::PrintErrorExit("Required key '" + key + "' is not found in the configuration. Add '" + key + " = <" + key + ">' to your config file manually or by using 'devcore set " + key + " <" + key + ">'");
    return ""; // Unreachable, but added to satisfy the return type.
}

// Set a configuration value by key and update the configuration file.
// If the key is not among the validKeys, print an error and exit.
inline void set(const std::string &key, const std::string &value) {
    validate();
    if (std::find(validKeys.begin(), validKeys.end(), key) == validKeys.end()) {
         Canvas::PrintErrorExit("Key '" + key + "' is not a valid configuration key.");
    }
    configMap[key] = value;
    updateFile();
}

// Returns a string with all key-value pairs in the following format:
// "key1: value1\nkey2: value2\n..."
inline std::string GetKeyValueString() {
    validate();
    std::ostringstream oss;
    if (configMap.empty())
        return "";
    for (const auto &pair : configMap) {
        oss << pair.first << ": " << pair.second << "\n";
    }
    return oss.str();
}

} // namespace Config

#endif // CONFIG_HPP
