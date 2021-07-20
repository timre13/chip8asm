#include "binary_generator.h"
#include "tokenizer.h"
#include "Logger.h"
#include "common.h"
#include <utility>

static void handleOpcode(const Tokenizer::Opcode* opcode, ByteList& output)
{
    auto printErrorIfWrongNumOfOps{
        [&opcode = std::as_const(opcode)](int maxArgs){
            bool shouldPrintError = false;
            assert(maxArgs >= 0 && maxArgs <= 3);
            assert(opcode->opcode != Tokenizer::OPCODE_INVALID);
            switch (maxArgs)
            {
            case 0:
                if (opcode->operand0.getType() != Tokenizer::OpcodeOperand::Type::Empty)
                    shouldPrintError = true;
                break;

            case 1:
                if (opcode->operand0.getType() == Tokenizer::OpcodeOperand::Type::Empty
                 || opcode->operand1.getType() != Tokenizer::OpcodeOperand::Type::Empty)
                    shouldPrintError = true;
                break;

            case 2:
                if (opcode->operand0.getType() == Tokenizer::OpcodeOperand::Type::Empty
                 || opcode->operand1.getType() == Tokenizer::OpcodeOperand::Type::Empty
                 || opcode->operand2.getType() != Tokenizer::OpcodeOperand::Type::Empty)
                    shouldPrintError = true;
                break;

            case 3:
                if (opcode->operand0.getType() == Tokenizer::OpcodeOperand::Type::Empty
                 || opcode->operand1.getType() == Tokenizer::OpcodeOperand::Type::Empty
                 || opcode->operand2.getType() == Tokenizer::OpcodeOperand::Type::Empty)
                    shouldPrintError = true;
                break;
            }
            if (shouldPrintError)
                Logger::fatal << "Invalid number of arguments for opcode: " << Tokenizer::opcodeNames[opcode->opcode] << Logger::End;
        }
    };

    Logger::dbg << "Opcode: " << opcode->opcode << Logger::End;
    switch (opcode->opcode)
    {
    case Tokenizer::OPCODE_NOP:
        printErrorIfWrongNumOfOps(0);
        output.append16(0x0000);
        break;

    case Tokenizer::OPCODE_SYS:
        if (opcode->operand0.getType() == Tokenizer::OpcodeOperand::Type::Uint)
        {
            output.append16(0x0000 |
                    (opcode->operand0.getAsUint() & 0x0fff));
        }
        else
        {
            output.append16(0x0000);
        }
        break;

    case Tokenizer::OPCODE_CLS:
        printErrorIfWrongNumOfOps(0);
        output.append16(0x00e0);
        break;

    case Tokenizer::OPCODE_RET:
        printErrorIfWrongNumOfOps(0);
        output.append16(0x00ee);
        break;

    case Tokenizer::OPCODE_JP:
        switch (opcode->operand0.getType())
        {
        case Tokenizer::OpcodeOperand::Type::Empty:
            Logger::fatal << "JP opcode requires operand(s)" << Logger::End;
            break;

        case Tokenizer::OpcodeOperand::Type::Register: // JP V0, addr
            printErrorIfWrongNumOfOps(2);
            if (opcode->operand0.getAsRegister() != Tokenizer::REGISTER_V0)
                Logger::fatal << "Register-relative jump is only possible with register V0" << Logger::End;
            output.append16(0xb000 |
                    (opcode->operand0.getAsUint() & 0x0fff));
            break;

        case Tokenizer::OpcodeOperand::Type::Uint: // JP addr
            printErrorIfWrongNumOfOps(1);
            output.append16(0x1000 |
                    (opcode->operand0.getAsUint() & 0x0fff));
            break;

        case Tokenizer::OpcodeOperand::Type::F:
        case Tokenizer::OpcodeOperand::Type::B:
        case Tokenizer::OpcodeOperand::Type::K:
            // Already handled
            break;
        }
        break;

    case Tokenizer::OPCODE_CALL:
        printErrorIfWrongNumOfOps(1);
        if (opcode->operand0.getType() != Tokenizer::OpcodeOperand::Type::Uint)
            Logger::fatal << "CALL opcode requires a constant value" << Logger::End;
        output.append16(0x2000 |
                (opcode->operand0.getAsUint() & 0x0fff));
        break;

    case Tokenizer::OPCODE_SE:
        printErrorIfWrongNumOfOps(2);
        if (opcode->operand0.getType() != Tokenizer::OpcodeOperand::Type::Register)
            Logger::fatal << "SE opcode requires a register name as left argument" << Logger::End;
        if (opcode->operand1.getType() == Tokenizer::OpcodeOperand::Type::Uint) // SE Vx, byte
        {
            output.append16(0x3000 |
                    (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                    (opcode->operand1.getAsUint() & 0xff));
        }
        else // SE Vx, Vy
        {
            output.append16(0x5000 |
                    (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                    Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4);
        }
        break;

    case Tokenizer::OPCODE_SNE:
        output.append16(0x4000 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (opcode->operand1.getAsUint() & 0xff));
        break;

    case Tokenizer::OPCODE_LD:
        printErrorIfWrongNumOfOps(2);
        switch (opcode->operand0.getType())
        {
        case Tokenizer::OpcodeOperand::Type::Register:
        {
            if (opcode->operand0.getAsRegister() == Tokenizer::REGISTER_I) // LD I, addr
            {
                output.append16(0xa000 |
                        (opcode->operand1.getAsUint() & 0x0fff));
            }
            else if (opcode->operand0.getAsRegister() == Tokenizer::REGISTER_I_ADDR) // LD [I], Vx
            {
                output.append16(0xa055 |
                        (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            }
            else if (opcode->operand0.getAsRegister() == Tokenizer::REGISTER_DT) // LD DT, Vx
            {
                output.append16(0xf015 |
                        (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            }
            else if (opcode->operand0.getAsRegister() == Tokenizer::REGISTER_ST) // LD ST, Vx
            {
                output.append16(0xf018 |
                        (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            }
            else // Operand 0: Vx register
            {
                switch (opcode->operand1.getType()) // Decide opcode using operand 1
                {
                case Tokenizer::OpcodeOperand::Type::Uint: // LD Vx, byte
                    output.append16(0x6000 |
                            (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                            (opcode->operand1.getAsUint() & 0xff));
                    break;

                case Tokenizer::OpcodeOperand::Type::Register:
                    if (opcode->operand1.getAsRegister() == Tokenizer::REGISTER_I) // We can't load from I
                    {
                        Logger::fatal << "LD can't load from register I" << Logger::End;
                    }
                    else if (opcode->operand1.getAsRegister() == Tokenizer::REGISTER_I_ADDR) // LD Vx, [I]
                    {
                        output.append16(0xf065 |
                                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
                    }
                    else if (opcode->operand1.getAsRegister() == Tokenizer::REGISTER_DT) // LD Vx, DT
                    {
                        output.append16(0xf007 |
                                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
                    }
                    else // LD Vx, Vy
                    {
                        output.append16(0x8000 |
                                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
                    }
                    break;

                case Tokenizer::OpcodeOperand::Type::K: // LD Vx, K
                        output.append16(0xf00a |
                                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
                    break;

                case Tokenizer::OpcodeOperand::Type::F:
                    Logger::fatal << "LD: Right-side operand can't be F" << Logger::End;
                    break;

                case Tokenizer::OpcodeOperand::Type::B:
                    Logger::fatal << "LD: Right-side operand can't be B" << Logger::End;
                    break;

                case Tokenizer::OpcodeOperand::Type::Empty:
                    // Already handled
                    break;
                }
            }
            break;
        }

        case Tokenizer::OpcodeOperand::Type::K:
            Logger::fatal << "LD: Left-side operand can't be K" << Logger::End;
            break;

        case Tokenizer::OpcodeOperand::Type::Uint:
        case Tokenizer::OpcodeOperand::Type::Empty: // Make the compiler happy
            Logger::fatal << "LD: Destination can't be a constant value" << Logger::End;
            break;

        case Tokenizer::OpcodeOperand::Type::F: // LD F, Vx
            output.append16(0xf029 |
                    (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            break;

        case Tokenizer::OpcodeOperand::Type::B: // LD B, Vx
            output.append16(0xf033 |
                    (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
            break;
        }
        break;

    case Tokenizer::OPCODE_ADD:
        if (opcode->operand0.getAsRegister() == Tokenizer::REGISTER_I) // ADD I, Vx
        {
            output.append16(0xf01e |
                    (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 8));
        }
        else
        {
            if (opcode->operand1.getType() == Tokenizer::OpcodeOperand::Type::Uint) // ADD Vx, byte
            {
                output.append16(0x7000 |
                        (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                        (opcode->operand1.getAsUint() & 0xff));
            }
            else // ADD Vx, Vy
            {
                output.append16(0x8004 |
                        (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                        (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
            }
        }
        break;

    case Tokenizer::OPCODE_OR:
        output.append16(0x8001 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_AND:
        output.append16(0x8002 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_XOR:
        output.append16(0x8003 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_SUB:
        output.append16(0x8005 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_SHR:
        output.append16(0x8006 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_SUBN:
        output.append16(0x8007 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_SHL:
        output.append16(0x800e |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_RND:
        output.append16(0xc000 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (opcode->operand1.getAsUint() & 0xff));
        break;

    case Tokenizer::OPCODE_DRW:
        output.append16(0xd000 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToNibble(opcode->operand1.getAsRegister()) << 4) |
                (opcode->operand2.getAsUint() & 0x0f));
        break;

    case Tokenizer::OPCODE_SKP:
        output.append16(0xe09e |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
        break;

    case Tokenizer::OPCODE_SKNP:
        output.append16(0xe0a1 |
                (Tokenizer::vRegisterToNibble(opcode->operand0.getAsRegister()) << 8));
        break;

    case Tokenizer::OPCODE_INVALID:
        Logger::fatal << "Invalid opcode" << Logger::End;
        break;
    }
}

ByteList generateBinary(const Tokenizer::tokenList_t& tokens)
{
    ByteList output;

    for (auto& token : tokens)
    {
        auto opcode = dynamic_cast<Tokenizer::Opcode*>(token.get());
        if (opcode)
        {
            handleOpcode(opcode, output);
            continue;
        }
    }

    return output;
}

