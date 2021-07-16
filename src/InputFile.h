#pragma once

#include <string>

class InputFile final
{
private:
    std::string m_filePath;
    std::string m_buffer;
    bool m_isOpenFailed = false;

public:
    InputFile() {}

    void open(const std::string& filePath);
    inline bool isOpenFailed() const { return m_isOpenFailed; }

    const std::string& getContent() const { return m_buffer; }
    const std::string& getFilePath() const { return m_filePath; }
};

