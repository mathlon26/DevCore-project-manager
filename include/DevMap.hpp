#ifndef DEVMAP_HPP
#define DEVMAP_HPP

#include "../dependencies/Canvas.hpp"
#include "../dependencies/Config.hpp"
#include "Main.hpp"
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <set>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <nlohmann/json.hpp>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif
namespace fs = std::filesystem;

namespace DevMap
{
    // Project structure holding project metadata.
    struct Project
    {
        std::string name;       // Virtual name for the manager.
        std::string folderName; // Actual folder name of the project.
        std::string lang;       // Language (also used as directory name).
        std::string createdBy;  // User who created the project.
        time_t createdAt;       // Creation time.
        size_t size;            // Project size in bytes.
        bool usesGit;           // Wether there is a .git folder in the projects
    };

    // Global inline variables to store the DevMap state.
    inline fs::path projectsPath;
    inline fs::path devmapFileName;
    inline nlohmann::json devmapData;
    inline std::vector<std::string> languages;
    inline std::set<std::string> users;
    inline std::vector<Project> projects;


    std::string getCurrentUser() {
    #ifdef _WIN32
        const char* user = std::getenv("USERNAME");
    #else
        const char* user = std::getenv("USER");
        // Fallback in case getenv doesn't return the username.
        if (!user) {
            struct passwd* pw = getpwuid(getuid());
            if (pw)
                user = pw->pw_name;
        }
    #endif
        return user ? std::string(user) : "unknown";
    }

    // Helper: Convert a time string ("HH:MM DD-MM-YYYY") to a time_t value.
    inline time_t parseTime(const std::string &timeStr)
    {
        std::tm tm = {};
        std::istringstream ss(timeStr);
        ss >> std::get_time(&tm, "%H:%M %d-%m-%Y");
        if (ss.fail())
        {
            return std::time(nullptr); // Fallback to current time.
        }
        return std::mktime(&tm);
    }

    // Helper: Convert a time_t value back to a formatted string.
    inline std::string timeToString(time_t t)
    {
        std::tm *tmPtr = std::localtime(&t);
        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%H:%M %d-%m-%Y", tmPtr);
        return std::string(buffer);
    }

    inline bool usesGit(const std::string &projectfolder)
    {
        fs::path gitPath = fs::path(projectfolder) / ".git";
        return fs::exists(gitPath) && fs::is_directory(gitPath);
    }

    inline size_t getFolderSize(const std::string &projectfolder)
    {
        size_t totalSize = 0;
        fs::path folderPath(projectfolder);
        if (fs::exists(folderPath) && fs::is_directory(folderPath))
        {
            for (const auto &entry : fs::recursive_directory_iterator(folderPath))
            {
                if (fs::is_regular_file(entry.status()))
                {
                    totalSize += fs::file_size(entry);
                }
            }
        }
        return totalSize;
    }

    inline void CreateProject(const Project &proj)
    {
        fs::path projPath = projectsPath / proj.lang / proj.folderName;
        try
        {
            if (!fs::exists(projPath))
            {
                fs::create_directories(projPath);
                Canvas::PrintInfo("Created project directory: " + projPath.string());
            }
            else
            {
                Canvas::PrintInfo("Project directory already exists: " + projPath.string());
            }
        }
        catch (const fs::filesystem_error &e)
        {
            Canvas::PrintError("Error creating project directory: " + std::string(e.what()));
        }
    }

