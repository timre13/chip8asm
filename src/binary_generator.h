#pragma once

#include "tokenizer.h"

#include <stdint.h>
#include <vector>

class ByteList final : public std::vector<uint8_t>
{
public:
    inline void append8(uint8_t value) { this->push_back(value); }
    inline void append16(uint16_t value) { this->push_back(value >> 8); this->push_back(value & 0x0f); }
};

ByteList generateBinary(const Tokenizer::tokenList_t& tokens);

