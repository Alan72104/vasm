// Copyright 2023 solar-mist

#ifndef VASM_BUILDER_BUILDER_H
#define VASM_BUILDER_BUILDER_H 1

#include "vasm/codegen/builder/Instruction.h"

namespace codegen
{
    class OpcodeBuilder
    {
    public:
        OpcodeBuilder(codegen::IOutputFormat* outputFormat);

        Instruction createInstruction(codegen::Section section);

        void addLabel(std::string name, codegen::Section section);
        void relocLabel(std::string name, std::string location, codegen::Section section, int offset = 0);

        void addExtern(const std::string& name);

        std::pair<unsigned long long, bool> getLabel(std::string name);
        unsigned long long getPosition(codegen::Section section);

    private:
        codegen::IOutputFormat* mOutputFormat;
    };
}

#endif