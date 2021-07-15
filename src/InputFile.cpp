#include "InputFile.h"

#include "Logger.h"

#include <string.h>
#include <exception>
#include <sstream>
#include <fstream>

void InputFile::open(const std::string& filename)
{
    Logger::dbg << "Reading file: " << filename << Logger::End;

    m_buffer.clear();
    try
    {
        std::ifstream file;
        file.open(filename);
        if (!file.is_open() || file.fail())
            throw std::runtime_error{strerror(errno)};

        std::stringstream ss;
        ss << file.rdbuf();
        m_buffer = ss.str();
    }
    catch (std::exception& e)
    {
        m_isOpenFailed = true;
        Logger::fatal << "Failed to read file: \"" << filename << "\": " << e.what() << Logger::End;
        return;
    }
    m_isOpenFailed = false;

    Logger::dbg << "File contents:\n" << m_buffer << "\n============= End of file contents =============" <<Logger::End;
}

