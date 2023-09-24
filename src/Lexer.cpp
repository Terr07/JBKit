#include "Jasmin/Lexer.hpp"

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
    throw std::runtime_error{"Failed to open file."};
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
  while(std::isspace(peek())
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
  if(peek() != '"')
    throw std::runtime_error{"failed to lex string literal: no opening \" encountered."};

  get();

  std::string str;

  while(peek() != '"' && peek() != EOF)
    str += get();

  if(get() != '"') 
    throw std::runtime_error{"failed to lex string literal: no closing \" encountered."};

  return makeLex(Lexeme::TokenType::StringLiteral, str);
}

Lexeme Lexer::lexNumericLiteral()
{
  if( !std::isdigit(peek()) )
    throw std::runtime_error{"failed to lex number literal: character encountered is not a digit."};

  std::string numStr;

  while(std::isdigit(peek()))
    numStr += get();

  return makeLex(Lexeme::TokenType::NumericLiteral, numStr);
}

Lexeme Lexer::lexString()
{
  if( !std::isalnum(peek()) )
    throw std::runtime_error{"failed to lex string: character encountered is not alnum."};

  std::string str;

  while((std::isalnum(peek())
        || std::ispunct(peek()))
      && peek() != ':')
  {
    str += get();
  }

  if(isKeyword(str))
    return makeLex(Lexeme::TokenType::Keyword, str);

  return makeLex(Lexeme::TokenType::Symbol, str);
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

