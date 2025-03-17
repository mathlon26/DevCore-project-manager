#ifndef CANVAS__H
#define CANVAS__H

#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <vector>

namespace Canvas
{
    // Extended color enumeration, including a DEFAULT for resetting styles.
    enum class Color { RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW, LIGHT_GREEN, PINK, DEFAULT };

    // Convert a Color to its corresponding ANSI escape code.
    inline std::string ColorToAnsi(Color color)
    {
        switch(color)
        {
            case Color::RED:         return "\033[31m";
            case Color::GREEN:       return "\033[32m";
            case Color::YELLOW:      return "\033[33m";
            case Color::BLUE:        return "\033[34m";
            case Color::MAGENTA:     return "\033[35m";
            case Color::CYAN:        return "\033[36m";
            case Color::LIGHT_GREEN: return "\033[92m"; // Bright green.
            case Color::PINK:        return "\033[95m"; // Bright magenta.
            case Color::DEFAULT:     
            default:                 return "\033[0m";
        }
    }

    inline std::string BoldText(const std::string &text)
    {
        return "\033[1m" + text + "\033[0m";
    }

    inline std::string UnderlineText(const std::string &text)
    {
        return "\033[4m" + text + "\033[0m";
    }

    // Reset the console color.
    inline std::string ResetColor()
    {
        return "\033[0m";
    }

    // Print text in a given color.
    inline void PrintColored(const std::string &text, Color color = Color::DEFAULT)
    {
        std::cout << ColorToAnsi(color) << text << ResetColor();
    }

    // Print text in a given color followed by a newline.
    inline void PrintColoredLine(const std::string &text, Color color = Color::DEFAULT, bool endl = true)
    {
        PrintColored(text, color);
        if (endl)
            std::cout << std::endl;
    }

    // Print a horizontal line using a specified Unicode character.
    inline void PrintLine(std::string ch = "─", int length = 50, Color color = Color::DEFAULT)
    {
        std::string line{};
        for (size_t i = 0; i < length; i++)
            line += ch;
        
        PrintColoredLine(line, color, false);
    }

    // Print a styled title with a border made of Unicode box-drawing characters.
    inline void PrintTitle(const std::string &title, Color color = Color::CYAN)
    {
        int padding = 4;
        int titleLength = title.size();
        int totalWidth = titleLength + padding * 2;
        // Title line.
        std::cout << "\033[1m" << ColorToAnsi(color) << '*' << std::string(totalWidth, '=') << ' ' << title << ' ' << std::string(totalWidth, '=') << '*';
        std::cout << ResetColor() << '\n' << std::endl;
    }

    // Print a box around a given (possibly multi-line) text.
    inline void PrintBox(const std::string &text, const std::string &title = "", Color color = Color::GREEN)
    {
        std::istringstream iss(text);
        std::string line;
        int maxLength = 0;
        std::vector<std::string> lines;
        while (std::getline(iss, line))
        {
            lines.push_back(line);
            if (line.size() > maxLength)
                maxLength = line.size();
        }
        // Top border.
        std::cout << ColorToAnsi(color) << "┌─" << title;
        PrintLine("─", maxLength + 1 - title.size(), color);
        std::cout << ColorToAnsi(color) << "┐" << ResetColor() << std::endl;
        // Content lines with side borders.
        for (const auto &l : lines)
        {
            std::cout << ColorToAnsi(color) << "│ " << ResetColor();
            std::cout << l << std::string(maxLength - l.size(), ' ') 
                      << ColorToAnsi(color) << " │" << ResetColor() << std::endl;
        }
        // Bottom border.
        std::cout << ColorToAnsi(color) << "└";
        PrintLine("─", maxLength + 2, color);
       std::cout << ColorToAnsi(color) << "┘" << ResetColor() << std::endl;
    }

    // Print a success message with a check mark.
    inline void PrintSuccess(const std::string &message)
    {
        PrintColoredLine(u8"✔ | " + message, Color::GREEN);
    }

    // Print a warning message with a warning sign.
    inline void PrintWarning(const std::string &message)
    {
        PrintColoredLine(u8"⚠ | " + message, Color::YELLOW);
    }

    // Print an error message with a cross mark.
    inline void PrintError(const std::string &message)
    {
        PrintColoredLine(u8"✖ | " + message, Color::RED);
    }

    // Exit the program with an error message.
    inline void PrintErrorExit(const std::string &error, int code = -1)
    {
        PrintError(error);
        exit(code);
    }

    // Print an informational message with an info symbol.
    inline void PrintInfo(const std::string &message)
    {
        PrintColoredLine(u8"🛈 | " + message, Color::CYAN);
    }

    // Clear the console screen (works on Unix-like systems with ANSI support).
    inline void ClearConsole()
    {
        std::cout << "\033[2J\033[H";
    }

    inline void GetInput(const std::string &prompt, const std::string &title = "", Color color = Color::YELLOW)
    {
        PrintColored(prompt, color);
        std::cin.get();
    }

    inline bool GetBoolInput(const std::string &prompt, const std::string &title = "", Color color = Color::YELLOW, Color titleColor = Color::CYAN)
    {
        if (title != "")
            PrintTitle(title, titleColor);
        
        PrintColored(prompt + "[Y/n]", color);
        char in = std::cin.get();
        std::cout << std::endl;
        return in == 'Y' || in == 'y';
    }

    // Print a divider line with an optional centered message.
    inline void PrintDivider(const std::string &message = "", Color color = Color::DEFAULT)
    {
        int width = 50;
        if (message.empty())
        {
            PrintLine("─", width, color);
            std::cout << std::endl;
        }
        else
        {
            int pad = (width - message.size() - 2) / 2;
            std::cout << ColorToAnsi(color) << std::string(pad, ' ') 
                      << " " << message << " " 
                      << std::string(pad, ' ') << ResetColor() << std::endl;
        }
    }

    // Additional print functions and styling helpers can be added here as needed.
};

#endif // CANVAS__H
