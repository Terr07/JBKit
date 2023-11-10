#include "Jasmin/Assembler.hpp"

#include <fmt/core.h>

using namespace Jasmin;

bool Assembler::hasMore() 
{
  return !lexemes.empty();
}

void Assembler::parseNext()
{
  //skip newlines
  while(peek().Type == Lexeme::TokenType::Newline)
    pop();

  if(!hasMore())
    return;

  if(peek().Type == Lexeme::TokenType::Directive)
  {
    parseDirective( DirectiveTypeFromStr(pop().Value) );
    return;
  }

  Lexeme firstToken = pop();

  if(lexemes.empty())
  {
    parseInstruction(firstToken);
    return;
  }

  Lexeme secondToken = peek();

  if(secondToken.Type == Lexeme::TokenType::Colon)
  {
    parseLabel(firstToken);
    return;
  }
  else
  {
    parseInstruction(firstToken);
    return;
  }

  throw error("Unknown top level expression encountered."); 
}

void Assembler::parseDirective(DirectiveType type) 
{ 
  switch(type)
  {
  }

  throw error( fmt::format("Unimplemented directive type \"{}\"", ToString(type) ) );
}

void Assembler::parseLabel(Lexeme labelName) 
{
  throw error("parse label unimplemented.");
}

void Assembler::parseInstruction(Lexeme instrName) 
{
  throw error("parse instruction unimplemented.");
}

Lexeme Assembler::pop()
{
  if(lexemes.empty())
    throw std::runtime_error{"Ran out of lexemes."};

  Lexeme lexeme{peek()};
  lexemes.pop();


  return lexeme;
}

const Lexeme& Assembler::peek() const
{
  if(lexemes.empty())
    throw std::runtime_error{"parser attempted to peek into empty queue."};

  return lexemes.front();
}

void Assembler::ensureNext(Lexeme::TokenType expected, std::string_view parserName)
{
  if(peek().Type != expected)
  {
    throw error(fmt::format("{} encountered '{}' when '{}' was expected", 
          parserName, peek().GetTypeString(), Lexeme::GetTypeString(expected)));
  }
}

std::runtime_error Assembler::error(std::string_view message) const
{
  //TODO: this is not always the correct line & column number
  return std::runtime_error{
    fmt::format("Parser error: {} on line {} col {}", 
        message, peek().Info.LineNumber, peek().Info.LineOffset)};
}

