#include "codegen/Elf.h"

namespace Codegen
{
    // TODO: Make this into an enum or similar
    constexpr char const* ELF_MAGIC = "\x7f" "ELF";
    constexpr char ELF_64 = 2;
    constexpr char ELF_LITTLE_ENDIAN = 1;
    constexpr char ELF_EI_VERSION = 1;
    constexpr char ELF_SYSV_ABI = 0;
    constexpr char ELF_ABI_VERSION = 1;
    constexpr char const* ELF_PAD = "\0\0\0\0\0\0\0";

    constexpr short ELF_OBJECT = 1;
    constexpr short ELF_AMD64 = 0x3E;
    constexpr int ELF_VERSION = 1;
    constexpr long ELF_ENTRY = 0;
    constexpr long ELF_PHOFF = 0;
    constexpr long ELF_SHOFF = 0x40;
    constexpr int ELF_FLAGS = 0;
    constexpr short ELF_EHSIZE = 64;
    constexpr short ELF_PHSIZE = 0;
    constexpr short ELF_PHNUM = 0;
    constexpr short ELF_SHSIZE = 0x40;
    constexpr short ELF_SHNUM = 5;
    constexpr short ELF_SHSTRNDX = 2;
    
    constexpr int TYPE_NULL = 0;
    constexpr int TYPE_PROGBITS = 1;
    constexpr int TYPE_SYMTAB = 2;
    constexpr int TYPE_STRTAB = 3;
    constexpr int TYPE_RELA = 4;

    constexpr long SECTION_ADDR = 0;

    constexpr unsigned char SYM_LOCAL = 0;
    constexpr unsigned char SYM_GLOBAL = 1 << 4;
    constexpr unsigned char SYM_DEFAULT = 0;
    constexpr unsigned char SYM_FUNC = 2;
    constexpr unsigned char SYM_SECTION = 3;
    constexpr unsigned char SYM_FILE = 4;

    constexpr unsigned long SYM_VALUE_0 = 0;
    constexpr unsigned long SYM_SIZE_0 = 0;

    constexpr unsigned short SYM_ABS = 0xFFF1;

    ELFFormat::ELFFormat(std::string_view fileName) : mFileName {fileName}
    {
        mSections.emplace_back("", TYPE_NULL, 0, 0, 0, 0, 0, Section::Other);
        mSections.emplace_back(".shstrtab", TYPE_STRTAB, 0, 0, 0, 1, 0, Section::Other);
        mSections.emplace_back(".symtab", TYPE_SYMTAB, 0, 3, 0, 8, 24, Section::Other);
        mSections.emplace_back(".strtab", TYPE_STRTAB, 0, 0, 0, 1, 0, Section::Other);

        mSections[3].write("", 1); // NULL string
        mSections[3].write(fileName.data(), fileName.length() + 1); // File name string

        // NULL symbol
        mSections[2].write((unsigned int)0);
        mSections[2].write((unsigned char)0);
        mSections[2].write(SYM_DEFAULT);
        mSections[2].write((unsigned short)0);
        mSections[2].write(SYM_VALUE_0);
        mSections[2].write(SYM_SIZE_0);
        mSections[2].mInfo++;

        // File symbol
        mSections[2].write((unsigned int)1);
        mSections[2].write(SYM_FILE);
        mSections[2].write(SYM_DEFAULT);
        mSections[2].write(SYM_ABS);
        mSections[2].write(SYM_VALUE_0);
        mSections[2].write(SYM_SIZE_0);
        mSections[2].mInfo++;
    }


    void ELFFormat::write(unsigned char data, Section section)
    {
        ELFSection* elfSection = getSection(section);
        if (!elfSection)
        {
            elfSection = createSection(section);
        }
        elfSection->write(reinterpret_cast<const char*>(&data), sizeof(data));
    }

    void ELFFormat::write(unsigned short data, Section section)
    {
        ELFSection* elfSection = getSection(section);
        if (!elfSection)
        {
            elfSection = createSection(section);
        }
        elfSection->write(reinterpret_cast<const char*>(&data), sizeof(data));
    }

    void ELFFormat::write(unsigned int data, Section section)
    {
        ELFSection* elfSection = getSection(section);
        if (!elfSection)
        {
            elfSection = createSection(section);
        }
        elfSection->write(reinterpret_cast<const char*>(&data), sizeof(data));
    }

