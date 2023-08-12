// Copyright 2023 solar-mist

#ifndef VASM_INSTRUCTION_TWO_OPERAND_INSTRUCTION_LEA_H
#define VASM_INSTRUCTION_TWO_OPERAND_INSTRUCTION_LEA_H 1

#include "instruction/TwoOperandInstruction.h"

namespace instruction
{
    struct LeaInstructionImpl;
    using LeaInstruction = TwoOperandInstructionTemplate<LeaInstructionImpl>;

    struct LeaInstructionImpl
    {
        static void emit(codegen::OpcodeBuilder& builder, codegen::Section section, LeaInstruction& instruction);
    };
}

#endif