    inline void syncDevMap()
    {
        users.clear();

        // 1. Validate languages from JSON and remove those that no longer exist.
        std::vector<std::string> validLanguages;
        if (devmapData.contains("Languages") && devmapData["Languages"].is_array())
        {
            for (const auto &lang : devmapData["Languages"])
            {
                std::string language = lang.get<std::string>();
                fs::path langPath = projectsPath / language;
                if (fs::exists(langPath))
                {
                    validLanguages.push_back(language);
                }
                else
                {
                    Canvas::PrintInfo("Language '" + language + "' has been moved or deleted: " + langPath.string());
                }
            }
        }
        languages = validLanguages;

        // 2. Scan the filesystem for language directories not listed in JSON and add them.
        for (const auto &entry : fs::directory_iterator(projectsPath))
        {
            if (entry.is_directory())
            {
                std::string langDir = entry.path().filename().string();
                if (std::find(languages.begin(), languages.end(), langDir) == languages.end())
                {
                    languages.push_back(langDir);
                    Canvas::PrintInfo("Added new language from filesystem to DevMap: " + langDir);
                }
            }
        }

        // Update the Languages JSON array.
        nlohmann::json newLanguagesJson = nlohmann::json::array();
        for (const auto &lang : languages)
        {
            newLanguagesJson.push_back(lang);
        }
        devmapData["Languages"] = newLanguagesJson;

        // 3. Rebuild the projects vector from JSON, keeping only those projects that exist.
        std::vector<Project> validProjects;
        nlohmann::json validProjectsJson = nlohmann::json::array();
        if (devmapData.contains("Projects") && devmapData["Projects"].is_array())
        {
            for (const auto &projData : devmapData["Projects"])
            {
                std::string lang = projData.value("lang", "");
                std::string folderName = projData.value("folderName", "");
                fs::path projPath = projectsPath / lang / folderName;
                if (fs::exists(projPath))
                {
                    Project proj;
                    proj.name = projData.value("name", "");
                    proj.folderName = folderName;
                    proj.lang = lang;
                    proj.createdBy = projData.value("created_by", "");
                    std::string createdAtStr = projData.value("created_at", "");
                    proj.createdAt = parseTime(createdAtStr);
                    proj.size = projData.value("size", 0);
                    proj.usesGit = projData.value("git", false);
                    validProjects.push_back(proj);

                    nlohmann::json projJson = {
                        {"name", proj.name},
                        {"folderName", proj.folderName},
                        {"lang", proj.lang},
                        {"created_by", proj.createdBy},
                        {"created_at", timeToString(proj.createdAt)},
                        {"size", proj.size},
                        {"git", proj.usesGit}
                    };
                    validProjectsJson.push_back(projJson);

                    users.insert(proj.createdBy);
                }
                else
                {
                    Canvas::PrintInfo("Project '" + projPath.string() + "' has been moved or deleted.");
                }
            }
        }
        projects = validProjects;
        devmapData["Projects"] = validProjectsJson;

        // 4.5. Update existing project data (size and Git status) from the filesystem.
        for (auto &projData : devmapData["Projects"])
        {
            std::string language = projData.value("lang", "");
            std::string folderName = projData.value("folderName", "");
            fs::path projPath = projectsPath / language / folderName;
            if (fs::exists(projPath) && fs::is_directory(projPath))
            {
                std::string fullProjPath = projPath.string();
                size_t currentSize = getFolderSize(fullProjPath);
                bool currentUsesGit = usesGit(fullProjPath);
                projData["size"] = currentSize;
                projData["git"] = currentUsesGit;
                // Update the corresponding project in the projects vector.
                for (auto &proj : projects)
                {
                    if (proj.folderName == folderName && proj.lang == language)
                    {
                        proj.size = currentSize;
                        proj.usesGit = currentUsesGit;
                        break;
                    }
                }
            }
        }

        // 5. For every language directory, add any project directory not listed in the JSON.
        for (const auto &language : languages)
        {
            fs::path langPath = projectsPath / language;
            if (!fs::exists(langPath))
                continue;
            for (const auto &entry : fs::directory_iterator(langPath))
            {
                if (entry.is_directory())
                {
                    std::string folderName = entry.path().filename().string();
                    bool found = false;
                    for (const auto &proj : projects)
                    {
                        if (proj.folderName == folderName && proj.lang == language)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        // New project detected on the filesystem; add it with default values.
                        Project newProj;
                        std::string projectPath = (projectsPath / language / folderName).string();
                        newProj.name = folderName; // Default: use folder name as project name.
                        newProj.folderName = folderName;
                        newProj.lang = language;
                        newProj.createdBy = getCurrentUser();
                        newProj.createdAt = std::time(nullptr);
                        newProj.size = getFolderSize(projectPath);
                        newProj.usesGit = usesGit(projectPath);
                        projects.push_back(newProj);

                        nlohmann::json newProjJson = {
                            {"name", newProj.name},
                            {"folderName", newProj.folderName},
                            {"lang", newProj.lang},
                            {"created_by", newProj.createdBy},
                            {"created_at", timeToString(newProj.createdAt)},
                            {"size", newProj.size},
                            {"git", newProj.usesGit}
                        };
                        devmapData["Projects"].push_back(newProjJson);
                        Canvas::PrintInfo("Added new project from filesystem to DevMap: " + folderName + " in " + language);
                    }
                }
            }
        }

        // 6. Optionally update the users vector from JSON.
        if (devmapData.contains("Users") && devmapData["Users"].is_array())
        {
            for (const auto user : devmapData["Users"])
                users.insert(user.get<std::string>());
            devmapData["Users"].clear();
            for (const auto &user : users)
                devmapData["Users"].push_back(user);
        }

        // 7. Write the updated JSON back to the file.
        std::ofstream outFile(devmapFileName);
        if (outFile.is_open())
        {
            outFile << devmapData.dump(4); // Pretty-print with indentations.
            outFile.close();
        }
        else
        {
            Canvas::PrintError("Unable to write to DevMap file: " + devmapFileName.string());
        }
    }