    void ELFFormat::write(unsigned long data, Section section)
    {
        ELFSection* elfSection = getSection(section);
        if (!elfSection)
        {
            elfSection = createSection(section);
        }
        elfSection->write(reinterpret_cast<const char*>(&data), sizeof(data));
    }


    size_t ELFFormat::getPosition(Section section)
    {
        ELFSection* elfSection = getSection(section);
        if (!elfSection)
        {
            elfSection = createSection(section);
        }
        return elfSection->mBuffer.size();
    }

    size_t ELFFormat::getSectionStart(Section)
    {
        return 0;
    }

    void ELFFormat::addSymbol(const std::string& name, unsigned long value, Section section, bool isGlobal)
    {
        ELFSection* strtab = getSection(".strtab");
        unsigned int strtabIndex = strtab->size();
        strtab->write(name);

        getOrCreateSection(section);

        ELFSection* symtab = getSection(".symtab");

        symtab->write(strtabIndex);
        symtab->write(isGlobal ? SYM_GLOBAL : SYM_LOCAL);
        symtab->write(SYM_DEFAULT);
        for (unsigned short sectionIndex = 0; sectionIndex < static_cast<unsigned short>(mSections.size()); sectionIndex++)
        {
            if (mSections[sectionIndex].mSection == section)
            {
                symtab->write(sectionIndex);
                break;
            }
        }
        symtab->write(value);
        symtab->write(SYM_SIZE_0);
        if (!isGlobal)
        {
            symtab->length()++;
        }

        mSymbols[name] = value;
    }

    unsigned long ELFFormat::getSymbol(const std::string& name) const
    {
        return mSymbols.at(name);
    }
    
    bool ELFFormat::hasSymbol(const std::string& name) const {
        return mSymbols.contains(name);
    }

    void ELFFormat::relocSymbol(const std::string& name, Section section)
    {
        ELFSection* sect = getOrCreateSection(section);
        ELFSection* rela = getSection(".rela" + sect->mName);
        if (!rela)
        {
            size_t symtabIndex;
            size_t sectionIndex = 0;
            for (symtabIndex = 0; symtabIndex < mSections.size(); symtabIndex++)
            {
                if (mSections[symtabIndex].mName == ".symtab")
                {
                    break;
                }
            }
            for (sectionIndex = 0; sectionIndex < mSections.size(); sectionIndex++)
            {
                if (mSections[sectionIndex].mName == sect->mName)
                {
                    break;
                }
            }

            mSections.emplace_back(".rela" + sect->mName, TYPE_RELA, 0, symtabIndex, sectionIndex, 8, 24, Section::Other);

            rela = &mSections.back();
        }

        unsigned long symbol = mSymbols.at(name);
        
        rela->write(getPosition(section));
        rela->write(0x200000001UL);
        rela->write(symbol);
    }


    ELFFormat::ELFSection::ELFSection(std::string_view name, int type, long flags, int link, int info, long align, long entrySize, Section section)
        : mBuffer {}, mName {name}, mNameIdx {0}, mType {type}, mFlags {flags}, mLink {link}, mInfo {info},
            mAlign {align}, mEntrySize {entrySize}, mSection {section}
    {
    }

    ELFFormat::ELFSection::ELFSection(Section section)
        : mBuffer {}, mNameIdx {0}, mSection(section)
    {
        switch (section)
        {
            case Section::Text:
                mName = ".text";
                mType = TYPE_PROGBITS;
                mFlags = 6;
                mLink = 0;
                mInfo = 0;
                mAlign = 16;
                mEntrySize = 0;
                break;
            case Section::Data:
                mName = ".data";
                mType = TYPE_PROGBITS;
                mFlags = 2;
                mLink = 0;
                mInfo = 0;
                mAlign = 16;
                mEntrySize = 0;
                break;
            default:
                break;
        }
    }

    void ELFFormat::ELFSection::write(std::unsigned_integral auto data)
    {
        for (size_t i = 0; i < sizeof(data) * 8; i += 8) {
            mBuffer.push_back(data >> i);
        }
    }

    void ELFFormat::ELFSection::write(const char* data, size_t size)
    {
        for (; size; --size) {
            mBuffer.push_back(*data++);
        }
    }

    void ELFFormat::ELFSection::write(std::string_view data)
    {
        for (size_t i = 0; i < data.size() + 1; ++i) {
            mBuffer.push_back(data[i]);
        }
    }

