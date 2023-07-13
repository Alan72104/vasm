#include <parser/Parser.h>

#include <lexer/Token.h>

#include <codegen/OutputFormat.h>

#include <cstdint>
#include <iostream>
#include <format>
#include <string_view>

namespace Parsing
{
    Parser::Parser(std::vector<Lexing::Token>& tokens, Codegen::OutputFormat& output)
        :mTokens(tokens), mPosition(0), mOutput(output), mSection(Codegen::Section::Text)
    {
    }
    
    Lexing::Token Parser::current()
    {
        return mTokens[mPosition];
    }

    Lexing::Token Parser::consume()
    {
        return mTokens[mPosition++];
    }

    Lexing::Token Parser::peek(int offset)
    {
        return mTokens[mPosition + offset];
    }

    void Parser::expectToken(Lexing::TokenType tokenType)
    {
        if(current().getTokenType() != tokenType)
        {
            Lexing::Token temp(tokenType);
            std::cerr << std::format("Expected {}. Found {}. Terminating program.\n", temp.getTokenTypeString(), current().getText()); // TODO: Error properly
            std::exit(1);
        }
    }

    void Parser::parse()
    {
        while (mPosition < mTokens.size())
        {
            parseStatement();
        }
    }

    void Parser::parseStatement()
    {
        switch(current().getTokenType())
        {
            case Lexing::TokenType::Error:
                std::cerr << "Found unknown symbol. Terminating program.\n"; // TODO: Error properly
                std::exit(1);
            case Lexing::TokenType::DBInst:
                parseDBInst();
                break;
            case Lexing::TokenType::DWInst:
                parseDWInst();
                break;
            case Lexing::TokenType::DDInst:
                parseDDInst();
                break;
            case Lexing::TokenType::DQInst:
                parseDQInst();
                break;
            default:
                std::cerr << "Expected statement. Found " << current().getText() << ". Terminating program.\n"; // TODO: Error properly
                std::exit(1);
        }
    }

    void Parser::parseDBInst()
    {
        consume();
        
        char value = parseImmediate();
        mOutput.write(value, mSection);
    }

    void Parser::parseDWInst()
    {
        consume();

        short value = parseImmediate();
        mOutput.write(value, mSection);
    }

    void Parser::parseDDInst()
    {
        consume();

        int value = parseImmediate();
        mOutput.write(value, mSection);
    }

    void Parser::parseDQInst()
    {
        consume();

        long value = parseImmediate();
        mOutput.write(value, mSection);
    }

    long long Parser::parseImmediate()
    {
        if (current().getTokenType() == Lexing::TokenType::Immediate)
            return std::stoll(consume().getText(), 0, 0);

        else if (current().getTokenType() == Lexing::TokenType::Dollar)
        {
            consume();
            return mOutput.getPosition();
        }
        else if (current().getTokenType() == Lexing::TokenType::DollarDollar)
        {
            consume();
            return mOutput.getSectionStart(mSection);
        }
        
        else
            expectToken(Lexing::TokenType::Immediate);
        return -1;
    }
}