#include "Jasmin/Parser.hpp"

#include <fmt/core.h>

using namespace Jasmin;


Parser::Parser(std::queue<Lexeme> lexQueue) 
: lexemes{std::move(lexQueue)} {}

bool Parser::HasMoreAfterSkip() 
{
  if(lexemes.empty())
    return false;

  //Newlines are part of some expression syntaxes but the top level parser
  //ignores them, so HasMoreAfterSkip() shouldn't return true if there are only
  //newlines left in the queue
  while(peek().Type == Lexeme::TokenType::Newline)
  {
    pop();

    if(lexemes.empty())
      return false;
  }

  return true;
}

uPtr<Node> Parser::ParseNext()
{
  //skip newlines
  while(peek().Type == Lexeme::TokenType::Newline)
    pop();

  if(peek().Type == Lexeme::TokenType::Directive)
    return parseDirective();

  Lexeme firstToken = pop();

  if(lexemes.empty())
    return parseInstruction(firstToken);

  Lexeme secondToken = peek();

  if(secondToken.Type == Lexeme::TokenType::Colon)
    return parseLabel(firstToken);
  else
    return parseInstruction(firstToken);

  throw error("Unknown top level expression encountered."); 
}

std::vector< uPtr<Node> > Parser::ParseAll()
{
  std::vector< uPtr<Node> > nodes;

  while(this->HasMoreAfterSkip())
    nodes.emplace_back(this->ParseNext());

  return nodes;
}

uPtr<Node> Parser::parseDirective() 
{ 
  ensureNext(Lexeme::TokenType::Directive, "DirectiveParser");
  Lexeme directive = pop();

  std::vector<std::string> params;

  while(peek().Type != Lexeme::TokenType::Newline)
  {
    Lexeme param = pop();
    params.push_back(param.Value);
  }

  return std::make_unique<Directive>(directive.Value, std::move(params));
}

uPtr<Node> Parser::parseLabel(Lexeme labelName) 
{
  ensureNext(Lexeme::TokenType::Colon, "LabelParser");
  pop();

  ensureNext(Lexeme::TokenType::Newline, "LabelParser");
  pop();

  uPtr<Label> pNode = std::make_unique<Label>(labelName.Value);

  while(!lexemes.empty() && peek().Type != Lexeme::TokenType::Newline)
    pNode->Body.emplace_back(this->ParseNext());

  return pNode;
}

uPtr<Node> Parser::parseInstruction(Lexeme instrName) 
{
  auto pNode = std::make_unique<Instruction>(instrName.Value);

  while(peek().Type != Lexeme::TokenType::Newline)
    pNode->Args.emplace_back(parseInstrArg());

  return pNode;
}

Instruction::ArgT Parser::parseInstrArg()
{
  Lexeme nextLex = peek();

  switch(nextLex.Type)
  {
    case Lexeme::TokenType::String:
    case Lexeme::TokenType::StringLiteral:
      return std::make_unique<ImmediateValue<std::string>>(std::move(pop().Value));

    case Lexeme::TokenType::NumericLiteral:
      return std::make_unique<ImmediateValue<double>>(pop().GetNumericValue());

    case Lexeme::TokenType::ArithmeticOperator:
    {
      if(nextLex.Value == "-")
      {
        pop();
        return std::make_unique<ImmediateValue<double>>(-(pop().GetNumericValue()));
      }
    }

    default:
    {
      throw error( fmt::format("failed to parse instruction argument \"{}\".", 
            nextLex.GetTypeString()));
    }
  }
}

Lexeme Parser::pop()
{
  if(lexemes.empty())
    throw std::runtime_error{"Ran out of lexemes."};

  Lexeme lexeme{peek()};
  lexemes.pop();


  return lexeme;
}

const Lexeme& Parser::peek() const
{
  if(lexemes.empty())
    throw std::runtime_error{"parser attempted to peek into empty queue."};

  return lexemes.front();
}

void Parser::ensureNext(Lexeme::TokenType expected, std::string_view parserName)
{
  if(peek().Type != expected)
  {
    throw error(fmt::format("{} encountered '{}' when '{}' was expected", 
          parserName, peek().GetTypeString(), Lexeme::GetTypeString(expected)));
  }
}

std::runtime_error Parser::error(std::string_view message) const
{
  return std::runtime_error{
    fmt::format("Parser error: {} on line {} col {}", 
        message, peek().Info.LineNumber, peek().Info.LineOffset)};
}