    // Load the DevMap from a JSON file.
    inline bool load(const std::string &filename, bool install = false)
    {

        if (install)
        {
            Canvas::PrintInfo("Checking for required directories");
            fs::path devmapPath = filename;
            fs::path devmapDir = devmapPath.parent_path();

            if (!fs::exists(devmapDir))
            {
                fs::create_directories(devmapDir);
            }

            // Clone and overwrite the DevMap file with the default template.
            Canvas::PrintInfo("Cloning the DevCore repository to retrieve the default DevMap.");
            std::string cloneCommand = "git clone " + Config::github + " /tmp/devcore_repo";
            if (std::system(cloneCommand.c_str()) != 0)
            {
                Canvas::PrintErrorExit("Failed to clone repository from " + Canvas::LinkText(Config::github));
            }

            fs::path sourceConfig = "/tmp/devcore_repo/devmap.json";
            if (!fs::exists(sourceConfig))
            {
                Canvas::PrintErrorExit("Default DevMap file not found in the cloned repository.");
            }
            Canvas::PrintInfo("Copying the new DevMap to '" + Canvas::LinkText(filename, Canvas::Color::CYAN) + "'");
            fs::copy_file(sourceConfig, devmapPath, fs::copy_options::overwrite_existing);
            Canvas::PrintInfo("Removing the temporary cloned repository.");
            fs::remove_all("/tmp/devcore_repo");
            Canvas::PrintSuccess(Canvas::BoldText("Done installing the default DevMap.") +
                                 Canvas::ColorToAnsi(Canvas::Color::GREEN) +
                                 "\n    You can list and manage projects in your devmap by running several commands (see `devcore --help` for more info). \n    You can edit the devmap manually at '" +
                                 Canvas::LinkText(filename, Canvas::Color::GREEN) + "', however, this is not recommended!");
        }

        // Save the filename and get projectsPath from configuration.
        devmapFileName = filename;
        projectsPath = Main::HOME_PATH + Config::get("projects_path");

        std::ifstream file(filename);
        if (!file.is_open())
        {
            return false;
        }

        try
        {
            file >> devmapData;
        }
        catch (const std::exception &e)
        {
            Canvas::PrintError("Failed to parse the DevMap file: " + std::string(e.what()));
            return false;
        }

        // At this point the JSON has been read.
        // The expected JSON structure is:
        // {
        //     "Projects": [
        //         {
        //             "name": "DevCore Project Manager",
        //             "folderName": "DevCore-project-manager",
        //             "lang": "C++",
        //             "created_by": "Huplo",
        //             "created_at": "23:04 17-03-2025",
        //             "size": 25042,
        //             "git": true,
        //         },
        //         ...
        //     ],
        //     "Languages": ["Java", "C++"],
        //     "Users": ["Huplo"]
        // }

        // Synchronize the JSON data with the filesystem.
        syncDevMap();

        return true;
    }

    // Setup function to create a default DevMap if one does not exist.
    inline int setup(const std::string &filename)
    {
        Canvas::ClearConsole();
        Canvas::PrintTitle("DevCore | Setup Zone");
        Canvas::PrintWarning("It seems like you do not yet have a DevMap file. You require the correct structure and we recommend you download the default template. Would you like to install the default (empty) DevMap? \n    If not, check out '" + Canvas::LinkText(filename, Canvas::Color::YELLOW) +
                             "' to configure one manually, although this is not recommended!");
        if (Canvas::GetBoolInput("    "))
        {
            load(filename, true);
            return 0;
        }
        return 0;
    }

    // Validate that the DevMap has been loaded.
    inline void validate()
    {
        if (devmapData.empty())
        {
            setup(devmapFileName);
            exit(0);
        }
    }

