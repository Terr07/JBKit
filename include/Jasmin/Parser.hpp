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

    uPtr<Node> parseDirective();
    uPtr<Node> parseLabel(Lexeme labelName);
    uPtr<Node> parseInstruction(Lexeme instrName);
    Instruction::ArgT parseInstrArg();
    uPtr<Value<double>> parseArithmeticExpr();

    Lexeme pop();

  private:
    std::queue<Lexeme> lexemes;
};


} //namespace: Jasmin
