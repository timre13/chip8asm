#include "binary_generator.h"
#include "tokenizer.h"
#include "Logger.h"
#include "common.h"

static void handleOpcode(const Tokenizer::Opcode* opcode, ByteList& output)
{
    Logger::dbg << "Opcode: " << opcode->opcode << Logger::End;
    switch (opcode->opcode)
    {
    case Tokenizer::OPCODE_CLS:
        output.append16(0x00e0);
        break;

    case Tokenizer::OPCODE_RET:
        output.append16(0x00ee);
        break;

    case Tokenizer::OPCODE_JP:
        output.append16(0x1000 | (opcode->operand0.getAsUint() & 0x0fff));
        // TODO: Implement other variations
        break;

    case Tokenizer::OPCODE_CALL:
        output.append16(0x2000 | (opcode->operand0.getAsUint() & 0x0fff));
        break;

    case Tokenizer::OPCODE_SE:
    case Tokenizer::OPCODE_SNE:
    case Tokenizer::OPCODE_LD:
    case Tokenizer::OPCODE_ADD:
    case Tokenizer::OPCODE_OR:
    case Tokenizer::OPCODE_AND:
    case Tokenizer::OPCODE_XOR:
    case Tokenizer::OPCODE_SUB:
    case Tokenizer::OPCODE_SHR:
    case Tokenizer::OPCODE_SUBN:
    case Tokenizer::OPCODE_SHL:
    case Tokenizer::OPCODE_RND:
    case Tokenizer::OPCODE_DRW:
    case Tokenizer::OPCODE_SKP:
    case Tokenizer::OPCODE_SKNP:
    case Tokenizer::OPCODE_INVALID:
        TODO();
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

