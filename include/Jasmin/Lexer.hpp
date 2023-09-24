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
    bool isKeyword(std::string_view) const;

    //allows passing to ensureNext. Apparently standard library functions such 
    //as std::isdigit, aren't addressable so need wrappers like this to be passed
    //as functors. 
    static bool isDigit(char c) { return std::isdigit(c); }
    static bool isAlnum(char c) { return std::isalnum(c); }
    static bool isAlpha(char c) { return std::isalpha(c); }
    static bool isSpace(char c) { return std::isspace(c); }
    static bool isPunct(char c) { return std::ispunct(c); }

    Lexeme makeLex(Lexeme::TokenType type, std::string c) const;
    Lexeme makeLex(Lexeme::TokenType type, char c) const;

    char get();
    char peek();
    void ensureNext(char);
    void ensureNext(std::function<bool(char)>);

    std::runtime_error error(std::string_view) const;
};

} //namespace: Jasmin

