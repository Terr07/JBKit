#pragma once

#include "Common.hpp"

namespace Jasmin
{

struct Lexeme
{
  struct Metainfo
  {
    unsigned int LineNumber;
    unsigned short LineOffset;
    size_t FileOffset;
  } Info;

  enum class TokenType
  {
    Symbol,
    Keyword,
    StringLiteral,
    NumericLiteral,
    ArithmeticOperator,
    Newline,
    Colon,
    Dot,
    Bracket,
    Brace,
    Paren
  } Type;

  std::string Value;
  double GetNumericValue() const;

  Lexeme(TokenType type, std::string val);
  Lexeme(TokenType type, char val);

  std::string_view GetTypeString() const;
};

class Lexer
{

  public:
    Lexer(const char* file);

    bool HasMoreAfterSkip();
    Lexeme LexNext();
    std::queue<Lexeme> LexAll();

  private:
    std::ifstream  inputStream;
    unsigned int   lineNumber{1};
    unsigned short lineOffset{1};
    size_t         fileOffset{0};


    void skipWhitespace();
    void skipComments();
    Lexeme lexStringLiteral();
    Lexeme lexNumericLiteral();
    Lexeme lexString();
    bool isKeyword(std::string_view str);

    Lexeme makeLex(Lexeme::TokenType type, std::string c) const;
    Lexeme makeLex(Lexeme::TokenType type, char c) const;

    std::tuple<size_t, size_t> getMetaInfo() const;

    char get();
    char peek();
};

} //namespace: Jasmin

