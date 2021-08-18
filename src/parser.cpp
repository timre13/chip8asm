#include "parser.h"
#include "Logger.h"
#include "common.h"
#include <cctype>
#include <cstdlib>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>
#include <utility>

namespace Parser
{

OpcodeEnum opcodeStrToEnum(std::string opcode)
{
    opcode = strToLower(opcode);
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
    if (reg.empty())
        return REGISTER_INVALID;

    reg = strToLower(reg);
    for (int en{}; en < REGISTER_INVALID; ++en)
    {
        if (reg.compare(registerNames[en]) == 0 || reg.compare(alternateVRegisterNames[en]) == 0)
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

uint8_t vRegisterToNibble(RegisterEnum reg)
{
    if (!isVRegister(reg))
        throw std::runtime_error{"Vx register expected, but got something else"};
    return uint8_t(reg & 0xf);
}

static char escapedCharToChar(char character, bool isString=false)
{
    if (isString && character == '"')
        return '"';
    else if (character == '\'')
        return '\'';

    switch (character)
    {
    case '0':  return '\0';
    case 'a':  return '\a';
    case 'b':  return '\b';
    case 't':  return '\t';
    case 'v':  return '\v';
    case 'f':  return '\f';
    case 'r':  return '\r';
    case 'n':  return '\n';
    case '\\': return '\\';
    default: throw std::runtime_error{""};
    }
}

static unsigned int stringToUint(const std::string& str, unsigned int limit)
{
    Logger::dbg << "Converting \"" + str + "\" to integer" << Logger::End;

    unsigned int integer{};
    if (str.length() > 2 && str[0] == '0' && str[1] == 'b')
    {
        for (size_t i{str.length()-1}; i >= 2; --i)
        {
            if (str[i] == '1')
            {
                integer |= 1 << ((str.length()-1)-i);
            }
            else if (str[i] != '0')
            {
                throw std::invalid_argument{"Invalid binary integer literal: "+str};
            }
        }
    }
    else if (str.length() == 3 && str[0] == '\'' && str[2] == '\'') // Normal character
    {
        if (str[1] == '\\')
            throw std::invalid_argument{"Spare '\\' in character literal"};
        integer = str[1];
    }
    else if (str.length() == 4 && str[0] == '\'' && str[1] == '\\' && str[3] == '\'') // Escaped character
    {
        try
        {
            integer = escapedCharToChar(str[2]);
        }
        catch (std::exception&)
        {
            throw std::invalid_argument{"Invalid escape sequence: "+str};
        }
    }
    else
    {
        try
        {
            integer = std::stoul(str, 0, 0);
        }
        catch (...)
        {
            throw std::runtime_error{"Integer conversion failed, value: "+str};
        }
    }
    if ((unsigned int)integer > limit)
        throw std::out_of_range{"Integer value \"" + str + "\" is out of range."};
    return integer;
}

static void processOperand(const std::string& operandStr, OpcodeOperand* operand)
{
    if (isComment(operandStr))
        return;

    auto reg = registerStrToEnum(operandStr);
    if (reg != REGISTER_INVALID) // A register name
    {
        operand->setRegister(reg);
        Logger::dbg << "Register: " << reg << Logger::End;
    }
    else if (strToLower(operandStr).compare("f") == 0)
    {
        operand->setF();
        Logger::dbg << "F operand" << Logger::End;
    }
    else if (strToLower(operandStr).compare("b") == 0)
    {
        operand->setB();
        Logger::dbg << "B operand" << Logger::End;
    }
    else if (strToLower(operandStr).compare("k") == 0)
    {
        operand->setK();
        Logger::dbg << "K operand" << Logger::End;
    }
    else if (std::isdigit(operandStr[0]) || operandStr[0] == '\'') // Probably an integer constant or a character
    {
        unsigned int integer = stringToUint(operandStr, 0x0fff);
        Logger::dbg << "Integer: " << integer << Logger::End;
        operand->setUint(integer);
    }
    else if (isValidLabelName(operandStr)) // Probably a label reference
    {
        Logger::dbg << "Label reference to \"" << operandStr << '"' << Logger::End;
        operand->setAsLabel(operandStr);
    }
    else
    {
        throw std::runtime_error{"Invalid operand value"};
    }
}

static std::string getWord(size_t& charI, const std::string& line)
{
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

    // Skip space
    while (charI < line.length() && isSpace(line[charI]))
        ++charI;

    bool isInsideSingleQuote{};
    bool isInsideDoubleQuote{};
    while (charI < line.size())
    {
        if (line.at(charI) == '\'' && charI > 0 && line.at(charI-1) != '\\')
        {
            if (isInsideSingleQuote)
            {
                word += line.at(charI++);
                break;
            }
            isInsideSingleQuote = true;
        }
        if (line.at(charI) == '"' && charI > 0 && line.at(charI-1) != '\\')
        {
            if (isInsideDoubleQuote)
            {
                word += line.at(charI++);
                break;
            }
            isInsideDoubleQuote = true;
        }

        // We break out if this is the end of the line or we found a space outside the quotes
        if (charI >= line.length() || ((isSpace(line.at(charI)) && !isInsideSingleQuote && !isInsideDoubleQuote)))
            break;
        word += line.at(charI++);
    }
    return word;
}

static inline std::string getWord(const std::string& line)
{
    size_t _{};
    return getWord(_, line);
}

static std::map<std::string, std::string> getMacroDefs(const std::string& str)
{
    std::map<std::string, std::string> output;

    std::stringstream ss; ss << str;
    std::string line;
    size_t lineI{};
    while (std::getline(ss, line))
    {
        ++lineI;
        if (line.empty())
            continue;

        if (isMacroDeclaration(line))
        {
            std::string macroName;
            {
                size_t i{7};
                // Skip space
                while (i < line.size() && isspace(line[i]))
                    i++;
                // Get macro name
                while (i < line.size() && !isspace(line[i]))
                    macroName += line[i++];
            }

            std::string macroVal;
            {
                size_t i{7};
                // Skip space
                while (i < line.size() && isspace(line[i]))
                    i++;
                // Skip macro name
                while (i < line.size() && !isspace(line[i]))
                    i++;
                // Skip space
                while (i < line.size() && isspace(line[i]))
                    i++;
                // Get remaining line
                macroVal = line.substr(i);
            }

            Logger::dbg << "Found a macro declaration: \"" << macroName
                << "\", value: \"" << macroVal << '"' << Logger::End;

            auto foundMacro = output.find(macroName);
            if (foundMacro != output.end())
            {
                Logger::warn << lineI << ":Macro redeclared: \"" << macroName << '"' << Logger::End;
            }
            output.insert({macroName, macroVal});
        }
    }
    return output;
}

std::string preprocessFile(const std::string& str, const::std::string& filename)
{
    const auto macroDefs = getMacroDefs(str);

    std::string output;
    // Remove preprocessor directives
    {
        std::stringstream ss;
        ss << str;
        std::string line;
        size_t lineI{};
        while (std::getline(ss, line))
        {
            ++lineI;
            if (line.empty())
            {
                output += '\n';
                continue;
            }

            if (line[0] == PREPRO_PREFIX_CHAR)
            {
                const std::string directive = getWord(line).substr(1);
                if (directive.compare("define") != 0)
                {
                    throw std::runtime_error{filename + ':' + std::to_string(lineI) + ':' + "Invalid preprocessor directive: " + line};
                }
                output += '\n';
                continue;
            }
            output += line + '\n';
        }
    }
    Logger::dbg << "Preprocessed file (stage 1):\n" << output << Logger::End;

    // Replace macros with their value
    for (const auto& macro : macroDefs)
    {
        const auto& from = macro.first;
        const auto& to = macro.second;

        size_t foundPos = output.find(from);
        if (foundPos != std::string::npos && to.empty())
        {
            throw std::runtime_error{filename + ": Invalid use of empty macro \"" + from + '"'};
        }
        while (foundPos != std::string::npos)
        {
            Logger::dbg << "Replacing macro \"" << from << "\" with \"" << to << '"' << Logger::End;
            output.replace(foundPos, from.size(), to);
            foundPos = output.find(from);
        }
    }
    Logger::dbg << "Preprocessed file (stage 2):\n" << output << Logger::End;
    return output;
}


void parseTokens(
        const std::string& str, const::std::string& filename,
        tokenList_t* tokenList, labelMap_t* labelMap)
{
    std::stringstream ss;
    ss << str;
    size_t lineI{};
    std::string line;
    uint16_t byteOffset{};
    while (std::getline(ss, line))
    {
        try
        {
            ++lineI;

            if (line.empty())
                continue;

            size_t charI{};

            std::string word = getWord(charI, line);

            // TODO: Refactor this whole block

            if (isComment(word))
                continue;
            Logger::dbg << "Word: " << '"' << word << '"' << Logger::End;

            if (isLabelDeclaration(word))
            {
                Logger::dbg << "Found a label declaration: \"" << word.substr(0, word.length()-1)
                    << "\", offset: 0x" << std::hex << byteOffset << std::dec << Logger::End;

                auto foundLabel = labelMap->find(word);
                if (foundLabel != labelMap->end())
                {
                    throw std::runtime_error{"Label redeclared: \"" + word
                        + "\", original offset: 0x" + intToHexStr(foundLabel->second) +
                        ", new offset: 0x" + std::to_string(byteOffset)};
                }
                labelMap->insert({word.substr(0, word.length()-1), byteOffset});
                continue;
            }

            OpcodeEnum opcode = opcodeStrToEnum(word);
            std::string lowerWord = strToLower(word);
            if (opcode != OPCODE_INVALID)
            {
                Logger::dbg << "Found an opcode: " << word << " = " << opcode << Logger::End;
                std::string operand0Str = getWord(charI, line);
                std::string operand1Str = getWord(charI, line);
                std::string operand2Str = getWord(charI, line);
                Logger::dbg << "Operand 0: \"" << operand0Str
                    << "\", operand 1: \"" << operand1Str
                    << "\", operand 2: \"" << operand2Str
                    << '"' << Logger::End;
                auto token = std::make_shared<Opcode>();
                token->opcode = opcode;

                // Don't try to parse comment after opcode as operands
                bool hasCommentStarted = false;

                if (operand0Str.size())
                {
                    if (isComment(operand0Str))
                    {
                        hasCommentStarted = true;
                    }
                    else
                    {
                        processOperand(operand0Str, &token->operand0);
                    }
                }

                if (operand1Str.size() && !hasCommentStarted)
                {
                    if (isComment(operand1Str))
                    {
                        hasCommentStarted = true;
                    }
                    else
                    {
                        processOperand(operand1Str, &token->operand1);
                    }
                }

                if (operand2Str.size() && !hasCommentStarted)
                {
                    if (isComment(operand2Str))
                    {
                        hasCommentStarted = true;
                    }
                    else
                    {
                        processOperand(operand2Str, &token->operand2);
                    }
                }

                // Possible variants:
                // LD:
                //   with F or B as first argument
                //   or with K as second argument
                if ((opcode != OPCODE_LD && (
                    token->operand0.getType() == Parser::OpcodeOperand::Type::F
                 || token->operand0.getType() == Parser::OpcodeOperand::Type::B
                 || token->operand0.getType() == Parser::OpcodeOperand::Type::K
                 || token->operand1.getType() == Parser::OpcodeOperand::Type::F
                 || token->operand1.getType() == Parser::OpcodeOperand::Type::B
                 || token->operand1.getType() == Parser::OpcodeOperand::Type::K
                 || token->operand2.getType() == Parser::OpcodeOperand::Type::F
                 || token->operand2.getType() == Parser::OpcodeOperand::Type::B
                 || token->operand2.getType() == Parser::OpcodeOperand::Type::K))
                 || (opcode == OPCODE_LD && (
                    token->operand1.getType() == Parser::OpcodeOperand::Type::F
                 || token->operand1.getType() == Parser::OpcodeOperand::Type::B
                 || token->operand2.getType() == Parser::OpcodeOperand::Type::F
                 || token->operand2.getType() == Parser::OpcodeOperand::Type::B
                 || token->operand0.getType() == Parser::OpcodeOperand::Type::K
                 || token->operand2.getType() == Parser::OpcodeOperand::Type::K)))
                {
                    throw std::runtime_error{"Invalid use of F/B/K operator"};
                }

                token->setLineNumber(lineI);
                tokenList->push_back(std::move(token));
                byteOffset += 2;
                continue;
            }
            else if (lowerWord.compare("db") == 0) // Define byte
            {
                Logger::dbg << "Found a byte definition" << Logger::End;;

                auto def = std::make_shared<DbInst>();
                std::string word;
                while (true)
                {
                    word = getWord(charI, line);
                    if (word.empty() || isComment(word))
                        break;

                    Logger::dbg << "DB argument: " << word << Logger::End;

                    if (word.size() > 1 && word[0] == '"' && word[word.size()-1] == '"' && word[word.size()-2] != '\\')
                    {
                        for (size_t i{1}; i < word.size()-1; ++i)
                        {
                            if (word[i] == '\\') // Escaped character
                            {
                                try
                                {
                                    def->arguments.push_back(escapedCharToChar(word[++i], true));
                                }
                                catch (std::exception&)
                                {
                                    throw std::invalid_argument{"Invalid escape sequence"};
                                }
                            }
                            else
                            {
                                def->arguments.push_back(word[i]);
                            }
                        }
                    }
                    else
                    {
                        def->arguments.push_back(stringToUint(word, 255));
                    }
                }

                if (def->arguments.empty())
                {
                    Logger::warn << "DB without data" << Logger::End;
                }
                byteOffset += def->arguments.size();
                tokenList->push_back(std::move(def));
                continue;
            }
            else if (lowerWord.compare("dw") == 0) // Define word
            {
                Logger::dbg << "Found a word definition" << Logger::End;;

                auto def = std::make_shared<DwInst>();
                std::string word;
                while (true)
                {
                    word = getWord(charI, line);
                    if (word.empty() || isComment(word))
                        break;
                    def->arguments.push_back(stringToUint(word, 0xffff));
                }

                if (def->arguments.empty())
                {
                    Logger::warn << "DW without data" << Logger::End;
                }
                byteOffset += def->arguments.size() * 2;
                tokenList->push_back(std::move(def));
                continue;
            }

            throw std::runtime_error{"Syntax error: "+line};
        }
        catch (std::exception& e)
        {
            // Rethrow the exception with more info
            throw std::runtime_error{filename + ':' + std::to_string(lineI) + ": " + e.what()};
        }
    }
}

} // namespace Parser

