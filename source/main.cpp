#include "../dependencies/Canvas.hpp"
#include "../dependencies/Config.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const std::string CONFIG_PATH = "/.config/devcore/devcore.conf";
const std::string HOME_PATH = getenv("HOME");

void PrintHelp() {
    Canvas::PrintTitle("DevCore | Help Menu", Canvas::Color::CYAN);
    Canvas::PrintBox(
        Canvas::ColorToAnsi(Canvas::Color::YELLOW) + "devcore config get <key>           " + Canvas::ColorToAnsi(Canvas::Color::MAGENTA) + " - Get a config value\n" +
        Canvas::ColorToAnsi(Canvas::Color::YELLOW) + "devcore config set <key> <value>   " + Canvas::ColorToAnsi(Canvas::Color::MAGENTA) + " - Set a config value\n" +
        Canvas::ColorToAnsi(Canvas::Color::YELLOW) + "devcore config reset               " + Canvas::ColorToAnsi(Canvas::Color::MAGENTA) + " - Reset config to default\n" +
        Canvas::ColorToAnsi(Canvas::Color::YELLOW) + "devcore config view                " + Canvas::ColorToAnsi(Canvas::Color::MAGENTA) + " - View current config\n" +
        Canvas::ColorToAnsi(Canvas::Color::YELLOW) + "devcore --help                     " + Canvas::ColorToAnsi(Canvas::Color::MAGENTA) + " - Display the menu you are looking at",
        Canvas::ColorToAnsi(Canvas::Color::CYAN) + " 🛈 Usage ",
        Canvas::Color::CYAN, 
        3 // fix offset issue for '🛈'
    );
}

int HandleConfig(int argc, char const *argv[])
{
    if (argc < 3)
    {
        Canvas::PrintCommandError(argc, argv);
        return 0;
    }
    
    std::string command = argv[2];

    if (command == "get" && argc == 4)
    {
        Canvas::PrintBox(std::string(argv[3]) + " is set to " + Config::get(argv[3]) + ".");
    }
    else if (command == "set" && argc == 5)
    {
        Config::set(argv[3], argv[4]);
        Canvas::PrintBox("set " + std::string(argv[3]) + " to " + std::string(argv[4]) + ".");
    }
    else if (command == "reset" && argc == 3)
    {
        Canvas::PrintTitle("DevCore | Danger Zone", Canvas::Color::RED);
        Canvas::PrintWarning("This is your current config, are you sure you want to reset it to the default config?");
        Canvas::PrintBox(Config::GetKeyValueString(), " devcore.conf ", Canvas::Color::RED);
        if (Canvas::GetBoolInput(""))
        {
            Canvas::PrintInfo("Resetting your config, this may take a while.");
            Config::load(HOME_PATH + CONFIG_PATH, true);
            Canvas::PrintSuccess("Your config has been reset to its default state.");
            Canvas::PrintBox(Config::GetKeyValueString(), " devcore.conf ", Canvas::Color::GREEN);
        }
    }
    else if (command == "view" && argc == 3)
    {
        Canvas::PrintTitle("DevCore | Config Zone", Canvas::Color::CYAN);
        Canvas::PrintInfo("This is your current config. You can find it here: '" + Canvas::LinkText(HOME_PATH + CONFIG_PATH, Canvas::Color::CYAN) + "'");
        Canvas::PrintBox(Config::GetKeyValueString(), " devcore.conf ", Canvas::Color::GREEN);
    }
    else
    {
        Canvas::PrintCommandError(argc, argv);
    }

    return 0;
}

int HandleList(int argc, char const *argv[])
{

    std::string command = argv[2];
    return 0;
}

int main(int argc, char const *argv[]) {
    if (!Config::load(HOME_PATH + CONFIG_PATH))
    {
        Config::setup(HOME_PATH + CONFIG_PATH);
    }

    if (argc < 2)
    {
        Canvas::PrintCommandError(argc, argv);
        return 1;
    }

    std::string command = argv[1];

    if (command == "config")
    {
        return HandleConfig(argc, argv);
    }
    else if (command == "-l" || command == "-la" || command == "list-projects")
    {
        return HandleList(argc, argv);
    }
    else if (command == "--help")
    {
        PrintHelp();
        return 0;
    }

    Canvas::PrintCommandError(argc, argv);
    return 1;
}
