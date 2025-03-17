#ifndef PROJECT__H
#define PROJECT__H

#include <string>
#include <vector>
#include "../dependencies/ConsoleLayout.h" // Assumed location for ConsoleLayout

class Project {
public:
    // Constructor
    Project(int bytes, const std::string &created_at, const std::string &created_by,
            const std::string &lang, const std::string &location, const std::string &name);

    // Getters
    int GetBytes() const { return m_bytes; }
    std::string GetCreatedAt() const { return m_createdAt; }
    std::string GetCreatedBy() const { return m_createdBy; }
    std::string GetLang() const { return m_lang; }
    std::string GetLocation() const { return m_location; }
    std::string GetName() const { return m_name; }

    // Table utilities
    std::vector<std::string> GetTableRow() const;
    static std::vector<std::string> GetTableHeader()
    {
        return std::vector<std::string>{"Created By", "Title", "Language", "Size (B)", "Created At"};
    }

private:
    int m_bytes;
    std::string m_createdAt;
    std::string m_createdBy;
    std::string m_lang;
    std::string m_location;
    std::string m_name;
};

// Implementation of constructor
Project::Project(int bytes, const std::string &created_at, const std::string &created_by, 
                 const std::string &lang, const std::string &location, const std::string &name)
    : m_bytes{bytes},
      m_createdAt{created_at},
      m_createdBy{created_by},
      m_lang{lang},
      m_location{location},
      m_name{name}
{
}

// Implementation of GetTableRow
std::vector<std::string> Project::GetTableRow() const {
    return {
        ConsoleLayout::CYAN + m_createdBy + ConsoleLayout::RESET,
        ConsoleLayout::CYAN + m_name + ConsoleLayout::RESET,
        ConsoleLayout::CYAN + m_lang + ConsoleLayout::RESET,
        ConsoleLayout::YELLOW + std::to_string(m_bytes) + ConsoleLayout::RESET,
        ConsoleLayout::YELLOW + m_createdAt + ConsoleLayout::RESET
    };
}

#endif // PROJECT__H
