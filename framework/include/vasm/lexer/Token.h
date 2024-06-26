#ifndef VASM_LEXER_TOKEN_H
#define VASM_LEXER_TOKEN_H

#include <format>
#include <string>

namespace lexing
{
    enum class TokenType : int
    {
        Error,

        Dollar, DollarDollar,

        Plus, Minus, Star, Slash,
        LParen, RParen, LBracket, RBracket,
        Comma, Colon,
        Ampersand,

        Size,

        Rel,

        Identifier,

        Extern,
        Section,
        
        Immediate, String,

        Register,

        Instruction,

        End,
    };
    
    struct SourceLocation {
        SourceLocation(size_t line, size_t column);
        
        size_t line;
        size_t column;
    };

    class Token
    {
    public:
        Token(SourceLocation location, TokenType tokenType, std::string text);
        Token(SourceLocation location, TokenType tokenType);


        [[nodiscard]] TokenType getTokenType() const;
        [[nodiscard]] const std::string& getText() const;
        [[nodiscard]] SourceLocation getSourceLocation() const;

        bool operator==(const Token& other) const;
        
    private:
        TokenType mTokenType{ TokenType::Error };

        std::string mText;

        SourceLocation mSourceLocation;
    };
}

template<>
struct std::formatter<lexing::TokenType> : std::formatter<int>
{
    template <typename FormatContext>
    auto format(lexing::TokenType tokenType, FormatContext& context) const
    {
        return formatter<int>::format((int)tokenType, context);
    }
};

#endif