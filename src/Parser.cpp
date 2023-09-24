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
  while(lexemes.front().Type == Lexeme::TokenType::Newline)
  {
    lexemes.pop();

    if(lexemes.empty())
      return false;
  }

  return true;
}

uPtr<Node> Parser::ParseNext()
{
  //skip newlines
  while(lexemes.front().Type == Lexeme::TokenType::Newline)
    pop();

  if(lexemes.front().Type == Lexeme::TokenType::Dot)
    return parseDirective();

  Lexeme firstToken = pop();

  if(lexemes.empty())
    return parseInstruction(firstToken);

  Lexeme secondToken = lexemes.front();

  if(secondToken.Type == Lexeme::TokenType::Colon)
    return parseLabel(firstToken);
  else
    return parseInstruction(firstToken);

  throw std::runtime_error{"Unknown top level expression encountered."};
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
  Lexeme nextLex = pop();

  if(nextLex.Type != Lexeme::TokenType::Dot)
  {
    throw std::runtime_error{ fmt::format(
        "parseDirective expected dot as first lexeme but got '{}'", nextLex.Value
        )};
  }

  Lexeme name = pop();

  std::vector<std::string> params;

  while(lexemes.front().Type != Lexeme::TokenType::Newline)
  {
    Lexeme param = pop();
    params.push_back(param.Value);
  }

  //pop final newline or EOF
  pop();

  return std::make_unique<Directive>(name.Value, std::move(params));
}

uPtr<Node> Parser::parseLabel(Lexeme labelName) 
{
  Lexeme colon = pop();

  if(colon.Type != Lexeme::TokenType::Colon)
    throw std::runtime_error{"parseLabel called but second lexeme wasn't a colon \":\""};

  Lexeme newline = pop();

  if(newline.Type != Lexeme::TokenType::Newline)
    throw std::runtime_error{"parseLabel called but third lexeme wasn't a newline\":\""};

  uPtr<Label> pNode = std::make_unique<Label>(labelName.Value);

  while(lexemes.front().Type != Lexeme::TokenType::Newline)
    pNode->Body.emplace_back(this->ParseNext());

  //pop final newline or EOF
  pop();

  return pNode;
}

uPtr<Node> Parser::parseInstruction(Lexeme instrName) 
{
  auto pNode = std::make_unique<Instruction>(instrName.Value);

  while(lexemes.front().Type != Lexeme::TokenType::Newline)
    pNode->Args.emplace_back(parseInstrArg());

  pop(); //pop newline or EOF after args

  return pNode;
}

Instruction::ArgT Parser::parseInstrArg()
{
  Lexeme nextLex = lexemes.front();

  switch(nextLex.Type)
  {
    case Lexeme::TokenType::Symbol:
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
        throw std::runtime_error{ fmt::format(
            "Failed to parse instruction argument, encountered {}.", nextLex.GetTypeString())};
      }
  }
}

Lexeme Parser::pop()
{
  if(lexemes.empty())
    throw std::runtime_error{"Ran out of lexemes."};

  Lexeme lexeme{lexemes.front()};
  lexemes.pop();


  return lexeme;
}
