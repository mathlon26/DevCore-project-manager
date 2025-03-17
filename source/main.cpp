#include "../include/ProjectManager.h"
#include "../include/ListCommand.h"
#include "../include/GetIndexCommand.h"
#include "../dependencies/CommandManager.hpp"

const std::string mainCommand = "pm";

void ParseCommand(int argc, char const *argv[], std::string& command, std::vector<std::string>& arguments)
{
    if (argc > 1) {
        command = argv[1]; // First argument as command
        for (int i = 2; i < argc; ++i) {
            arguments.emplace_back(argv[i]); // Add remaining arguments
        }
    }
}
int main(int argc, char const *argv[])
{
    ProjectManager* pm{new ProjectManager};
    CommandManager* cm{new CommandManager{mainCommand}};

    GetIndexCommand* getIndexCommand{new GetIndexCommand{pm}};
    cm->AddCommand(std::vector<std::string>{"-I", "--index", "--source"}, getIndexCommand);

    ListCommand* listCommand{new ListCommand{pm}};
    cm->AddCommand(std::vector<std::string>{"-L", "-Ll", "-Lu", "-Lt"}, listCommand);



    std::string command;
    std::vector<std::string> arguments;
    ParseCommand(argc, argv, command, arguments);

    cm->Execute(command, arguments);
    return 0;
}