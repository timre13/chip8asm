#pragma once

#include <string>

class InputFile final
{
private:
    std::string m_buffer;
    bool m_isOpenFailed = false;

public:
    InputFile() {}

    void open(const std::string& filename);
    inline bool isOpenFailed() const { return m_isOpenFailed; }

    std::string getContent() const { return m_buffer; }
};

