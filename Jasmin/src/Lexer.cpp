#include "Jasmin/Lexer.hpp"

#include <fmt/core.h>

using namespace Jasmin;

Lexeme::Lexeme(TokenType type, std::string_view val)
: Type{type}, Value{val} {}

Lexeme::Lexeme(TokenType type, char val)
: Type{type}, Value{val} {}

std::string_view Lexeme::GetTypeString() const
{
  return GetTypeString(this->Type);
}

std::string_view Lexeme::GetTypeString(TokenType type)
{
  std::unordered_map<TokenType, std::string_view> names =
  {
    {TokenType::String,             "String"},
    {TokenType::Keyword,            "Keyword"},
    {TokenType::Directive,          "Directive"},
    {TokenType::StringLiteral,      "StringLiteral"},
    {TokenType::NumericLiteral,     "NumericLiteral"},
    {TokenType::ArithmeticOperator, "ArithmeticOperator"},
    {TokenType::Newline,            "Newline"},
    {TokenType::Colon,              "Colon"},
    {TokenType::Bracket,            "Bracket"},
    {TokenType::Brace,              "Brace"},
    {TokenType::Paren,              "Paren"},
  };

  return names[type];
}

double Lexeme::GetNumericValue() const
{
  return std::stod(this->Value);
}

static const std::unordered_map<std::string_view, DirectiveType> strDirMap =
{
  {"catch"     , DirectiveType::Catch     },
  {"class"     , DirectiveType::Class     },
  {"end"       , DirectiveType::End       },
  {"field"     , DirectiveType::Field     },
  {"implements", DirectiveType::Implements},
  {"interface" , DirectiveType::Interface },
  {"limit"     , DirectiveType::Limit     },
  {"line"      , DirectiveType::Line      },
  {"method"    , DirectiveType::Method    },
  {"source"    , DirectiveType::Source    },
  {"super"     , DirectiveType::Super     },
  {"throws"    , DirectiveType::Throws    },
  {"var"       , DirectiveType::Var       },
};

static const std::unordered_map<DirectiveType, std::string_view> dirStrMap = 
{
  {DirectiveType::Catch     , "catch"     },
  {DirectiveType::Class     , "class"     },
  {DirectiveType::End       , "end"       },
  {DirectiveType::Field     , "field"     },
  {DirectiveType::Implements, "implements"},
  {DirectiveType::Interface , "interface" },
  {DirectiveType::Limit     , "limit"     },
  {DirectiveType::Line      , "line"      },
  {DirectiveType::Method    , "method"    },
  {DirectiveType::Source    , "source"    },
  {DirectiveType::Super     , "super"     },
  {DirectiveType::Throws    , "throws"    },
  {DirectiveType::Var       , "var"       },
};

bool Jasmin::IsDirectiveType(std::string_view str)
{
  return strDirMap.find(str) != strDirMap.end();
}

DirectiveType Jasmin::DirectiveTypeFromStr(std::string_view str)
{
  return DirectiveType{strDirMap.at(str)};
}

std::string Jasmin::ToString(DirectiveType dir)
{
  return std::string{dirStrMap.at(dir)};
}

bool Jasmin::IsKeyword(std::string_view str) 
{
  std::unordered_set<std::string_view> keywords = 
  {
    "public",
    "private",
    "protected",
    "static",
    "volatile",
    "transient",
    "final",
    "abstract",
    "native",
    "synchronized",
  };

  return keywords.find(str) != keywords.end();
}

std::queue<Lexeme> Lexer::Lex(const char* file)
{
  Lexer lexer{file};

  while(lexer.hasMoreAfterSkip())
    lexer.lexemes.emplace(lexer.lexNext());

  return std::move(lexer.lexemes);
}

Lexer::Lexer(const char* file) 
: inputStream{file, std::ios::binary} 
{
  if(!inputStream.good())
    throw std::runtime_error{"failed to open file."};
}

bool Lexer::hasMoreAfterSkip() 
{
  skipWhitespace();
  skipComments();
  return peek() != EOF;
}

Lexeme Lexer::lexNext()
{
  skipWhitespace();
  skipComments();

  char ch = peek();

  if(ch == '.')
  {
    get();
    Lexeme next = lexNext();

    if(next.Type != Lexeme::TokenType::String)
      throw error("Expected string token as directive name after \'.\' character.");

    if(!Jasmin::IsDirectiveType(next.Value))
      throw error( fmt::format("\"{}\" is not a valid directive.", next.Value));

    return makeLex(Lexeme::TokenType::Directive, next.Value);
  }

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
  {
    Lexeme strLex = lexString();

    if(Jasmin::IsKeyword(strLex.Value))
      return makeLex(Lexeme::TokenType::Keyword, strLex.Value);
    else
      return strLex;
  }

  throw error(fmt::format("encountered unknown lexeme value \"{}\"", ch));
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
  get('"');

  std::string str;

  while(peek() != '"' && peek() != EOF)
    str += get();

  get('"');

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

  return makeLex(Lexeme::TokenType::String, str);
}

Lexeme Lexer::makeLex(Lexeme::TokenType type, std::string_view c) const
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

char Lexer::get(char c)
{
  ensureNext(c);
  return get();
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

std::runtime_error Lexer::error(std::string_view message) const
{
  return std::runtime_error{fmt::format("Lexer error: {} on line {} col {}", 
      message, lineNumber, lineOffset)};
}

