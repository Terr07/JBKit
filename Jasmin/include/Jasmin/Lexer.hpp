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
    String,
    Keyword,
    Directive,
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

  Lexeme(TokenType type, std::string_view val);
  Lexeme(TokenType type, char val);

  std::string_view GetTypeString() const;
  static std::string_view GetTypeString(TokenType);
};

enum class DirectiveType 
{
  Catch,
  Class,
  End,
  Field,
  Implements,
  Interface,
  Limit,
  Line,
  Method,
  Source,
  Super,
  Throws,
  Var,
};

bool IsDirectiveType(std::string_view);
DirectiveType DirectiveTypeFromStr(std::string_view);
std::string ToString(DirectiveType);

bool IsKeyword(std::string_view);

class Lexer
{
  public:
    static std::queue<Lexeme> Lex(const char* file);

  private:
    Lexer(const char* file);

    bool hasMoreAfterSkip();
    void skipWhitespace();
    void skipComments();

    Lexeme lexNext();
    Lexeme lexStringLiteral();
    Lexeme lexNumericLiteral();
    Lexeme lexString();

    //allows passing to ensureNext. Apparently standard library functions such 
    //as std::isdigit, aren't addressable so need wrappers like this to be passed
    //as functors. 
    static bool isDigit(char c) { return std::isdigit(c); }
    static bool isAlnum(char c) { return std::isalnum(c); }
    static bool isAlpha(char c) { return std::isalpha(c); }
    static bool isSpace(char c) { return std::isspace(c); }
    static bool isPunct(char c) { return std::ispunct(c); }

    Lexeme makeLex(Lexeme::TokenType type, std::string_view c) const;
    Lexeme makeLex(Lexeme::TokenType type, char c) const;

    char get();
    char get(char);
    char peek();
    void ensureNext(char);
    void ensureNext(std::function<bool(char)>);

    std::runtime_error error(std::string_view) const;

  private:
    std::ifstream  inputStream;

    std::queue<Lexeme> lexemes;

    unsigned int   lineNumber{1};
    unsigned short lineOffset{1};
    size_t         fileOffset{0};
};

} //namespace: Jasmin

