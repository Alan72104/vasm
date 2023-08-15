// Copyright 2023 solar-mist

#ifndef VASM_INSTRUCTION_OPERAND_REGISTER_H
#define VASM_INSTRUCTION_OPERAND_REGISTER_H 1

#include "vasm/instruction/Operand.h"

#include "vasm/codegen/Opcodes.h"

namespace instruction
{
    class Register : public Operand
    {
    public:
        Register(unsigned char id, codegen::OperandSize size);

        unsigned char getID() const;
        codegen::OperandSize getSize() const;

    private:
        unsigned char mID;
        codegen::OperandSize mSize;
    };
    using RegisterPtr = std::unique_ptr<Register>;
}

#endif