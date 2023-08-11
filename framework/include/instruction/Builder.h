// Copyright 2023 solar-mist

#include "codegen/Opcodes.h"
#ifndef VASM_INSTRUCTION_BUILDER_H
#define VASM_INSTRUCTION_BUILDER_H 1

#include "instruction/NoOperandInstruction.h"
#include "instruction/SingleOperandInstruction.h"
#include "instruction/TwoOperandInstruction.h"

#include "instruction/operand/Immediate.h"
#include "instruction/operand/Register.h"

#include "lexer/Token.h"

#include <vector>

namespace instruction
{
    struct TokenStream
    {
        std::vector<lexing::Token>* tokens;
        size_t* position;
    };

    template<typename T>
    class Builder
    {
    public:
        Builder() {}

        std::unique_ptr<T> parse(TokenStream tokens)
        {
            mTokens = tokens;
            if constexpr (std::derived_from<T, NoOperandInstruction>)
            {
                return std::make_unique<T>();
            }
            else if constexpr (std::derived_from<T, SingleOperandInstruction>)
            {
                return std::make_unique<T>(parseOperand());
            }
            else if constexpr (std::derived_from<T, TwoOperandInstruction>)
            {
                OperandPtr left = parseOperand();
                // TODO: expectToken(lexing::TokenType::Comma);
                consume();
                OperandPtr right = parseOperand();
                return std::make_unique<T>(std::move(left), std::move(right));
            }
        }

    private:
        lexing::Token& current()
        {
            return mTokens.tokens->at(*mTokens.position);
        }

        lexing::Token& consume()
        {
            return mTokens.tokens->at((*mTokens.position)++);
        }


        OperandPtr parseOperand()
        {
            switch (mTokens.tokens->at(*mTokens.position).getTokenType())
            {
                case lexing::TokenType::Register:
                    return parseRegister();

                case lexing::TokenType::Immediate:
                    return parseImmediate();
            }
        }

        ImmediatePtr parseImmediate()
        {
            return std::make_unique<Immediate>(std::stoi(consume().getText()));
        }

        RegisterPtr parseRegister()
        {
            constexpr int REGISTERS_PER_ENCODING = 4;

            unsigned long long index;
            for (index = 0; index < static_cast<unsigned long long>(codegen::Registers.size()); index++)
            {
                if (codegen::Registers[index] == current().getText())
                {
                    break;
                }
            }
            consume();

            return std::make_unique<Register>(index / REGISTERS_PER_ENCODING, static_cast<codegen::OperandSize>(index % REGISTERS_PER_ENCODING));
        }

        TokenStream mTokens;
    };
}

#endif