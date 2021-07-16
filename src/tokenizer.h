#pragma once

#include <cctype>
#include <stdint.h>
#include <cassert>
#include <string>
#include <vector>
#include <memory>

namespace Tokenizer
{

/*
 * Base class for all the things that can appear in the code.
 */
class Token
{
};

using tokenList_t = std::vector<std::shared_ptr<Token>>;

//------------------------------------ Opcode ----------------------------------

enum OpcodeEnum
{
    OPCODE_CLS,
    OPCODE_RET,
    OPCODE_JP,
    OPCODE_CALL,
    OPCODE_SE,
    OPCODE_SNE,
    OPCODE_LD,
    OPCODE_ADD,
    OPCODE_OR,
    OPCODE_AND,
    OPCODE_XOR,
    OPCODE_SUB,
    OPCODE_SHR,
    OPCODE_SUBN,
    OPCODE_SHL,
    OPCODE_RND,
    OPCODE_DRW,
    OPCODE_SKP,
    OPCODE_SKNP,
    OPCODE_INVALID,
};

constexpr const char* const opcodeNames[] = {
    "cls",
    "ret",
    "jp",
    "call",
    "se",
    "sne",
    "ld",
    "add",
    "or",
    "and",
    "xor",
    "sub",
    "shr",
    "subn",
    "shl",
    "rnd",
    "drw",
    "skp",
    "sknp",
};

[[nodiscard]] OpcodeEnum opcodeStrToEnum(const std::string& opcode);

enum RegisterEnum
{
    REGISTER_V0,
    REGISTER_V1,
    REGISTER_V2,
    REGISTER_V3,
    REGISTER_V4,
    REGISTER_V5,
    REGISTER_V6,
    REGISTER_V7,
    REGISTER_V8,
    REGISTER_V9,
    REGISTER_VA,
    REGISTER_VB,
    REGISTER_VC,
    REGISTER_VD,
    REGISTER_VE,
    REGISTER_VF,
    REGISTER_SP,
    REGISTER_DT,
    REGISTER_ST,
    REGISTER_I,
    REGISTER_INVALID,
};

constexpr const char* const registerNames[] = {
    "v0",
    "v1",
    "v2",
    "v3",
    "v4",
    "v5",
    "v6",
    "v7",
    "v8",
    "v9",
    "va",
    "vb",
    "vc",
    "vd",
    "ve",
    "vf",
    "sp",
    "dt",
    "st",
    "i",
};
[[nodiscard]] RegisterEnum registerStrToEnum(const std::string& reg);
[[nodiscard]] bool isNumberedRegister(RegisterEnum reg);
[[nodiscard]] uint8_t numberedRegisterToByte(RegisterEnum reg);

class OpcodeOperand
{
private:
    enum class Type
    {
        Empty,
        Byte,
        Nibble,
        Register,
    } m_type;

    union
    {
        uint8_t m_byte = 0;
        uint8_t m_nibble;
        RegisterEnum m_register;
    };

    inline Type getType() const { return m_type; }
    inline uint8_t getAsByte() const { assert(m_type == Type::Byte); return m_byte; }
    inline uint8_t getAsNibble() const { assert(m_type == Type::Nibble); return m_nibble & 0x0f; }
    inline RegisterEnum getAsRegister() const { assert(m_type == Type::Register); return m_register; }
};

class Opcode final : public Token
{
public:
    OpcodeEnum opcode;
    OpcodeOperand operand0;
    OpcodeOperand operand1;
};

//--------------------------------- Label --------------------------------------

class Label final : public Token
{
public:
    std::string name;
    uint16_t address : 13;
};

//--------------------------------- Macro --------------------------------------

enum MacroType
{
    MACROTYPE_INCLUDE,
    MACROTYPE_DEFINE,
};

class Macro final : public Token
{
public:
    MacroType type;
    std::string argument;
};

//------------------------------------------------------------------------------

[[nodiscard]] inline bool isComment(const std::string& str) { return str[0] == ';'; }
[[nodiscard]] inline bool isLabel(const std::string& str) { return str[str.length()-1] == ':'; }
[[nodiscard]] inline std::string strToLower(std::string str)
{
    for (size_t i{}; i < str.size(); ++i)
        str[i] = std::tolower(str[i]);
    return str;
}

tokenList_t tokenize(const std::string& str, const::std::string& filename);

} // namespace Tokenizer

