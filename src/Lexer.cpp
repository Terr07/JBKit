#include "Jasmin/Lexer.hpp"

using namespace Jasmin;

Lexeme::Lexeme(TokenType type, std::string val)
: Type{type}, Value{val} {}

Lexeme::Lexeme(TokenType type, int val)
: Type{type}, Value{static_cast<char>(val)} {}


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
    throw std::runtime_error{"Failed to open file."};
}

bool Lexer::HasMoreAfterSkip() 
{
  skipWhitespace();
  skipComments();
  return inputStream.peek() != EOF;
}

Lexeme Lexer::LexNext()
{
  skipWhitespace();
  skipComments();

  char ch = inputStream.peek();

  if(ch == '.')
    return {Lexeme::TokenType::Dot, inputStream.get()};

  if(ch == ':')
    return {Lexeme::TokenType::Colon, inputStream.get()};

  if(ch == '\n' || ch == EOF)
    return {Lexeme::TokenType::Newline, inputStream.get()};

  if(ch == '+' || ch == '-' || ch == '/' || ch == '*')
    return {Lexeme::TokenType::ArithmeticOperator, inputStream.get()};

  if(ch == '(' || ch == ')')
    return {Lexeme::TokenType::Paren, inputStream.get()};

  if(ch == '[' || ch == ']')
    return {Lexeme::TokenType::Bracket, inputStream.get()};

  if(ch == '{' || ch == '}')
    return {Lexeme::TokenType::Brace, inputStream.get()};

  if(ch == '"')
    return lexStringLiteral();

  if(std::isdigit(ch))
    return lexNumericLiteral();

  if(std::isalnum(ch))
    return lexString();

  throw std::runtime_error{"Failed to lex unknown character encountered."};
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
  while(std::isspace(inputStream.peek())
      && inputStream.peek() != '\n')
  {
    inputStream.get();
  }
}

void Lexer::skipComments()
{
  if(inputStream.peek() != ';')
    return;

  inputStream.get();

  while(inputStream.peek() != '\n' && inputStream.peek() != EOF)
    inputStream.get();
}

Lexeme Lexer::lexStringLiteral()
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

Lexeme Lexer::lexNumericLiteral()
{
  if( !std::isdigit(inputStream.peek()) )
    throw std::runtime_error{"failed to lex number literal: character encountered is not a digit."};

  std::string numStr;

  while(std::isdigit(inputStream.peek()))
    numStr += inputStream.get();

  return Lexeme{Lexeme::TokenType::NumericLiteral, numStr};
}

Lexeme Lexer::lexString()
{
  if( !std::isalnum(inputStream.peek()) )
    throw std::runtime_error{"failed to lex string: character encountered is not alnum."};

  std::string str;

  while((std::isalnum(inputStream.peek())
        || std::ispunct(inputStream.peek()))
      && inputStream.peek() != ':')
  {
    str += inputStream.get();
  }

  if(isKeyword(str))
    return Lexeme{Lexeme::TokenType::Keyword, str};

  return Lexeme{Lexeme::TokenType::Symbol, str};
}

bool Lexer::isKeyword(std::string_view str)
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
