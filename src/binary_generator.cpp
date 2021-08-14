#include "binary_generator.h"
#include "Logger.h"
#include "common.h"
#include <utility>
#include <map>
#include <cassert>

#define ROM_LOAD_OFFSET 0x200

using labelMap_t = std::map<std::string, uint16_t>;

// Clangd can't find as_const()
#ifndef NOT_CLANGD
namespace std { template <typename T> add_const_t<T>& as_const(T&) noexcept; }
#endif

static void handleOpcode(const Parser::Opcode* opcode, ByteList& output, const labelMap_t& labels)
{
    auto printErrorIfWrongNumOfOps{
        [&opcode = std::as_const(opcode)](int maxArgs){
            bool shouldPrintError = false;
            assert(maxArgs >= 0 && maxArgs <= 3);
            assert(opcode->opcode != Parser::OPCODE_INVALID);
            switch (maxArgs)
            {
            case 0:
                if (opcode->operand0.getType() != Parser::OpcodeOperand::Type::Empty)
                    shouldPrintError = true;
                break;

            case 1:
                if (opcode->operand0.getType() == Parser::OpcodeOperand::Type::Empty
                 || opcode->operand1.getType() != Parser::OpcodeOperand::Type::Empty)
                    shouldPrintError = true;
                break;

            case 2:
                if (opcode->operand0.getType() == Parser::OpcodeOperand::Type::Empty
                 || opcode->operand1.getType() == Parser::OpcodeOperand::Type::Empty
                 || opcode->operand2.getType() != Parser::OpcodeOperand::Type::Empty)
                    shouldPrintError = true;
                break;

            case 3:
                if (opcode->operand0.getType() == Parser::OpcodeOperand::Type::Empty
                 || opcode->operand1.getType() == Parser::OpcodeOperand::Type::Empty
                 || opcode->operand2.getType() == Parser::OpcodeOperand::Type::Empty)
                    shouldPrintError = true;
                break;
            }
            if (shouldPrintError)
                throw std::runtime_error{"Invalid number of arguments for opcode: "
                    + std::string{Parser::opcodeNames[opcode->opcode]}
                    + ", expected " + std::to_string(maxArgs)};
        }
    };

    auto getLabelAddress{
        [&labels](const std::string& name){
            auto it = labels.find(name);
            if (it == labels.end())
                throw std::runtime_error{"Reference to undefined label: " + name};
            return ROM_LOAD_OFFSET + it->second;
        }
    };

    Logger::dbg << "Opcode: " << opcode->opcode << Logger::End;
    switch (opcode->opcode)
    {
    case Parser::OPCODE_NOP:
        printErrorIfWrongNumOfOps(0);
        output.append16(0x0000);
        break;

    case Parser::OPCODE_SYS:
        if (opcode->operand0.getType() == Parser::OpcodeOperand::Type::Uint)
        {
            output.append16(0x0000 |
                    (opcode->operand0.getAsUint() & 0x0fff));
        }
        else
        {
            output.append16(0x0000);
        }
        break;

    case Parser::OPCODE_CLS:
        printErrorIfWrongNumOfOps(0);
        output.append16(0x00e0);
        break;

    case Parser::OPCODE_RET:
        printErrorIfWrongNumOfOps(0);
        output.append16(0x00ee);
        break;

    case Parser::OPCODE_JP:
        switch (opcode->operand0.getType())
        {
        case Parser::OpcodeOperand::Type::Empty:
            throw std::runtime_error{"JP opcode requires operand(s)"};

        case Parser::OpcodeOperand::Type::Register: // JP V0, addr
            printErrorIfWrongNumOfOps(2);
            if (opcode->operand0.getAsRegister() != Parser::REGISTER_V0)
                throw std::runtime_error{"Register-relative jump is only possible with register V0"};
            if (opcode->operand1.getType() == Parser::OpcodeOperand::Type::Uint)
            {
                output.append16(0xb000 |
                        (opcode->operand1.getAsUint() & 0x0fff));
            }
            else if (opcode->operand1.getType() == Parser::OpcodeOperand::Type::LabelReference)
            {
                output.append16(0xb000 |
                        (getLabelAddress(opcode->operand0.getAsLabel().name) & 0x0fff));
            }
            else
            {
                throw std::runtime_error{"JP V0 requires an address"};
            }
            break;

        case Parser::OpcodeOperand::Type::Uint: // JP addr
            printErrorIfWrongNumOfOps(1);
            output.append16(0x1000 |
                    (opcode->operand0.getAsUint() & 0x0fff));
            break;

        case Parser::OpcodeOperand::Type::LabelReference: // JP addr
            printErrorIfWrongNumOfOps(1);
            output.append16(0x1000 |
                    (getLabelAddress(opcode->operand0.getAsLabel().name) & 0x0fff));
            break;

        case Parser::OpcodeOperand::Type::F:
        case Parser::OpcodeOperand::Type::B:
        case Parser::OpcodeOperand::Type::K:
            // Already handled
            break;
        }
        break;

    case Parser::OPCODE_CALL:
        printErrorIfWrongNumOfOps(1);
        if (opcode->operand0.getType() == Parser::OpcodeOperand::Type::Uint)
        {
            output.append16(0x2000 |
                    (opcode->operand0.getAsUint() & 0x0fff));
        }
        else if (opcode->operand0.getType() == Parser::OpcodeOperand::Type::LabelReference)
        {
            output.append16(0x2000 |
                    (getLabelAddress(opcode->operand0.getAsLabel().name) & 0x0fff));
        }
        else
        {
            throw std::runtime_error{"CALL opcode requires a constant value"};
        }
        break;

    case Parser::OPCODE_SE:
        printErrorIfWrongNumOfOps(2);
        if (opcode->operand0.getType() != Parser::OpcodeOperand::Type::Register)
            throw std::runtime_error{"SE opcode requires a register name as left argument"};
        if (opcode->operand1.getType() == Parser::OpcodeOperand::Type::Uint) // SE Vx, byte
        {
            output.append16(0x3000 |
                    (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                    (opcode->operand1.getAsUint() & 0xff));
        }
        else // SE Vx, Vy
        {
            output.append16(0x5000 |
                    (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                    Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4);
        }
        break;

    case Parser::OPCODE_SNE:
        printErrorIfWrongNumOfOps(2);
        if (opcode->operand0.getType() != Parser::OpcodeOperand::Type::Register)
            throw std::runtime_error{"SNE opcode requires a register name as left argument"};
        if (opcode->operand1.getType() == Parser::OpcodeOperand::Type::Uint) // SNE Vx, byte
        {
            output.append16(0x4000 |
                    (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                    (opcode->operand1.getAsUint() & 0xff));
        }
        else // SNE Vx, Vy
        {
            output.append16(0x9000 |
                    (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                    Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4);
        }
        break;

    case Parser::OPCODE_LD:
        printErrorIfWrongNumOfOps(2);
        switch (opcode->operand0.getType())
        {
        case Parser::OpcodeOperand::Type::Register:
        {
            if (opcode->operand0.getAsRegister() == Parser::REGISTER_I) // LD I, addr
            {
                if (opcode->operand1.getType() == Parser::OpcodeOperand::Type::Uint)
                {
                    output.append16(0xa000 |
                            (opcode->operand1.getAsUint() & 0x0fff));
                }
                else if (opcode->operand1.getType() == Parser::OpcodeOperand::Type::LabelReference)
                {
                    output.append16(0xa000 |
                            (getLabelAddress(opcode->operand1.getAsLabel().name) & 0x0fff));
                }
                else
                {
                    throw std::runtime_error{"LD can only load constant value to I"};
                }
            }
            else if (opcode->operand0.getAsRegister() == Parser::REGISTER_I_ADDR) // LD [I], Vx
            {
                output.append16(0xa055 |
                        (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            }
            else if (opcode->operand0.getAsRegister() == Parser::REGISTER_DT) // LD DT, Vx
            {
                output.append16(0xf015 |
                        (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            }
            else if (opcode->operand0.getAsRegister() == Parser::REGISTER_ST) // LD ST, Vx
            {
                output.append16(0xf018 |
                        (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            }
            else // Operand 0: Vx register
            {
                switch (opcode->operand1.getType()) // Decide opcode using operand 1
                {
                case Parser::OpcodeOperand::Type::Uint: // LD Vx, byte
                    output.append16(0x6000 |
                            (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                            (opcode->operand1.getAsUint() & 0xff));
                    break;

                case Parser::OpcodeOperand::Type::Register:
                    if (opcode->operand1.getAsRegister() == Parser::REGISTER_I) // We can't load from I
                    {
                        throw std::runtime_error{"LD can't load from register I"};
                    }
                    else if (opcode->operand1.getAsRegister() == Parser::REGISTER_I_ADDR) // LD Vx, [I]
                    {
                        output.append16(0xf065 |
                                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
                    }
                    else if (opcode->operand1.getAsRegister() == Parser::REGISTER_DT) // LD Vx, DT
                    {
                        output.append16(0xf007 |
                                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
                    }
                    else // LD Vx, Vy
                    {
                        output.append16(0x8000 |
                                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
                    }
                    break;

                case Parser::OpcodeOperand::Type::K: // LD Vx, K
                        output.append16(0xf00a |
                                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
                    break;

                case Parser::OpcodeOperand::Type::F:
                    throw std::runtime_error{"LD: Right-side operand can't be F"};
                    break;

                case Parser::OpcodeOperand::Type::B:
                    throw std::runtime_error{"LD: Right-side operand can't be B"};
                    break;

                case Parser::OpcodeOperand::Type::LabelReference:
                    throw std::runtime_error{"LD: Can't load address into a Vx register"};
                    break;

                case Parser::OpcodeOperand::Type::Empty:
                    // Already handled
                    break;
                }
            }
            break;
        }

        case Parser::OpcodeOperand::Type::K:
            throw std::runtime_error{"LD: Left-side operand can't be K"};
            break;

        case Parser::OpcodeOperand::Type::Uint:
        case Parser::OpcodeOperand::Type::LabelReference:
        case Parser::OpcodeOperand::Type::Empty: // Make the compiler happy
            throw std::runtime_error{"LD: Destination can't be a constant value"};
            break;

        case Parser::OpcodeOperand::Type::F: // LD F, Vx
            output.append16(0xf029 |
                    (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            break;

        case Parser::OpcodeOperand::Type::B: // LD B, Vx
            output.append16(0xf033 |
                    (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            break;
        }
        break;

    case Parser::OPCODE_ADD:
        if (opcode->operand0.getAsRegister() == Parser::REGISTER_I) // ADD I, Vx
        {
            output.append16(0xf01e |
                    (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
        }
        else
        {
            if (opcode->operand1.getType() == Parser::OpcodeOperand::Type::Uint) // ADD Vx, byte
            {
                output.append16(0x7000 |
                        (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                        (opcode->operand1.getAsUint() & 0xff));
            }
            else // ADD Vx, Vy
            {
                output.append16(0x8004 |
                        (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                        (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
            }
        }
        break;

    case Parser::OPCODE_OR:
        output.append16(0x8001 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Parser::OPCODE_AND:
        output.append16(0x8002 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Parser::OPCODE_XOR:
        output.append16(0x8003 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Parser::OPCODE_SUB:
        output.append16(0x8005 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Parser::OPCODE_SHR:
        output.append16(0x8006 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Parser::OPCODE_SUBN:
        output.append16(0x8007 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Parser::OPCODE_SHL:
        output.append16(0x800e |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Parser::OPCODE_RND:
        output.append16(0xc000 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (opcode->operand1.getAsUint() & 0xff));
        break;

    case Parser::OPCODE_DRW:
        output.append16(0xd000 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Parser::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4) |
                (opcode->operand2.getAsUint() & 0x0f));
        break;

    case Parser::OPCODE_SKP:
        output.append16(0xe09e |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
        break;

    case Parser::OPCODE_SKNP:
        output.append16(0xe0a1 |
                (Parser::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
        break;

    case Parser::OPCODE_INVALID:
        throw std::runtime_error{"Invalid opcode"};;
    }
}

static void handleDbInst(const Parser::DbInst* db, ByteList& output)
{
    for (uint8_t data : db->arguments)
        output.append8(data);

    if (output.size() % 2)
        Logger::warn << "Unaligned data. Instructions should only be at even addresses." << Logger::End;
}

static void handleDwInst(const Parser::DwInst* dw, ByteList& output)
{
    for (uint16_t data : dw->arguments)
        output.append16(data);
}

ByteList generateBinary(const Parser::tokenList_t& tokens, const Parser::labelMap_t& labels)
{
    ByteList output;

    for (auto& token : tokens)
    {
        try
        {
            auto opcode = dynamic_cast<Parser::Opcode*>(token.get());
            auto dbInst = dynamic_cast<Parser::DbInst*>(token.get());
            auto dwInst = dynamic_cast<Parser::DwInst*>(token.get());
            if (opcode)
            {
                handleOpcode(opcode, output, labels);
            }
            else if (dbInst)
            {
                handleDbInst(dbInst, output);
            }
            else if (dwInst)
            {
                handleDwInst(dwInst, output);
            }
            else
            {
                // Invalid token pointer - current token is implemented in the tokenizer but not in the binary generator
                // Or something is broken
                assert(false);
            }
        }
        catch (std::exception& e)
        {
            // Rethrown the exception with more info
            throw std::runtime_error{"Line " + token->getLineNumberStr() + ": " + e.what()};
        }
    }
    return output;
}

