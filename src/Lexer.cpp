#include "Jasmin/Lexer.hpp"

#include <fmt/core.h>

using namespace Jasmin;

Lexeme::Lexeme(TokenType type, std::string val)
: Type{type}, Value{val} {}

Lexeme::Lexeme(TokenType type, char val)
: Type{type}, Value{val} {}

std::string_view Lexeme::GetTypeString() const
{
  static std::unordered_map<TokenType, std::string_view> names =
  {
    {TokenType::Symbol,             "Symbol"},
    {TokenType::Keyword,            "Keyword"},
    {TokenType::StringLiteral,      "StringLiteral"},
    {TokenType::NumericLiteral,     "NumericLiteral"},
    {TokenType::ArithmeticOperator, "ArithmeticOperator"},
    {TokenType::Newline,            "Newline"},
    {TokenType::Colon,              "Colon"},
    {TokenType::Dot,                "Dot"},
    {TokenType::Bracket,            "Bracket"},
    {TokenType::Brace,              "Brace"},
    {TokenType::Paren,              "Paren"},
  };

  return names[this->Type];
}

double Lexeme::GetNumericValue() const
{
  return std::stod(this->Value);
}

Lexer::Lexer(const char* file) 
: inputStream{file, std::ios::binary} 
{
  if(!inputStream.good())
    throw std::runtime_error{"failed to open file."};
}

bool Lexer::HasMoreAfterSkip() 
{
  skipWhitespace();
  skipComments();
  return peek() != EOF;
}

Lexeme Lexer::LexNext()
{
  skipWhitespace();
  skipComments();

  char ch = peek();

  if(ch == '.')
    return makeLex(Lexeme::TokenType::Dot, get());

  if(ch == ':')
    return makeLex(Lexeme::TokenType::Colon, get());

  if(ch == '\n' || ch == EOF)
    return makeLex(Lexeme::TokenType::Newline, get());

  if(ch == '+' || ch == '-' || ch == '/' || ch == '*')
    return makeLex(Lexeme::TokenType::ArithmeticOperator, get());

  if(ch == '(' || ch == ')')
    return makeLex(Lexeme::TokenType::Paren, get());

  if(ch == '[' || ch == ']')
    return makeLex(Lexeme::TokenType::Bracket, get());

  if(ch == '{' || ch == '}')
    return makeLex(Lexeme::TokenType::Brace, get());

  if(ch == '"')
    return lexStringLiteral();

  if(isDigit(ch))
    return lexNumericLiteral();

  if(isAlpha(ch))
    return lexString();

  throw error(fmt::format("encountered unknown lexeme value \"{}\"", ch));
}

std::queue<Lexeme> Lexer::LexAll()
{
  std::queue<Lexeme> lexemes;

  while(this->HasMoreAfterSkip())
    lexemes.emplace(this->LexNext());

  return lexemes;
}

void Lexer::skipWhitespace()
{
  while(isSpace(peek())
      && peek() != '\n')
  {
    get();
  }
}

void Lexer::skipComments()
{
  if(peek() != ';')
    return;

  get();

  while(peek() != '\n' && peek() != EOF)
    get();
}

Lexeme Lexer::lexStringLiteral()
{
  ensureNext('"');
  get();

  std::string str;

  while(peek() != '"' && peek() != EOF)
    str += get();

  ensureNext('"');
  get();

  return makeLex(Lexeme::TokenType::StringLiteral, str);
}

Lexeme Lexer::lexNumericLiteral()
{
  ensureNext(isDigit);

  std::string numStr;

  while(isDigit(peek()))
    numStr += get();

  return makeLex(Lexeme::TokenType::NumericLiteral, numStr);
}

Lexeme Lexer::lexString()
{
  ensureNext(isAlpha);

  std::string str;

  while((isAlnum(peek())
        || isPunct(peek()))
      && peek() != ':')
  {
    str += get();
  }

  if(isKeyword(str))
    return makeLex(Lexeme::TokenType::Keyword, str);

  return makeLex(Lexeme::TokenType::Symbol, str);
}

bool Lexer::isKeyword(std::string_view str) const
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

Lexeme Lexer::makeLex(Lexeme::TokenType type, std::string c) const
{
  auto lexeme = Lexeme{type, c};

  lexeme.Info.LineNumber = lineNumber;
  lexeme.Info.LineOffset = lineOffset - c.length();
  lexeme.Info.FileOffset = fileOffset - c.length();

  return lexeme;
}

Lexeme Lexer::makeLex(Lexeme::TokenType type, char c) const
{
  return makeLex(type, std::string{c});
}


char Lexer::get()
{
  char ch = inputStream.get();
  ++lineOffset;
  ++fileOffset;

  if(ch == '\n')
  {
    ++lineNumber;
    lineOffset = 1;
  }

  return ch;
}

char Lexer::peek()
{
  return inputStream.peek();
}

void Lexer::ensureNext(char next)
{
  if(peek() != next)
    throw error(fmt::format("encountered '{}' when '{}' was expected", peek(), next));
}

void Lexer::ensureNext(std::function<bool(char)> isWhatsExpected)
{
  if(!isWhatsExpected(peek()))
    throw error(fmt::format("encountered unexpected lexeme value '{}'", peek()));
}

std::runtime_error Lexer::error(std::string message) const
{
  return std::runtime_error{fmt::format("Lexer error: {} on line {}", message, lineNumber)};
}

