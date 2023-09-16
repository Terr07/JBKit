#pragma once

#include "Common.hpp"

namespace Jasmin
{

struct Lexeme
{
  enum class TokenType
  {
    Symbol,
    Keyword,
    StringLiteral,
    NumberLiteral,
    ArithmeticOperator,
    Newline,
    Colon,
    Dot,
  } Type;

  std::string Value;

  Lexeme(TokenType type, std::string val): Type{type}, Value{val} {}
  Lexeme(TokenType type, int val): Type{type}, Value{static_cast<char>(val)} {}

  std::string_view GetTypeString() const
  {
    static std::unordered_map<TokenType, std::string_view> names =
    {
      {TokenType::Symbol,             "Symbol"},
      {TokenType::Keyword,            "Keyword"},
      {TokenType::StringLiteral,      "StringLiteral"},
      {TokenType::NumberLiteral,      "NumberLiteral"},
      {TokenType::ArithmeticOperator, "ArithmeticOperator"},
      {TokenType::Newline,            "Newline"},
      {TokenType::Colon,              "Colon"},
      {TokenType::Dot,                "Dot"},
    };

    return names[this->Type];
  }
};

class Lexer
{

  public:
    Lexer(const char* file) : inputStream{file, std::ios::binary} 
    {
      if(!inputStream.good())
        throw std::runtime_error{"Failed to open file."};
    }

    bool HasMore() 
    {
      return inputStream.peek() != EOF;
    }

    Lexeme LexNext()
    {
      skipWhitespace();
      skipComments();

      char ch = inputStream.peek();

      if(ch == '.')
        return {Lexeme::TokenType::Dot, inputStream.get()};

      if(ch == ':')
        return {Lexeme::TokenType::Colon, inputStream.get()};

      if(ch == '\n')
        return {Lexeme::TokenType::Newline, inputStream.get()};

      if(ch == '+' || ch == '-' || ch == '/' || ch == '*')
        return {Lexeme::TokenType::ArithmeticOperator, inputStream.get()};

      if(ch == '"')
        return lexStringLiteral();

      if(std::isdigit(ch))
        return lexNumberLiteral();

      if(std::isalnum(ch))
        return lexString();

      throw std::runtime_error{"Failed to lex unknown character encountered."};
    }

    std::vector<Lexeme> LexAll()
    {
      std::vector<Lexeme> lexemes;

      while(this->HasMore())
        lexemes.push_back(this->LexNext());

      return lexemes;
    }

  private:
    std::ifstream inputStream;


    void skipWhitespace()
    {
      while(std::isspace(inputStream.peek()))
        inputStream.get();
    }

    void skipComments()
    {
      if(inputStream.peek() != ';')
        return;

      inputStream.get();

      while(inputStream.peek() != '\n' && inputStream.peek() != EOF)
        inputStream.get();
    }

    Lexeme lexStringLiteral()
    {
      if(inputStream.peek() != '"')
        throw std::runtime_error{"failed to lex string literal: no opening \" encountered."};

      inputStream.get();

      std::string str;

      while(inputStream.peek() != '"' && inputStream.peek() != EOF)
        str += inputStream.get();

      if(inputStream.get() != '"') 
        throw std::runtime_error{"failed to lex string literal: no closing \" encountered."};

      return {Lexeme::TokenType::StringLiteral, str};
    }

    Lexeme lexNumberLiteral()
    {
      if( !std::isdigit(inputStream.peek()) )
        throw std::runtime_error{"failed to lex number literal: character encountered is not a digit."};

      std::string numStr;

      while(std::isdigit(inputStream.peek()))
        numStr += inputStream.get();

      return Lexeme{Lexeme::TokenType::NumberLiteral, numStr};
    }

    Lexeme lexString()
    {
      if( !std::isalnum(inputStream.peek()) )
        throw std::runtime_error{"failed to lex string: character encountered is not alnum."};

      std::string str;

      while(std::isalnum(inputStream.peek()) || std::ispunct(inputStream.peek()))
        str += inputStream.get();

      if(isKeyword(str))
        return Lexeme{Lexeme::TokenType::Keyword, str};

      return Lexeme{Lexeme::TokenType::Symbol, str};
    }

    bool isKeyword(std::string_view str)
    {
      static std::unordered_set<std::string_view> keywords = 
      {
        "public",
        "private",
        "protected",
        "static",
        "volatile",
        "transient",
        "final",
        "super",
        "interface",
        "abstract",
        "native",
        "synchronized",
      };

      return keywords.find(str) != keywords.end();
    }

};

} //namespace: Jasmin

