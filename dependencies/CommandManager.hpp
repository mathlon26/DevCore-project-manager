#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include "CommandHandler.h"

class CommandManager
{
private:
    std::map<std::string, CommandHandler*> m_commands;
    std::string m_mainCommand;
public:
    CommandManager(std::string mainCommand) : m_mainCommand{mainCommand} {};
    ~CommandManager() 
    {
        for (const auto& [command, handler] : m_commands)
            delete handler;
    };

    void Execute(std::string command, std::vector<std::string>& argv);
    void AddCommand(std::string command, CommandHandler* handler);
    void AddCommand(std::vector<std::string> aliasses, CommandHandler* handler);

    static void Error(std::string error)
    {
        ConsoleLayout::PrintError(error);
        exit(EXIT_FAILURE);
    }

};

void CommandManager::AddCommand(std::string command, CommandHandler* handler)
{
    m_commands.insert(std::make_pair(command, handler));
}

void CommandManager::AddCommand(std::vector<std::string> aliasses, CommandHandler* handler)
{
    for (auto command : aliasses)
        m_commands.insert(std::make_pair(command, handler));
}

void CommandManager::Execute(std::string command, std::vector<std::string>& argv)
{
    auto it = m_commands.find(command);
    if (it != m_commands.end()) {
        it->second->Execute(command, argv);
    } else {
        Error(m_mainCommand + ' ' + command + " is not recognized, use " + m_mainCommand + " --help to view all possible combinations for " + m_mainCommand);
    }
}

#endif // COMMAND_MANAGER_H
