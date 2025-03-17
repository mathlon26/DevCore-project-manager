#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>
#include "ConsoleLayout.h"

class CommandHandler
{
private:
public:
    CommandHandler() {};
    ~CommandHandler() {};

    virtual void Execute(std::string& command, std::vector<std::string>& argv) = 0;
};

#endif // COMMAND_HANDLER