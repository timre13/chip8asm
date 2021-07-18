#include "tokenizer.h"
#include "Logger.h"
#include "common.h"
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <utility>

namespace Tokenizer
{

OpcodeEnum opcodeStrToEnum(std::string opcode)
{
    opcode = strToLower(opcode);

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

RegisterEnum registerStrToEnum(std::string reg)
{
    reg = strToLower(reg);

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

bool isVRegister(RegisterEnum reg)
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

uint8_t vRegisterToByte(RegisterEnum reg)
{
    assert(isVRegister(reg));
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
                auto isSpace{
                    [](char c){
                        switch (c)
                        {
                        case ' ':
                        case '\f':
                        case '\n':
                        case '\r':
                        case '\t':
                        case '\v':
                        case ',':
                            return true;
                        default:
                            return false;
                        }
                    }
                };

                while (charI < line.length() && isSpace(line[charI]))
                    ++charI;
                while (charI < line.length() && !isSpace(line[charI]))
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
            TODO();
            continue;
        }

        OpcodeEnum opcode = opcodeStrToEnum(word);
        if (opcode != OPCODE_INVALID)
        {
            auto processOperand{ // -> bool: true if error happened, false otherwise
                [&filename = std::as_const(filename), &lineI = std::as_const(lineI)]
                    (const std::string& operandStr, OpcodeOperand& operand){
                    if (isComment(operandStr))
                        return false;

                    auto reg = registerStrToEnum(operandStr);
                    if (reg != REGISTER_INVALID) // A register name
                    {
                        operand.setRegister(reg);
                        Logger::dbg << "Register: " << reg << Logger::End;
                    }
                    else if (strToLower(operandStr).compare("f") == 0)
                    {
                        operand.setF();
                        Logger::dbg << "F operand" << Logger::End;
                    }
                    else if (strToLower(operandStr).compare("b") == 0)
                    {
                        operand.setB();
                        Logger::dbg << "B operand" << Logger::End;
                    }
                    else // Should be an integer constant
                    {
                        try
                        {
                            // TODO: Support binary base
                            int integer = std::stoi(operandStr, 0, 0);
                            if ((unsigned int)integer > 0x0fff)
                                throw std::out_of_range{"Value cannot fit in one byte."};
                            Logger::dbg << "Integer: " << integer << Logger::End;
                            operand.setUint(integer);
                        }
                        catch (std::out_of_range&)
                        {
                            Logger::fatal << filename << ':' << lineI << ": Integer literal is out of range: \"" << operandStr << '"' << Logger::End;
                            return true;
                        }
                        catch (std::invalid_argument&)
                        {
                            Logger::fatal << filename << ':' << lineI << ": Unknown value: \"" << operandStr << '"' << Logger::End;
                            return true;
                        }
                    }
                    return false;
                }
            };

            Logger::dbg << "Found an opcode: " << word << " = " << opcode << Logger::End;
            std::string operand0Str = getWord();
            std::string operand1Str = getWord();
            std::string operand2Str = getWord();
            Logger::dbg << "Operand 0: \"" << operand0Str << "\", operand 1: \"" << operand1Str << "\", operand 2: \"" << operand2Str << '"' << Logger::End;
            auto token = std::make_shared<Opcode>();
            token->opcode = opcode;
            if (operand0Str.size() && processOperand(operand0Str, token->operand0))
                continue;
            if (operand1Str.size() && processOperand(operand1Str, token->operand1))
                continue;
            if (operand2Str.size() && processOperand(operand2Str, token->operand2))
                continue;

            // Only LD can hava F and B as operands
            if (opcode != OPCODE_LD
             &&(token->operand0.getType() == Tokenizer::OpcodeOperand::Type::F
             || token->operand0.getType() == Tokenizer::OpcodeOperand::Type::B
             || token->operand1.getType() == Tokenizer::OpcodeOperand::Type::F
             || token->operand1.getType() == Tokenizer::OpcodeOperand::Type::B
             || token->operand2.getType() == Tokenizer::OpcodeOperand::Type::F
             || token->operand2.getType() == Tokenizer::OpcodeOperand::Type::B))
            {
                goto print_syntax_error;
            }

            token->setLineNumber(lineI);
            tokens.push_back(std::move(token));
            continue;
        }

print_syntax_error:
        Logger::fatal << filename << ':' << lineI << ": Syntax error: \"" << word << '"' << Logger::End;
    }

    return tokens;
}

} // namespace Tokenizer

