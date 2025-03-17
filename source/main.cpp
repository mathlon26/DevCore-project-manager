#include "../dependencies/Canvas.hpp"
#include "../dependencies/Config.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const std::string CONFIG_PATH = "/.config/devcore/devcore.conf";
const std::string HOME_PATH = getenv("HOME");


int main(int argc, char const *argv[]) {
    if (!Config::load(HOME_PATH + CONFIG_PATH))
    {
        return Config::setup(HOME_PATH + CONFIG_PATH);
    }
    

    if (argc == 4 && std::string{argv[1]} == "config" && std::string{argv[2]} == "get")
    {
        Canvas::PrintBox(std::string(argv[3]) + " is set to " + Config::get(argv[3]) + ".");
    }

    if (argc == 5 && std::string{argv[1]} == "config" && std::string{argv[2]} == "set")
    {
        Config::set(argv[3], argv[4]);
        Canvas::PrintBox("set " + std::string(argv[3]) + " to " + std::string(argv[4]) + ".");
    }

    if (argc == 3 && std::string{argv[1]} == "config" && std::string{argv[2]} == "reset")
    {
        Canvas::PrintTitle("DevCore | Danger Zone", Canvas::Color::RED);
        Canvas::PrintWarning("This is your current config, are you sure you want to reset it to the default config?");
        Canvas::PrintBox(Config::GetKeyValueString(), " devcore.conf ", Canvas::Color::RED);
        if (Canvas::GetBoolInput(""))
        {
            //reset to default
            Canvas::PrintInfo("Resetting your config, this may take a while.");
            Config::load(HOME_PATH + CONFIG_PATH, true);
            Canvas::PrintSuccess("Your config has been reset to its default state.");
            Canvas::PrintBox(Config::GetKeyValueString(), " devcore.conf ", Canvas::Color::GREEN);
        }
    }

    return 0;
}