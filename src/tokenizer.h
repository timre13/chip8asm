#pragma once

#include <stdint.h>
#include <cassert>

enum OpcodeEnum
{
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
};

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
};

class OpcodeOperand final
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

struct Label
{
    uint16_t address : 13;
};

struct Opcode
{
    OpcodeEnum opcode;
    OpcodeOperand operand0;
    OpcodeOperand operand1;
};

