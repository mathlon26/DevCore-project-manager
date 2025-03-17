#ifndef PROJECT_MANAGER__H
#define PROJECT_MANAGER__H

#include "../dependencies/CommandManager.hpp"
#include "Project.h"
#include <iostream>
#include <fstream>
#include <cstdlib> 
#include <nlohmann/json.hpp>

class ProjectManager
{
public:
    ProjectManager();
    ~ProjectManager() {};

    std::string GetIndexDir();

    std::vector<Project*> Projects;
    std::vector<std::string> Languages;

    std::vector<std::string> GetUsers()
    {
        std::vector<std::string> users{};
        for (Project* project : Projects)
            users.push_back(project->GetCreatedBy());
        
        return users;
    }

private:
    std::string m_indexPath;
};

using json = nlohmann::json;

ProjectManager::ProjectManager()
{
    // Get home directory from environment variable
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        CommandManager::Error("Home directory could not be retrieved by getenv() from <cstdlib>.");
    }
    m_indexPath = std::string(homeDir) + "/.config/custom/pm/pm.index.json";

    // Open the JSON file
    std::ifstream file(m_indexPath);
    if (!file.is_open()) {
        CommandManager::Error("Could not open file: " + m_indexPath);
    }

    // Parse the JSON file
    json j;
    file >> j;

    // Populate Languages vector
    if(j.contains("languages") && j["languages"].is_array()) {
        for (const auto& lang : j["languages"]) {
            Languages.push_back(lang.get<std::string>());
        }
    } else {
        CommandManager::Error("The JSON file does not contain a valid 'languages' array.");
    }

    // Populate Projects vector
    if(j.contains("projects") && j["projects"].is_array()) {
        for (const auto& proj : j["projects"]) {
            // Assuming Project has a constructor like:
            // Project(int bytes, const std::string &created_at, const std::string &created_by,
            //         const std::string &lang, const std::string &location, const std::string &name);
            Project* newProject = new Project(
                proj["bytes"].get<int>(),
                proj["created_at"].get<std::string>(),
                proj["created_by"].get<std::string>(),
                proj["lang"].get<std::string>(),
                proj["location"].get<std::string>(),
                proj["name"].get<std::string>()
            );
            Projects.push_back(newProject);
        }
    } else {
        CommandManager::Error("The JSON file does not contain a valid 'projects' array.");
    }
}

std::string ProjectManager::GetIndexDir()
{
    return m_indexPath;
}

#endif // PROJECT_MANAGER__H