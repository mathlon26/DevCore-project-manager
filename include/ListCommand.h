#ifndef LIST_COMMAND__H
#define LIST_COMMAND__H

#include "../dependencies/CommandHandler.h"
#include "ProjectManager.h"
#include <sstream>  // For efficient string concatenation

class ListCommand : public CommandHandler
{
public:
    ListCommand(ProjectManager* manager) : m_manager{manager}, CommandHandler{} {};
    ~ListCommand() {};

    void Execute(std::string& command, std::vector<std::string>& argv) override;
private:
    ProjectManager* m_manager;
};

void ListCommand::Execute(std::string& command, std::vector<std::string>& argv)
{
    if (argv.size() > 1)
    {
        CommandManager::Error("Too many arguments for 'pm " + command + "'.");
    }
    
    // List all projects in a table
    if (argv.empty() && command == "-L")
    {
        ConsoleLayout::PrintHeader("Project Manager | Project list");
        std::vector<std::vector<std::string>> rows{};
        for (Project* project : m_manager->Projects)
            rows.push_back(project->GetTableRow());

        ConsoleLayout::PrintTableUnicode(Project::GetTableHeader(), rows);
        return;
    }

    // List languages using Unicode box with " : " separator
    if (argv.empty() && command == "-Ll")
    {
        ConsoleLayout::PrintHeader("Project Manager | Language list");

        std::ostringstream oss;
        bool first = true;
        for (const std::string &lang : m_manager->Languages)
        {
            if (!first)
                oss << " : ";
            oss << ConsoleLayout::CYAN << lang << ConsoleLayout::RESET;
            first = false;
        }
        ConsoleLayout::PrintUnicodeBox(oss.str());
        return;
    }

    // List titles using Unicode box with " : " separator
    if (argv.empty() && command == "-Lt")
    {
        ConsoleLayout::PrintHeader("Project Manager | Title list");

        std::ostringstream oss;
        bool first = true;
        for (Project* project : m_manager->Projects)
        {
            if (!first)
                oss << " : ";
            oss << ConsoleLayout::CYAN << project->GetName() << ConsoleLayout::RESET;
            first = false;
        }
        ConsoleLayout::PrintUnicodeBox(oss.str());
        return;
    }

    // List users using Unicode box with " : " separator
    if (argv.empty() && command == "-Lu")
    {
        ConsoleLayout::PrintHeader("Project Manager | User list");

        std::ostringstream oss;
        bool first = true;
        for (const std::string &user : m_manager->GetUsers())
        {
            if (!first)
                oss << " : ";
            oss << ConsoleLayout::CYAN << user << ConsoleLayout::RESET;
            first = false;
        }

        std::string users = oss.str();
        if (users.empty())
        {
            CommandManager::Error("No users with projects.");
        }
        ConsoleLayout::PrintUnicodeBox(users);
        return;
    }

    // List projects under a specific user
    if (command == "-Lu")
    {
        ConsoleLayout::PrintHeader("Project Manager | Project list | " + argv[0]);
        std::vector<std::vector<std::string>> rows{};
        for (Project* project : m_manager->Projects)
        {
            if (project->GetCreatedBy() == argv[0])
                rows.push_back(project->GetTableRow());
        }

        if (rows.empty())
        {
            CommandManager::Error("No projects under user " + argv[0] + " exist.");
        }
        ConsoleLayout::PrintTableUnicode(Project::GetTableHeader(), rows);
        return;
    }

    // List projects under a specific language
    if (command == "-Ll")
    {
        ConsoleLayout::PrintHeader("Project Manager | Project list | " + argv[0]);
        std::vector<std::vector<std::string>> rows{};
        for (Project* project : m_manager->Projects)
        {
            if (project->GetLang() == argv[0])
                rows.push_back(project->GetTableRow());
        }

        if (rows.empty())
        {
            CommandManager::Error("No projects under language " + argv[0] + " exist.");
        }
        ConsoleLayout::PrintTableUnicode(Project::GetTableHeader(), rows);
        return;
    }

    // List projects with a specific title
    if (command == "-Lt")
    {
        ConsoleLayout::PrintHeader("Project Manager | Project " + argv[0]);
        std::vector<std::vector<std::string>> rows{};
        for (Project* project : m_manager->Projects)
        {
            if (project->GetName() == argv[0])
                rows.push_back(project->GetTableRow());
        }

        if (rows.empty())
        {
            CommandManager::Error("No projects with title " + argv[0] + " exist.");
        }
        ConsoleLayout::PrintTableUnicode(Project::GetTableHeader(), rows);
        return;
    }

    CommandManager::Error("Invalid use of command 'pm " + command + "'.");
}

#endif // LIST_COMMAND__H