    // Return a string representation of the current DevMap configuration.
    inline std::string GetStringRepresentation()
    {
        validate();
        return devmapData.dump(4);
    }

    inline void ListProjects(bool extra = false)
    {
        std::vector<std::string> header;
        std::vector<std::vector<std::string>> rows;

        if (!extra)
        {
            header = {"Created By", "Name", "Language"};
            for (const auto &proj : projects)
            {
                rows.push_back({proj.createdBy,
                                proj.name,
                                proj.lang});
            }
        }
        else
        {
            header = {"Created By", "Name", "Folder", "Language", "Created At", "Size", "Git"};
            for (const auto &proj : projects)
            {
                rows.push_back({proj.createdBy,
                                proj.name,
                                proj.folderName,
                                proj.lang,
                                timeToString(proj.createdAt),
                                std::to_string(proj.size),
                                proj.usesGit ? "Yes" : "No"});
            }
        }
        // Display the table with the default color.
        Canvas::PrintTable(" Projects ", header, rows, Canvas::Color::CYAN);
    }

    inline void ListUsers()
    {
        std::vector<std::string> header;
        std::vector<std::vector<std::string>> rows;

        header = {"Users"};
        for (const auto &user : users)
        {
            rows.push_back({user});
        }
        // Display the table with the default color.
        Canvas::PrintTable("", header, rows, Canvas::Color::CYAN);
    }

    inline void ListTemplates()
    {
        std::vector<std::string> header = {"Templates"};
        std::vector<std::vector<std::string>> rows;

        // Build the template directory path
        std::string templateDir = Main::HOME_PATH + Main::TEMPLATE_PATH + "/";

        try {
            // Check if the template directory exists and is a directory
            if (fs::exists(templateDir) && fs::is_directory(templateDir)) {
                // Iterate over first-level directories (subdir1)
                for (const auto& entry : fs::directory_iterator(templateDir)) {
                    if (entry.is_directory()) {
                        std::string subdir1Name = entry.path().filename().string();

                        // Iterate over second-level directories (subdir2) inside each subdir1
                        for (const auto& subentry : fs::directory_iterator(entry.path())) {
                            if (subentry.is_directory()) { // You can modify this condition if you also want files
                                std::string subdir2Name = subentry.path().filename().string();
                                // Combine the names with a '/'
                                std::string templateName = subdir1Name + "/" + subdir2Name;
                                rows.push_back({templateName});
                            }
                        }
                    }
                }
            } else {
                std::cerr << "Directory not found: " << templateDir << std::endl;
            }
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
        }

        // Display the table with the default color.
        Canvas::PrintTable("", header, rows, Canvas::Color::CYAN);
    }

    inline void ListLanguages()
    {
        std::vector<std::string> header;
        std::vector<std::vector<std::string>> rows;

        header = {"Languages   "};
        for (const auto &lang : languages)
        {
            rows.push_back({lang});
        }
        // Display the table with the default color.
        Canvas::PrintTable("", header, rows, Canvas::Color::CYAN);
    }

