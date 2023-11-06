#pragma once

#include "Common.hpp"
#include "Lexer.hpp"
#include "AstNodes.hpp"


namespace Jasmin
{

using namespace AstNodes;

class Parser
{
  public:
    Parser(std::queue<Lexeme> lexQueue);

    bool HasMoreAfterSkip();
    uPtr<Node> ParseNext();
    std::vector< uPtr<Node> > ParseAll();

  private:
    std::queue<Lexeme> lexemes;

    uPtr<Node> parseDirective();
    uPtr<Node> parseLabel(Lexeme labelName);
    uPtr<Node> parseInstruction(Lexeme instrName);
    Instruction::ArgT parseInstrArg();

    Lexeme pop();
    const Lexeme& peek() const;

    void ensureNext(Lexeme::TokenType, std::string_view);

    std::runtime_error error(std::string_view) const;
};


} //namespace: Jasmin
