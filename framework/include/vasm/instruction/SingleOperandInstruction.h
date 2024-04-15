// Copyright 2023 solar-mist

#ifndef VASM_INSTRUCTION_SINGLE_OPERAND_INSTRUCTION_H
#define VASM_INSTRUCTION_SINGLE_OPERAND_INSTRUCTION_H 1

#include "vasm/instruction/Instruction.h"
#include "vasm/instruction/Operand.h"

namespace instruction
{
    class SingleOperandInstruction : public Instruction
    {
    public:
        SingleOperandInstruction(OperandPtr operand, codegen::OperandSize size, int lineNumber)
            : mOperand(std::move(operand))
            , mSize(size)
            , mLineNumber(lineNumber)
        { }

        virtual ~SingleOperandInstruction() { }

        int getLineNumber() const { return mLineNumber; }
        codegen::OperandSize getSize() const { return mSize; }

    protected:
        OperandPtr mOperand;
        codegen::OperandSize mSize;
        int mLineNumber;
    };

    template <typename T>
    class SingleOperandInstructionTemplate : public SingleOperandInstruction
    {
    public:
        SingleOperandInstructionTemplate(OperandPtr operand, codegen::OperandSize size, int lineNumber=-1) : SingleOperandInstruction(std::move(operand), size, lineNumber) { }

        void emit(codegen::OpcodeBuilder& builder, codegen::Section section) override
        {
            T::emit(builder, section, *this);
        }

        OperandPtr& getOperand()
        {
            return mOperand;
        }
    };
}

#endif