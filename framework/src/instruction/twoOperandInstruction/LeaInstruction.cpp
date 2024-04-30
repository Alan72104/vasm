// Copyright 2023 solar-mist


#include "vasm/instruction/twoOperandInstruction/LeaInstruction.h"

#include "vasm/instruction/operand/Memory.h"
#include "vasm/instruction/operand/Register.h"
#include "vasm/instruction/operand/Relative.h"
#include "vasm/instruction/operand/Label.h"

#include "vasm/codegen/Opcodes.h"

#include "vasm/error/ErrorMessages.h"
#include <iostream>

namespace instruction
{
    void LeaInstructionImpl::emit(codegen::OpcodeBuilder& builder, codegen::Section section, LeaInstruction& instruction)
    {
        Register* lhs = static_cast<Register*>(instruction.getLeft().get());
        Memory* rhs   = dynamic_cast<Memory*>(instruction.getRight().get());
        Relative* relRhs = dynamic_cast<Relative*>(instruction.getRight().get());
        
        if (relRhs)
        {
            int instructionSize = 7;
            if (lhs->getSize() == codegen::OperandSize::Long) instructionSize = 6;

            int displacement = relRhs->getLabel()->getValue(builder, section).first - builder.getPosition(section) - instructionSize;

            switch(lhs->getSize())
            {
                case codegen::OperandSize::Byte:
                    builder.reportError(instruction.getLineNumber(), error::INV_OPCODE_OPERAND);
                    break;
                case codegen::OperandSize::Word:
                    builder.createInstruction(section)
                        .prefix(codegen::SIZE_PREFIX)
                        .opcode(codegen::LEA)
                        .modrm(codegen::AddressingMode::RegisterIndirect, lhs->getID(), 0b101)
                        .displacement(displacement, true)
                        .emit();
                    break;
                case codegen::OperandSize::Long:
                    builder.createInstruction(section)
                        .opcode(codegen::LEA)
                        .modrm(codegen::AddressingMode::RegisterIndirect, lhs->getID(), 0b101)
                        .displacement(displacement, true)
                        .emit();
                    break;
                case codegen::OperandSize::Quad:
                    builder.createInstruction(section)
                        .prefix(codegen::REX::W)
                        .opcode(codegen::LEA)
                        .modrm(codegen::AddressingMode::RegisterIndirect, lhs->getID(), 0b101)
                        .displacement(displacement, true)
                        .emit();
                    break;
                default:
                    break; // Unreachable
            }
            return;
        }

        codegen::AddressingMode addressingMode = rhs->getAddressingMode();

        switch (lhs->getSize())
        {
            case codegen::OperandSize::Byte:
                builder.reportError(instruction.getLineNumber(), error::INV_OPCODE_OPERAND);
                break;
            case codegen::OperandSize::Word:
                builder.createInstruction(section)
                       .prefix(codegen::SIZE_PREFIX)
                       .opcode(codegen::LEA)
                       .modrm(addressingMode, lhs->getID(), rhs->getBase()->getID())
                       .sib(rhs->getSIB())
                       .displacement(rhs->getDisplacement())
                       .emit();
                break;
            case codegen::OperandSize::Long:
                builder.createInstruction(section)
                       .opcode(codegen::LEA)
                       .modrm(addressingMode, lhs->getID(), rhs->getBase()->getID())
                       .sib(rhs->getSIB())
                       .displacement(rhs->getDisplacement())
                       .emit();
                break;
            case codegen::OperandSize::Quad:
                builder.createInstruction(section)
                       .prefix(codegen::REX::W)
                       .opcode(codegen::LEA)
                       .modrm(addressingMode, lhs->getID(), rhs->getBase()->getID())
                       .sib(rhs->getSIB())
                       .displacement(rhs->getDisplacement())
                       .emit();
                break;
            default:
                break; // Unreachable
        }
    }
}