#ifndef MAIN__H
#define MAIN__H
#include <string.h>

namespace Main
{
    const std::string TEMPLATE_PATH = "/.config/devcore/templates";
    const std::string CONFIG_PATH = "/.config/devcore/devcore.conf";
    const std::string DEVMAP_PATH = "/.config/devcore/devmap.json";
    const std::string HOME_PATH = getenv("HOME");
}

#endif // MAIN__H