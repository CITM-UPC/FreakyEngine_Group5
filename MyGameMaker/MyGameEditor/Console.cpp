#include "Console.h"

void Console::Log(const std::string& message)
{
    messages.push_back(message);
}

void Console::Clear()
{
    messages.clear();
}

const std::vector<std::string>& Console::GetMessages() const
{
    return messages;
}
