#ifndef CONSOLE_LAYOUT_H
#define CONSOLE_LAYOUT_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>

namespace ConsoleLayout {

// ANSI escape codes for colors and styles
const std::string RESET      = "\033[0m";
const std::string RED        = "\033[31m";
const std::string GREEN      = "\033[32m";
const std::string YELLOW     = "\033[33m";
const std::string BLUE       = "\033[34m";
const std::string MAGENTA    = "\033[35m";
const std::string CYAN       = "\033[36m";
const std::string WHITE      = "\033[37m";

const std::string BOLD       = "\033[1m";
const std::string UNDERLINE  = "\033[4m";

std::string StripAnsiCodes(const std::string& str) {
    static const std::regex ansiPattern("\033\\[[0-9;]*m"); // Matches ANSI escape codes
    return std::regex_replace(str, ansiPattern, "");
}

// Prints a header with a title
inline void PrintHeader(const std::string& title) {
    std::cout << BOLD << BLUE << "===== " << title << " =====" << RESET << std::endl;
}

// Prints a footer with a title
inline void PrintFooter(const std::string& title) {
    std::cout << BOLD << BLUE << "===== " << title << " =====" << RESET << std::endl;
}

// Prints an error message in bold red
inline void PrintError(const std::string& message) {
    std::cout << BOLD << RED << "Error: " << message << RESET << std::endl;
}

// Prints a warning message in bold yellow
inline void PrintWarning(const std::string& message) {
    std::cout << BOLD << YELLOW << "Warning: " << message << RESET << std::endl;
}

// Prints an informational message in green
inline void PrintInfo(const std::string& message) {
    std::cout << GREEN << "Info: " << message << RESET << std::endl;
}

// Prints a plain line of text
inline void PrintLine(const std::string& text) {
    std::cout << text << std::endl;
}

// Prints a divider line with a given character and width
inline void PrintDivider(char c = '-', int width = 50) {
    std::cout << std::string(width, c) << std::endl;
}

// Prints a text inside a box drawn with a specified character (default '*')
inline void PrintBox(const std::string& text, char borderChar = '*') {
    int width = static_cast<int>(text.length()) + 4;
    PrintDivider(borderChar, width);
    std::cout << borderChar << " " << text << " " << borderChar << std::endl;
    PrintDivider(borderChar, width);
}



inline void PrintUnicodeBox(const std::string& text) {
    std::string cleanText = StripAnsiCodes(text); // Remove ANSI escape codes for length calculation
    int width = static_cast<int>(cleanText.length()) + 2; // Padding for spacing

    // Unicode box-drawing characters
    const std::string topLeft = "┌";
    const std::string topRight = "┐";
    const std::string bottomLeft = "└";
    const std::string bottomRight = "┘";
    const std::string horizontal = "─";
    const std::string vertical = "│";

    // Print top border
    std::cout << topLeft;
    for (size_t i = 0; i < width; i++)
        std::cout << horizontal;
    std::cout << topRight << std::endl;

    // Print text line
    std::cout << vertical << " " << text << " " << vertical << std::endl;

    // Print bottom border
    std::cout << bottomLeft;
    for (size_t i = 0; i < width; i++)
        std::cout << horizontal;
    std::cout << bottomRight << std::endl;
}

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

// Helper function to compute the printable width of a string, ignoring ANSI escape sequences.
inline size_t displayWidth(const std::string& s) {
    size_t width = 0;
    bool inEscape = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (!inEscape) {
            if (c == '\033') {  // ANSI escape sequence starts with ESC (0x1B)
                inEscape = true;
            } else {
                width++;
            }
        } else {
            // Assuming ANSI escape sequences end with 'm'
            if (c == 'm') {
                inEscape = false;
            }
        }
    }
    return width;
}

// Prints a table with Unicode borders.
// The header is a vector of strings representing the column titles.
// Each row is a vector of strings representing a table row.
inline void PrintTableUnicode(const std::vector<std::string>& header,
                              const std::vector<std::vector<std::string>>& rows) {
    size_t cols = header.size();
    // Compute the maximum display width for each column.
    std::vector<size_t> colWidths(cols, 0);
    
    // Initialize column widths using header printable lengths.
    for (size_t i = 0; i < cols; ++i) {
        colWidths[i] = displayWidth(header[i]);
    }
    
    // Update column widths based on each cell's printable width.
    for (const auto& row : rows) {
        for (size_t i = 0; i < std::min(cols, row.size()); ++i) {
            colWidths[i] = std::max(colWidths[i], displayWidth(row[i]));
        }
    }
    
    // Lambda to print a horizontal line of the given character.
    auto printLine = [](size_t length, const std::string& ch) {
        for (size_t i = 0; i < length; ++i)
            std::cout << ch;
    };

    // Lambda to print the top border.
    auto printTopBorder = [&]() {
        std::cout << "┌";
        for (size_t i = 0; i < cols; ++i) {
            printLine(colWidths[i] + 2, "─");
            std::cout << (i < cols - 1 ? "┬" : "┐");
        }
        std::cout << "\n";
    };

    // Lambda to print the middle border (separating header from rows).
    auto printMiddleBorder = [&]() {
        std::cout << "├";
        for (size_t i = 0; i < cols; ++i) {
            printLine(colWidths[i] + 2, "─");
            std::cout << (i < cols - 1 ? "┼" : "┤");
        }
        std::cout << "\n";
    };

    // Lambda to print the bottom border.
    auto printBottomBorder = [&]() {
        std::cout << "└";
        for (size_t i = 0; i < cols; ++i) {
            printLine(colWidths[i] + 2, "─");
            std::cout << (i < cols - 1 ? "┴" : "┘");
        }
        std::cout << "\n";
    };
    
    // Print the table.
    printTopBorder();
    
    // Print the header row with proper padding.
    std::cout << "│";
    for (size_t i = 0; i < cols; ++i) {
        std::cout << " " << header[i];
        size_t pad = colWidths[i] - displayWidth(header[i]);
        std::cout << std::string(pad, ' ') << " │";
    }
    std::cout << "\n";
    
    // Print the border separating header from rows.
    printMiddleBorder();
    
    // Print each row.
    for (const auto& row : rows) {
        std::cout << "│";
        for (size_t i = 0; i < cols; ++i) {
            std::string cell = (i < row.size() ? row[i] : "");
            std::cout << " " << cell;
            size_t pad = colWidths[i] - displayWidth(cell);
            std::cout << std::string(pad, ' ') << " │";
        }
        std::cout << "\n";
    }
    
    // Print the bottom border.
    printBottomBorder();
}


} // namespace ConsoleLayout

#endif // CONSOLE_LAYOUT_H
