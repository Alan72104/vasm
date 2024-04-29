// Copyright 2023 solar-mist


#include "vasm/instruction/operand/Memory.h"
#include "vasm/instruction/operand/Register.h"

#include <cmath>

namespace instruction
{
    Memory::Memory(RegisterPtr base, std::optional<int> displacement, RegisterPtr index, std::optional<int> scale)
        : mBase(std::move(base))
        , mDisplacement(displacement)
        , mIndex(std::move(index))
        , mScale(scale)
    {
    }

    Register* Memory::getBase() const
    {
        return mBase.get();
    }

    std::optional<int> Memory::getDisplacement() const
    {
        return mDisplacement;
    }

    Register* Memory::getIndex() const
    {
        return mIndex.get();
    }

    std::optional<int> Memory::getScale() const
    {
        return mScale;
    }

    codegen::AddressingMode Memory::getAddressingMode() const
    {
        if (!mDisplacement)
        {
            return codegen::AddressingMode::RegisterIndirect;
        }
        if (mDisplacement <= INT8_MAX)
        {
            return codegen::AddressingMode::OneByteDisp;
        }
        return codegen::AddressingMode::FourByteDisp;
    }

    codegen::SIB Memory::getSIB() const
    {
        if(mIndex != nullptr)
        {
            unsigned char scale = mScale ? std::log2(*mScale) : 0;
            return codegen::SIB(scale, mIndex->getID(), mBase->getID());
        }
        else
        {
            return codegen::SIB(0xff, 0xff, 0xff);
        }
    }

    std::unique_ptr<Operand> Memory::clone()
    {
        Register* index = mIndex ? static_cast<Register*>(mIndex->clone().release()) : nullptr;
        return std::make_unique<Memory>(RegisterPtr(static_cast<Register*>(mBase->clone().release())), mDisplacement, RegisterPtr(index), mScale);
    }
}