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

class LabelDeclaration final : public Token
{
public:
    std::string name;
    uint16_t address;
};
[[nodiscard]] inline bool isLabelDeclaration(const std::string& str)
{
    return str[str.length()-1] == ':' && isValidLabelName(str.substr(0, str.size()-1));
}

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
    union
    {
        uint16_t m_uint = 0;
        RegisterEnum m_vRegister;
        LabelReference m_label;
    };

public:
    inline OpcodeOperand() {}

    inline Type getType() const { return m_type; }
    inline uint16_t getAsUint() const { assert(m_type == Type::Uint); return m_uint; }
    inline RegisterEnum getAsRegister() const { assert(m_type == Type::Register); return m_vRegister; }
    inline LabelReference getAsLabel() const { assert(m_type == Type::LabelReference); return m_label; }

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

[[nodiscard]] inline bool isComment(const std::string& str) { return str[0] == ';'; }
[[nodiscard]] inline std::string strToLower(std::string str)
{
    for (size_t i{}; i < str.size(); ++i)
        str[i] = std::tolower(str[i]);
    return str;
}

tokenList_t tokenize(const std::string& str, const::std::string& filename);

} // namespace Tokenizer

