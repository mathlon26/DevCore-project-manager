#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Canvas.hpp"
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
    "test"
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
inline void updateFile() {
    if (configFilename.empty()) {
        Canvas::PrintErrorExit("No configuration file loaded to update.");
    }
    std::ofstream file(configFilename);
    if (!file.is_open()) {
        Canvas::PrintErrorExit("Unable to open configuration file for writing: " + configFilename);
    }
    for (const auto &pair : configMap) {
        file << pair.first << " = " << pair.second << "\n";
    }
    file.close();
}

// Load configuration from a .conf file with lines in the format "key = value".
// Lines starting with '#' or empty lines are ignored.
inline bool load(const std::string &filename, bool install=false) {
    if (install) {
        fs::path configPath = filename;
        fs::path configDir = configPath.parent_path();

        if (!fs::exists(configDir)) {
            fs::create_directories(configDir);
        }

        if (!fs::exists(configPath)) {
            std::string cloneCommand = "git clone " + github + " /tmp/devcore_repo";
            if (std::system(cloneCommand.c_str()) != 0) {
                Canvas::PrintErrorExit("Failed to clone repository from " + github);
            }
            
            fs::path sourceConfig = "/tmp/devcore_repo/devcore.conf";
            if (!fs::exists(sourceConfig)) {
                Canvas::PrintErrorExit("Default configuration file not found in cloned repository.");
            }
            
            fs::copy_file(sourceConfig, configPath, fs::copy_options::overwrite_existing);
            fs::remove_all("/tmp/devcore_repo");
        }
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

// Retrieve a configuration value by key.
// If the key is not among the validKeys, print an error and exit.
inline std::string get(const std::string &key) {
    if (std::find(validKeys.begin(), validKeys.end(), key) == validKeys.end()) {
        Canvas::PrintErrorExit("Invalid key '" + key + "' should not even be in the configuration. Why are you looking for it?");
    }
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        return it->second;
    }
    Canvas::PrintErrorExit("Unable to retrieve key '" + key + "' from the configuration.");
    return ""; // Unreachable, but added to satisfy the return type.
}

// Set a configuration value by key and update the configuration file.
// If the key is not among the validKeys, print an error and exit.
inline void set(const std::string &key, const std::string &value) {
    if (std::find(validKeys.begin(), validKeys.end(), key) == validKeys.end()) {
         Canvas::PrintErrorExit("Key '" + key + "' is not a valid configuration key.");
    }
    configMap[key] = value;
    updateFile();
}

// Returns a string with all key-value pairs in the following format:
// "key1: value1\nkey2: value2\n..."
inline std::string GetKeyValueString() {
    std::ostringstream oss;
    for (const auto &pair : configMap) {
        oss << pair.first << ": " << pair.second << "\n";
    }
    return oss.str();
}

} // namespace Config

#endif // CONFIG_HPP