    inline void DeleteLanguage(std::string &lang)
    {
        // Verify that the language exists in the languages vector.
        auto it = std::find(languages.begin(), languages.end(), lang);
        if (it == languages.end())
        {
            Canvas::PrintInfo("Language '" + lang + "' does not exist.");
            return;
        }

        // Construct paths for the language and template directories.
        fs::path langPath = projectsPath / lang;
        fs::path templatePath = Main::HOME_PATH + Main::TEMPLATE_PATH + "/" + lang;

        bool returnEarly = false;
        // Check if the language directory exists and is empty.
        if (fs::exists(langPath))
        {
            if (!fs::is_empty(langPath))
            {
                Canvas::PrintError("Cannot delete language directory '" + langPath.string() + "': Directory is not empty. You will have to empty this yourself or by deleting each project with DevCore commands.");
                returnEarly = true;
            }
        }

        // Check if the template directory exists and is empty.
        if (fs::exists(templatePath))
        {
            if (!fs::is_empty(templatePath))
            {
                Canvas::PrintError("Cannot delete template directory '" + templatePath.string() + "': Directory is not empty. You will have to empty this yourself or by deleting each template with DevCore commands");
                returnEarly = true;
            }
        }

        if (returnEarly)
            return;
        

        // Attempt to delete the language directory.
        if (fs::exists(langPath))
        {
            if (fs::remove(langPath))
                Canvas::PrintInfo("Deleted language directory: " + langPath.string());
            else
            {
                Canvas::PrintError("Failed to delete language directory: " + langPath.string());
                return;
            }
        }
        
        // Attempt to delete the template directory.
        if (fs::exists(templatePath))
        {
            if (fs::remove(templatePath))
                Canvas::PrintInfo("Deleted template directory: " + templatePath.string());
            else
            {
                Canvas::PrintError("Failed to delete template directory: " + templatePath.string());
                return;
            }
        }

        // Remove the language from the languages vector.
        languages.erase(it);

        // Update the JSON: remove the language from the "Languages" array.
        if (devmapData.contains("Languages") && devmapData["Languages"].is_array())
        {
            nlohmann::json newLangArray = nlohmann::json::array();
            for (const auto &item : devmapData["Languages"])
            {
                if (item.get<std::string>() != lang)
                {
                    newLangArray.push_back(item);
                }
            }
            devmapData["Languages"] = newLangArray;
        }

        // Write the updated JSON back to the file.
        std::ofstream outFile(devmapFileName);
        if (outFile.is_open())
        {
            outFile << devmapData.dump(4); // Pretty-print with indentations.
            outFile.close();
            Canvas::PrintInfo("DevMap updated successfully.");
        }
        else
        {
            Canvas::PrintError("Unable to write updated DevMap to: " + devmapFileName.string());
        }
    }


    inline void CreateLang(std::string &lang)
    {
        // Check if the language is already in the vector.
        if (std::find(languages.begin(), languages.end(), lang) == languages.end())
        {
            // Add to the languages vector.
            languages.push_back(lang);

            // Create the language folder if it does not exist.
            fs::path langPath = projectsPath / lang;
            if (!fs::exists(langPath))
            {
                fs::create_directories(langPath);
                Canvas::PrintInfo("Created language directory: " + langPath.string());
            }

            fs::path templatePath =  Main::HOME_PATH + Main::TEMPLATE_PATH + '/' + lang;
            if (!fs::exists(templatePath))
            {
                fs::create_directories(templatePath);
                Canvas::PrintInfo("Created template directory: " + templatePath.string());
            }

            // Ensure the JSON "Languages" array exists.
            if (!devmapData.contains("Languages") || !devmapData["Languages"].is_array())
            {
                devmapData["Languages"] = nlohmann::json::array();
            }

            // Add the language to the JSON data.
            devmapData["Languages"].push_back(lang);
            Canvas::PrintInfo("Added language to DevMap: " + lang);

            // Write the updated JSON back to the file.
            std::ofstream outFile(devmapFileName);
            if (outFile.is_open())
            {
                outFile << devmapData.dump(4); // Pretty-print with indentations.
                outFile.close();
                Canvas::PrintInfo("DevMap updated successfully.");
            }
            else
            {
                Canvas::PrintError("Unable to write updated DevMap to: " + devmapFileName.string());
            }
        }
        else
        {
            Canvas::PrintInfo("Language already exists: " + lang);
        }
    }

