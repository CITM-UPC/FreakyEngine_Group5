#ifndef CONSOLE_H
#define CONSOLE_H

#include <vector>
#include <string>

class Console
{
public:
    static Console& Instance()
    {
        static Console instance;
        return instance;
    }

    void Log(const std::string& message);
    void Clear();
    const std::vector<std::string>& GetMessages() const;

private:

    Console() = default; // Constructor is private.
    ~Console() = default; // Destructor is private.
    Console(const Console&) = delete; // Prevent copy-construction.
    Console& operator=(const Console&) = delete; // Prevent assignment.

    std::vector<std::string> messages;
};

#endif // CONSOLE_H