    size_t ELFFormat::ELFSection::size() const
    {
        return mBuffer.size();
    }

    int& ELFFormat::ELFSection::length()
    {
        return mInfo;
    }


    template <typename T>
    static inline constexpr void WriteELF(std::ostream& stream, T data)
    {
        stream.write((char*)&data, sizeof(T));
    }

    static inline void WriteELF(std::ostream& stream, const char* data, int size)
    {
        stream.write(data, size);
    }

    void ELFFormat::print(std::ostream& stream)
    {
        WriteELF(stream, ELF_MAGIC, 4);
        WriteELF(stream, ELF_64);
        WriteELF(stream, ELF_LITTLE_ENDIAN);
        WriteELF(stream, ELF_EI_VERSION);
        WriteELF(stream, ELF_SYSV_ABI);
        WriteELF(stream, ELF_ABI_VERSION);
        WriteELF(stream, ELF_PAD, 7);

        WriteELF(stream, ELF_OBJECT);
        WriteELF(stream, ELF_AMD64);
        WriteELF(stream, ELF_VERSION);
        WriteELF(stream, ELF_ENTRY);
        WriteELF(stream, ELF_PHOFF);
        WriteELF(stream, ELF_SHOFF);
        WriteELF(stream, ELF_FLAGS);
        WriteELF(stream, ELF_EHSIZE);
        WriteELF(stream, ELF_PHSIZE);
        WriteELF(stream, ELF_PHNUM);
        WriteELF(stream, ELF_SHSIZE);
        WriteELF(stream, (short)mSections.size());
        WriteELF(stream, (short)1);

        ELFSection* shstrtab = getSection(".shstrtab");
        for (ELFSection& section : mSections)
        {
            section.mNameIdx = static_cast<int>(shstrtab->size());
            shstrtab->write(section.mName);
        }

        unsigned long currentOffset = ELF_SHSIZE * mSections.size() + ELF_EHSIZE;
        for (ELFSection& section : mSections)
        {
            currentOffset = (currentOffset + 15) & ~15; // Align to 16 bytes

            WriteELF(stream, section.mNameIdx);
            WriteELF(stream, section.mType);
            WriteELF(stream, section.mFlags);
            WriteELF(stream, SECTION_ADDR);
            WriteELF(stream, currentOffset);
            WriteELF(stream, static_cast<long>(section.size()));
            WriteELF(stream, section.mLink);
            WriteELF(stream, section.mInfo);
            WriteELF(stream, section.mAlign);
            WriteELF(stream, section.mEntrySize);

            currentOffset += section.size();
        }
        
        unsigned long written = 0;
        unsigned long alignedWritten = 0;
        for (ELFSection& section : mSections)
        {
            for (unsigned long i = 0; i < alignedWritten - written; i++)
            {
                stream.put(0);
            }
            written = alignedWritten;

            stream.write(section.mBuffer.data(), static_cast<std::streamsize>(section.size()));

            written += section.size();
            alignedWritten = (written + 15) & ~15;
        }
    }

    ELFFormat::ELFSection* ELFFormat::getSection(std::string_view name)
    {
        for (ELFSection& section : mSections)
        {
            if (section.mName == name)
            {
                return &section;
            }
        }

        return nullptr;
    }

    ELFFormat::ELFSection* ELFFormat::getSection(Section section)
    {
        for (ELFSection& elfSection : mSections)
        {
            if (elfSection.mSection == section)
            {
                return &elfSection;
            }
        }

        return nullptr;
    }

    ELFFormat::ELFSection* ELFFormat::createSection(Section section)
    {
        mSections.emplace_back(section);

        ELFSection* newSection = &mSections.back();

        ELFSection* strtab = getSection(".strtab");
        ELFSection* symtab = getSection(".symtab");

        symtab->write(static_cast<unsigned int>(strtab->mBuffer.size()));
        symtab->write(SYM_SECTION);
        symtab->write(SYM_DEFAULT);
        symtab->write(static_cast<unsigned short>(mSections.size() - 1));
        symtab->write(0UL);
        symtab->write(0UL);
        symtab->mInfo++;

        strtab->write(newSection->mName.data(), newSection->mName.length() + 1);

        return newSection;
    }

    ELFFormat::ELFSection* ELFFormat::getOrCreateSection(Section section)
    {
        ELFSection* ret = getSection(section);
        if (!ret)
        {
            ret = createSection(section);
        }

        return ret;
    }
}