    inline void CreateProjectWizard()
    {
        // Clear the console and print a vibrant title.
        Canvas::ClearConsole();
        Canvas::PrintTitle(u8"DevCore | Project Creation Wizard 🚀", Canvas::Color::MAGENTA);

        // 1. Ask for the project language.
        ListLanguages();
        std::string projectLang = Canvas::GetStringInput(u8"👉 Please enter the project language: ", "", Canvas::Color::CYAN);
        // Verify if the language exists; if not, offer to create it.
        if (std::find(languages.begin(), languages.end(), projectLang) == languages.end())
        {
            bool createLang = Canvas::GetBoolInput(u8"⚠️ Language '" + projectLang + "' not found. Create it? ", "", Canvas::Color::YELLOW);
            if (createLang)
            {
                CreateLang(projectLang);
                Canvas::PrintSuccess(u8"Language '" + projectLang + "' created successfully!");
            }
            else
            {
                Canvas::PrintInfo(u8"❌ Project creation cancelled. Please choose an existing language next time.");
                return;
            }
        }

        // 2. Ask for the project name.
        std::string projectName = Canvas::GetStringInput(u8"📝 Enter your project name (spaces allowed): ", "", Canvas::Color::CYAN);

        // 3. Determine the project folder name.
        bool useNamingConvention = Canvas::GetBoolInput(u8"🔠 Use GitHub naming conventions for folder name? ", "", Canvas::Color::CYAN);
        std::string projectFolderName;
        if (useNamingConvention)
        {
            projectFolderName = projectName;
            // Convert to lowercase.
            std::transform(projectFolderName.begin(), projectFolderName.end(), projectFolderName.begin(), ::tolower);
            // Replace spaces with hyphens.
            std::replace(projectFolderName.begin(), projectFolderName.end(), ' ', '-');
            // Remove any characters other than alphanumeric or hyphen.
            projectFolderName.erase(std::remove_if(projectFolderName.begin(), projectFolderName.end(),
                [](char c) { return !(std::isalnum(c) || c == '-'); }), projectFolderName.end());
            Canvas::PrintInfo(u8"📁 Using folder name: " + projectFolderName);
        }
        else
        {
            projectFolderName = Canvas::GetStringInput(u8"📁 Enter a custom project folder name: ", "", Canvas::Color::CYAN);
        }

        // 4. Ask if the project should be initialized as a Git repository.
        bool initGit = Canvas::GetBoolInput(u8"🐙 Initialize as a Git repository? ", "", Canvas::Color::CYAN);

        // 5. Ask if the user wants to use a project template.
        bool useTemplate = Canvas::GetBoolInput(u8"🎨 Would you like to apply a project template? ", "", Canvas::Color::CYAN);
        std::string selectedTemplate = "";
        if (useTemplate)
        {
            // Template directory: HOME_PATH/TEMPLATE_PATH/projectLang.
            fs::path templateDir = fs::path(Main::HOME_PATH) / Main::TEMPLATE_PATH / projectLang;
            if (!fs::exists(templateDir) || !fs::is_directory(templateDir))
            {
                Canvas::PrintInfo(u8"📂 No templates available for '" + projectLang + "'. Skipping template.");
                useTemplate = false;
            }
            else
            {
                // List available templates.
                std::vector<std::string> templates;
                for (const auto &entry : fs::directory_iterator(templateDir))
                {
                    if (entry.is_directory())
                        templates.push_back(entry.path().filename().string());
                }
                if (templates.empty())
                {
                    Canvas::PrintInfo(u8"📂 No templates found in " + templateDir.string() + ". Skipping template.");
                    useTemplate = false;
                }
                else
                {
                    Canvas::PrintInfo(u8"✨ Available templates:");
                    for (size_t i = 0; i < templates.size(); i++)
                    {
                        Canvas::PrintInfo(u8"  " + std::to_string(i + 1) + u8". " + templates[i]);
                    }
                    std::string templateChoice = Canvas::GetStringInput(u8"🔢 Enter template number (or press Enter to skip): ", "", Canvas::Color::CYAN);
                    if (!templateChoice.empty())
                    {
                        int choice = std::stoi(templateChoice);
                        if (choice > 0 && choice <= static_cast<int>(templates.size()))
                        {
                            selectedTemplate = templates[choice - 1];
                            Canvas::PrintInfo(u8"🎉 Template '" + selectedTemplate + "' selected.");
                        }
                        else
                        {
                            Canvas::PrintInfo(u8"❌ Invalid choice. Skipping template.");
                            useTemplate = false;
                        }
                    }
                    else
                    {
                        useTemplate = false;
                    }
                }
            }
        }

        // 6. Prepare the new project data.
        Project newProj;
        newProj.name = projectName;
        newProj.folderName = projectFolderName;
        newProj.lang = projectLang;
        newProj.createdBy = "current_user";  // Replace with actual user info if available.
        newProj.createdAt = std::time(nullptr);
        newProj.size = 0;  // Will be updated if a template is applied.
        newProj.usesGit = initGit;

        // 7. Create the project directory.
        CreateProject(newProj);
        Canvas::PrintSuccess(u8"🚀 Project directory created successfully!");
        bool openInCode = Canvas::GetBoolInput(u8"🎨 Would you like to open this project in Visual Studio Code? ", "", Canvas::Color::CYAN);


        // 8. If a template was selected, copy its contents into the new project folder.
        if (useTemplate && !selectedTemplate.empty())
        {
            fs::path templatePath = fs::path(Main::HOME_PATH) / Main::TEMPLATE_PATH / projectLang / selectedTemplate;
            fs::path projectPath = projectsPath / projectLang / projectFolderName;
            try
            {
                fs::copy(templatePath, projectPath, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                Canvas::PrintSuccess(u8"✨ Template '" + selectedTemplate + "' applied to project.");
            }
            catch (const std::exception &e)
            {
                Canvas::PrintError(u8"Error copying template: " + std::string(e.what()));
            }
            // Update project size after copying template contents.
            newProj.size = getFolderSize(projectPath.string());
        }

        // 9. Initialize Git repository if requested.
        if (initGit)
        {
            fs::path projectPath = projectsPath / projectLang / projectFolderName;
            std::string initCommand = "cd " + projectPath.string() + " && git init";
            if (std::system(initCommand.c_str()) == 0)
            {
                Canvas::PrintSuccess(u8"🐙 Git repository initialized in " + projectPath.string());
            }
            else
            {
                Canvas::PrintError(u8"Failed to initialize Git repository in " + projectPath.string());
            }
        }

        // 10. Update the DevMap JSON with the new project entry.
        nlohmann::json projJson = {
            {"name", newProj.name},
            {"folderName", newProj.folderName},
            {"lang", newProj.lang},
            {"created_by", newProj.createdBy},
            {"created_at", timeToString(newProj.createdAt)},
            {"size", newProj.size},
            {"git", newProj.usesGit}
        };
        devmapData["Projects"].push_back(projJson);
        std::ofstream outFile(devmapFileName);
        if (outFile.is_open())
        {
            outFile << devmapData.dump(4); // Pretty-print with indentations.
            outFile.close();
        }
        else
        {
            Canvas::PrintError("Unable to write to DevMap file: " + devmapFileName.string());
        }
        Canvas::PrintSuccess(u8"✅ Project '" + newProj.name + "' created successfully!");

        if (openInCode)
        {
            std::string openCodeCmd = "code " + std::string(projectsPath / projectLang / projectFolderName);
            if (std::system(openCodeCmd.c_str()) != 0)
            {
                Canvas::PrintError(u8"❌ Failed to open the project in Visual Studio Code, make sure its installed and added to your PATH.");
            }
        }
        
    }


    inline void DeleteProjectWizard()
    {
        // Clear the console and print a vibrant title.
        Canvas::ClearConsole();
        Canvas::PrintColoredLine(u8"*========== DevCore | Danger Zone | Project Deletion Wizard ❌ ==========*", Canvas::Color::RED);

        // 1. List projects and ask for the project name to delete.
        ListProjects(true);
        std::string projectName = Canvas::GetStringInput(u8"👉 Please enter the project name you want to delete: ", "", Canvas::Color::CYAN);
        
        Project project;
        bool found = false;
        for (auto &proj : projects)
        {
            if (proj.name == projectName)
            {
                found = true;
                project = proj;
                break; // Exit loop once the project is found.
            }
        }

        if (!found || !fs::exists(projectsPath / project.lang / project.folderName))
        {
            Canvas::PrintErrorExit("You tried to delete '" + projectName + "'. No such project exists");
        }

        fs::path projPath = projectsPath / project.lang / project.folderName;
        
        // 2. Confirm deletion with the user.
        Canvas::ClearConsole();
        bool confirmation1 = Canvas::GetBoolInput(u8"🔥 Are you absolutely sure you want to delete '" + projectName + "' located at '" + Canvas::LinkText(projPath.string(), Canvas::Color::RED) + "'?", "Delete Project Confirmation 1", Canvas::Color::RED);
        Canvas::ClearConsole();
        bool confirmation2 = Canvas::GetBoolInput(u8"🔥 Please confirm again: Delete '" + projectName + "' from '" + Canvas::LinkText(projPath.string(), Canvas::Color::RED) + "'?", "Delete Project Confirmation 2", Canvas::Color::RED);

        if (confirmation1 && confirmation2)
        {
            // 3. Attempt to delete the project directory recursively.
            std::error_code ec;
            auto removedCount = fs::remove_all(projPath, ec);
            if (ec)
            {
                Canvas::PrintError("Failed to delete project directory '" + Canvas::LinkText(projPath.string(), Canvas::Color::RED) + "'. Error: " + ec.message());
                return;
            }
            else
            {
                Canvas::PrintInfo("Deleted " + std::to_string(removedCount) + " items from " + Canvas::LinkText(projPath.string()));
            }

            // 4. Remove the project from the projects vector.
            projects.erase(std::remove_if(projects.begin(), projects.end(),
                [&](const Project &p) { return p.name == projectName && p.lang == project.lang; }),
                projects.end());

            // 5. Update the devmapData JSON: remove the project entry.
            if (devmapData.contains("Projects") && devmapData["Projects"].is_array())
            {
                nlohmann::json newProjects = nlohmann::json::array();
                for (auto &projJson : devmapData["Projects"])
                {
                    if (projJson.value("name", "") != projectName)
                    {
                        newProjects.push_back(projJson);
                    }
                }
                devmapData["Projects"] = newProjects;

                // Write the updated JSON back to the file.
                std::ofstream outFile(devmapFileName);
                if (outFile.is_open())
                {
                    outFile << devmapData.dump(4); // Pretty-print with indentations.
                    outFile.close();
                }
                else
                {
                    Canvas::PrintError("Unable to write updated DevMap to: " + devmapFileName.string());
                }
            }
            
            Canvas::PrintSuccess(u8"✅ Project '" + project.name + "' deleted successfully!");
        }
        else
        {
            Canvas::PrintInfo(u8"Project deletion aborted.");
        }
    }

    inline void RemoveTemplate()
    {
        Canvas::ClearConsole();
        ListTemplates();

        std::string templateDir = Canvas::GetStringInput(u8"👉 Please enter a template listed above that you want to delete: ", "", Canvas::Color::CYAN);
        std::string delDir = Main::HOME_PATH + Main::TEMPLATE_PATH + "/" + templateDir;
        Canvas::ClearConsole();
        bool confirmation1 = Canvas::GetBoolInput(u8"🔥 Are you absolutely sure you want to delete '" + templateDir + "' located at '" + Canvas::LinkText(delDir, Canvas::Color::RED) + "'?", "Delete Template Confirmation 1", Canvas::Color::RED);
        Canvas::ClearConsole();
        bool confirmation2 = Canvas::GetBoolInput(u8"🔥 Please confirm again: Delete '" + templateDir + "' from '" + Canvas::LinkText(delDir, Canvas::Color::RED) + "'?", "Delete Template Confirmation 2", Canvas::Color::RED);

        if (confirmation1 && confirmation2)
        {
            // 3. Attempt to delete the project directory recursively.
            std::error_code ec;
            auto removedCount = fs::remove_all(delDir, ec);
            if (ec)
            {
                Canvas::PrintError("Failed to delete template directory '" + Canvas::LinkText(delDir, Canvas::Color::RED) + "'. Error: " + ec.message());
                return;
            }
            else
            {
                Canvas::PrintInfo("Deleted " + std::to_string(removedCount) + " items from " + Canvas::LinkText(delDir));
            }

            Canvas::PrintSuccess(u8"✅ Template '" + templateDir + "' deleted successfully!");
        }
        else
        {
            Canvas::PrintInfo(u8"Project deletion aborted.");
        }

    }

    inline void AddTemplate()
    {
        Canvas::ClearConsole();
        std::string name = Canvas::GetStringInput(u8"👉 Please enter a template name: ", "", Canvas::Color::CYAN);
        std::string lang = Canvas::GetStringInput(u8"👉 Please enter the template language: ", "", Canvas::Color::CYAN);
        std::string source = Canvas::GetStringInput(u8"👉 Please enter the template source folder path: ", "", Canvas::Color::CYAN);

        if (std::find(languages.begin(), languages.end(), lang) == languages.end())
        {
            Canvas::PrintWarning("The language does not exist yet, would you like to create it?");
            if (Canvas::GetBoolInput("   "))
            {
                CreateLang(lang);
            }
            else
            {
                Canvas::PrintInfo("Aborting template addition.");
                return;
            }
        }

        // Construct the target directory path
        std::string targetDir = Main::HOME_PATH + Main::TEMPLATE_PATH + lang + "/" + name;
        
        // Create the target directory if it doesn't exist
        std::string mkdirCommand = "mkdir -p " + targetDir;
        system(mkdirCommand.c_str());

        // Copy the contents of the source directory into the target directory
        // Using '/*' to copy the contents rather than the directory itself
        std::string copyCommand = "cp -r " + source + "/* " + targetDir;
        system(copyCommand.c_str());

        Canvas::PrintSuccess("Succesfully added your template to the " + Canvas::LinkText(".config/devcore/templates", Canvas::Color::GREEN) + " directory.");
    }




} // namespace DevMap

#endif // DEVMAP_HPP
