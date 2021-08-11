#pragma once

#include <string>

class InputFile final
{
private:
    std::string m_filePath;
    std::string m_buffer;

public:
    InputFile() {}

    void open(const std::string& filePath);

    const std::string& getContent() const { return m_buffer; }
    const std::string& getFilePath() const { return m_filePath; }
};

