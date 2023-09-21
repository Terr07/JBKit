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
  Lexeme(TokenType type, int val);

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
    std::ifstream inputStream;

    void skipWhitespace();
    void skipComments();
    Lexeme lexStringLiteral();
    Lexeme lexNumericLiteral();
    Lexeme lexString();
    bool isKeyword(std::string_view str);
};

} //namespace: Jasmin

