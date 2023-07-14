#ifndef VASM_PARSER_PARSER_H
#define VASM_PARSER_PARSER_H 1

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Lexing
{
    class Token;
    enum class TokenType;
}

namespace Codegen
{
    class OutputFormat;
    enum class Section;
    enum class OperandSize;
}

namespace Parsing
{
    class Parser
    {
    public:
        Parser(std::vector<Lexing::Token>& tokens, Codegen::OutputFormat& output);

        void parse();

    private:
        std::vector<Lexing::Token>& mTokens;
        Codegen::OutputFormat& mOutput;
        int mPosition{ 0 };
        Codegen::Section mSection;
        std::unordered_map<std::string_view, std::function<void()>> mInstructionParsers;


        Lexing::Token& current();
        Lexing::Token& consume();
        Lexing::Token& peek(int offset);
        void expectToken(Lexing::TokenType tokenType);
        int getBinaryOperatorPrecedence(Lexing::TokenType tokenType) const;
        bool isImmediate(Lexing::TokenType tokenType) const;
        
        void parseStatement();

        void parseLabel();

        long long parseExpression(int precedence = 1);
        long long parseImmediate();
        
        std::pair<long long, Codegen::OperandSize> parseRegister();
    };
}

#endif