#include "tokenizer.h"
#include "Logger.h"
#include <cctype>
#include <string>
#include <sstream>

namespace Tokenizer
{

OpcodeEnum opcodeStrToEnum(const std::string& opcode)
{
    // TODO: Is this UB?

    for (int en{}; en < OPCODE_INVALID; ++en)
    {
        if (opcode.compare(opcodeNames[en]) == 0)
        {
            return (OpcodeEnum)en;
        }
    }
    return OPCODE_INVALID;
}

RegisterEnum registerStrToEnum(const std::string& reg)
{
    // TODO: Is this UB?

    for (int en{}; en < REGISTER_INVALID; ++en)
    {
        if (reg.compare(registerNames[en]) == 0)
        {
            return (RegisterEnum)en;
        }
    }
    return REGISTER_INVALID;
}

bool isNumberedRegister(RegisterEnum reg)
{
    switch (reg)
    {
    case REGISTER_V0:
    case REGISTER_V1:
    case REGISTER_V2:
    case REGISTER_V3:
    case REGISTER_V4:
    case REGISTER_V5:
    case REGISTER_V6:
    case REGISTER_V7:
    case REGISTER_V8:
    case REGISTER_V9:
    case REGISTER_VA:
    case REGISTER_VB:
    case REGISTER_VC:
    case REGISTER_VD:
    case REGISTER_VE:
    case REGISTER_VF:
        return true;
    default:
        return false;
    }
}

uint8_t numberedRegisterToByte(RegisterEnum reg)
{
    assert(isNumberedRegister(reg));
    return (uint8_t)reg;
}

tokenList_t tokenize(const std::string& str, const::std::string& filename)
{
    tokenList_t tokens;
    
    std::stringstream ss;
    ss << str;
    size_t lineI = 0;
    std::string line;
    while (std::getline(ss, line))
    {
        ++lineI;

        if (line.empty())
            continue;

        size_t charI{};

        auto getWord{
            [&charI, &line](){
                std::string word;
                while (charI < line.length() && std::isspace(line[charI]))
                    ++charI;
                while (charI < line.length() && !std::isspace(line[charI]))
                    word += line[charI++];
                return word;
            }
        };
        std::string word = getWord();

        if (isComment(word))
            continue;

        Logger::dbg << "Word: " << '"' << word << '"' << Logger::End;

        if (isLabel(word))
        {
            Logger::dbg << "Found a label: \"" << word.substr(0, word.length()-1) << '"' << Logger::End;
            // XXX: Implement labels
            continue;
        }

        OpcodeEnum opcode = opcodeStrToEnum(strToLower(word));
        if (opcode != OPCODE_INVALID)
        {
            Logger::dbg << "Found an opcode: " << word << " = " << opcode << Logger::End;
            std::string operand0 = getWord();
            std::string operand1 = getWord();
            Logger::dbg << "Operand 0: \"" << operand0 << "\", operand 1: \"" << operand1 << '"' << Logger::End;
            auto token = std::make_shared<Opcode>();
            token->opcode = opcode;
            // XXX: Implement operands
            tokens.push_back(std::move(token));
            // XXX: Implement opcodes
            continue;
        }

        Logger::err << filename << ':' << lineI << ": Syntax error: \"" << word << "\"" << Logger::End;
    }

    return tokens;
}

} // namespace Tokenizer

