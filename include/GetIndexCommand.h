#ifndef GET_INDEX_COMMAND__H
#define GET_INDEX_COMMAND__H

#include "../dependencies/CommandHandler.h"
#include "ProjectManager.h"

class GetIndexCommand : public CommandHandler
{
public:
    GetIndexCommand(ProjectManager* manager) : m_manager{manager}, CommandHandler{} {};
    ~GetIndexCommand() {};

    void Execute(std::string& command, std::vector<std::string>& argv) override;
private:
    ProjectManager* m_manager;
};

void GetIndexCommand::Execute(std::string& command, std::vector<std::string>& argv)
{
    ConsoleLayout::PrintHeader("Project Manager | Index dir");
    ConsoleLayout::PrintUnicodeBox(ConsoleLayout::CYAN + ConsoleLayout::BOLD + m_manager->GetIndexDir() + ConsoleLayout::RESET);
}

#endif // GET_INDEX_COMMAND__H