#pragma once

#include "parser.h"
#include "Logger.h"
#include <stdint.h>
#include <vector>

class ByteList final : public std::vector<uint8_t>
{
public:
    inline void append8(uint8_t value)
    {
        this->push_back(value);
        Logger::dbg << "Wrote 0x" << std::hex << +value << std::dec << " to output buffer" << Logger::End;
    }

    inline void append16(uint16_t value)
    {
        this->push_back(value >> 8);
        this->push_back(value & 0xff);
        Logger::dbg << "Wrote 0x" << std::hex << value << std::dec << " to output buffer" << Logger::End;
    }
};

ByteList generateBinary(const Parser::tokenList_t& tokens, const Parser::labelMap_t& labels);

