#pragma once

#include <cctype>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include "Logger.h"

#define PREPRO_PREFIX_CHAR '%'

namespace Parser
{

/*
 * Base class for all the things that can appear in the assembly code.
 */
class Token
{
private:
    int m_lineNumber{};

public:
    virtual inline void setLineNumber(int value) { m_lineNumber = value; }
    virtual inline int getLineNumber() const { return m_lineNumber; }
    virtual inline std::string getLineNumberStr() const { return m_lineNumber > 0 ? std::to_string(m_lineNumber) : "?"; }

    virtual inline ~Token(){}
};

using tokenList_t = std::vector<std::shared_ptr<Token>>;

//--------------------------------- Label --------------------------------------

class LabelReference final : public Token
{
public:
    std::string name;
};
[[nodiscard]] inline bool isValidLabelName(const std::string& str)
{
    if (isdigit(str[0]))
            return false;
    for (size_t i{}; i < str.size(); ++i)
    {
        if (!std::isalnum(str[i]) && str[i] != '_')
        {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool isLabelDeclaration(const std::string& str)
{
    return str[str.length()-1] == ':' && isValidLabelName(str.substr(0, str.size()-1));
}
//                          V - name     V - offset
using labelMap_t = std::map<std::string, uint16_t>;

//------------------------------ Macro definition ------------------------------

[[nodiscard]] inline bool isMacroDeclaration(const std::string& str)
{
    const std::string defineStr = PREPRO_PREFIX_CHAR+std::string("define");
    if (str.substr(0, 7).compare(defineStr) != 0)
        return false;

    size_t macroNameStart = 7;
    while (macroNameStart < str.size() && isspace(str[macroNameStart]))
        ++macroNameStart;

    // First character in the name can't be a digit
    if (isdigit(str[macroNameStart]))
        goto malformed;

    for (size_t i{macroNameStart+1}; i < str.size(); ++i)
    {
        if (isspace(str[i]))
            break;
        if (!std::isalnum(str[i]) && str[i] != '_')
        {
            goto malformed;
        }
    }
    return true;

malformed:
    throw std::runtime_error{"Invalid macro name"};
    return false; // Make the compiler happy
}

//------------------------------------ Opcode ----------------------------------

enum OpcodeEnum
{
    OPCODE_NOP,
    OPCODE_SYS,
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
    "nop",
    "sys",
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

[[nodiscard]] OpcodeEnum opcodeStrToEnum(std::string opcode);

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
    REGISTER_I,
    REGISTER_I_ADDR,
    REGISTER_DT,
    REGISTER_ST,
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
    "i",
    "[i]",
    "dt",
    "st",
};

constexpr const char* const alternateVRegisterNames[] = {
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
    "v10",
    "v11",
    "v12",
    "v13",
    "v14",
    "v15",
    "",
    "",
    "",
    "",
};
[[nodiscard]] RegisterEnum registerStrToEnum(std::string reg);
[[nodiscard]] bool isVRegister(RegisterEnum reg);
[[nodiscard]] uint8_t vRegisterToNibble(RegisterEnum reg);

class OpcodeOperand
{
public:
    enum class Type
    {
        Empty,
        Uint,           // Byte (8 bits), nibble (4 bits) or address (12 bits)
        Register,       // Register
        LabelReference, // A label is used
        F,              // Used by LD
        B,              // Used by LD
        K,              // Used by LD
    };

private:
    Type m_type = Type::Empty;
    uint16_t m_uint = 0;
    RegisterEnum m_vRegister;
    LabelReference m_label;

public:
    inline OpcodeOperand() {}

    inline Type getType() const { return m_type; }
    inline std::string getTypeStr() const
    {
        switch (m_type)
        {
        case Type::Empty: return "Empty";
        case Type::Uint: return "Integer";
        case Type::Register: return "Register";
        case Type::LabelReference: return "Label";
        case Type::F: return "Sprite Operator (F)";
        case Type::B: return "BCD Operator (B)";
        case Type::K: return "Key Operator (K)";
        }
    }

    inline uint16_t getAsUint() const
    {
        if (m_type != Type::Uint)
            throw std::runtime_error{"Unexpected type of operand. Expected Integer, got "+getTypeStr()};
        return m_uint;
    }

    inline RegisterEnum getAsRegister() const
    {
        if (m_type != Type::Register)
            throw std::runtime_error{"Unexpected type of operand. Expected Register, got "+getTypeStr()};
        return m_vRegister;
    }

    inline LabelReference getAsLabel() const
    {
        if (m_type != Type::LabelReference)
            throw std::runtime_error{"Unexpected type of operand. Expected Label, got "+getTypeStr()};
        return m_label;
    }

    inline void setUint(uint16_t value) { m_uint = value; m_type = Type::Uint; }
    inline void setRegister(RegisterEnum reg) { m_vRegister = reg; m_type = Type::Register; }
    inline void setF() { m_type = Type::F; }
    inline void setB() { m_type = Type::B; }
    inline void setK() { m_type = Type::K; }
    inline void setAsLabel(const std::string& labelName) { m_label.name = labelName; m_type = Type::LabelReference; }

    virtual inline ~OpcodeOperand(){}
};

class Opcode final : public Token
{
public:
    OpcodeEnum opcode;
    OpcodeOperand operand0;
    OpcodeOperand operand1;
    OpcodeOperand operand2;

    inline Opcode() {}
};

//------------------------------------------------------------------------------

template <typename T>
class DataStoreInst : public Token
{
public:
    std::vector<T> arguments;
};

// Define Byte
using DbInst = DataStoreInst<uint8_t>;
// Define Word
using DwInst = DataStoreInst<uint16_t>;

//------------------------------------------------------------------------------

[[nodiscard]] inline bool isComment(const std::string& str) { return str[0] == ';'; }

//------------------------------------------------------------------------------

/*
 * Handles the preprocessor macros.
 *
 * Throws on error.
 */
std::string preprocessFile(const std::string &str, const::std::string& filename);

/*
 * Transforms the string into a vector of tokens.
 *
 * Throws on error.
 */
void parseTokens(
        const std::string& str, const::std::string& filename,
        tokenList_t* tokenList, labelMap_t* labelMap);

} // namespace Parser

