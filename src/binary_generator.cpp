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
        if (opcode->operand0.getType() == Tokenizer::OpcodeOperand::Type::Uint) // SE Vx, byte
        {
            output.append16(0x3000 |
                    (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                    (opcode->operand1.getAsUint() & 0xff));
        }
        else // SE Vx, Vy
        {
            output.append16(0x3000 |
                    (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                    Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()));
        }
        break;

    case Tokenizer::OPCODE_SNE:
        output.append16(0x4000 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (opcode->operand1.getAsUint() & 0xff));
        break;

    case Tokenizer::OPCODE_LD:
        printErrorIfWrongNumOfOps(2);
        if (opcode->operand0.getType() == Tokenizer::OpcodeOperand::Type::Register)
        {
            output.append16(0x6000 |
                    (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                    (opcode->operand1.getAsUint() & 0xff));
        }
        // XXX: More LD variants
        break;

    case Tokenizer::OPCODE_ADD:
        output.append16(0x7000 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (opcode->operand1.getAsUint() & 0xff));
        break;

    case Tokenizer::OPCODE_OR:
        output.append16(0x8001 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToByte(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_AND:
        output.append16(0x8002 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToByte(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_XOR:
        output.append16(0x8003 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToByte(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_SUB:
        output.append16(0x8005 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToByte(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_SHR:
        output.append16(0x8006 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToByte(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_SUBN:
        output.append16(0x8007 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToByte(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_SHL:
        output.append16(0x800e |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToByte(opcode->operand1.getAsRegister()) << 4));
        break;

    case Tokenizer::OPCODE_RND:
        output.append16(0xc000 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (opcode->operand1.getAsUint() & 0xff));
        break;

    case Tokenizer::OPCODE_DRW:
        output.append16(0x8000 |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8) |
                (Tokenizer::vRegisterToByte(opcode->operand1.getAsRegister()) << 4) |
                (opcode->operand1.getAsUint() & 0x0f));
        break;

    case Tokenizer::OPCODE_SKP:
        output.append16(0xe09e |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8));
        break;

    case Tokenizer::OPCODE_SKNP:
        output.append16(0xe09e |
                (Tokenizer::vRegisterToByte(opcode->operand0.getAsRegister()) << 8));
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

