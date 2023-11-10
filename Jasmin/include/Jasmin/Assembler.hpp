#pragma once

#include <ClassFile/ClassFile.hpp>

#include "Common.hpp"
#include "Lexer.hpp"

namespace Jasmin
{

namespace CF = ClassFile;

class Assembler
{
  public:
    template <typename T, typename = std::enable_if_t< 
               std::is_same_v<std::remove_reference_t<T>, std::queue<Lexeme>>>>
    static CF::ClassFile Assemble(T&& lexs)
    {
      Assembler assembler{ std::forward<T>(lexs) };

      while(assembler.hasMore())
        assembler.parseNext();

      return std::move(assembler.classFile);
    }

    static CF::ClassFile Assemble(const char* file)
    {
      return std::move(Assembler::Assemble(Lexer::Lex(file)));
    }

  private:
    template <typename T, typename = std::enable_if_t< 
               std::is_same_v<std::remove_reference_t<T>, std::queue<Lexeme>>>>
    Assembler(T&& lexs)
    : lexemes{std::forward<T>(lexs)} 
    {}

  private:
    bool hasMore();
    void parseNext();
    void parseDirective(DirectiveType);
    void parseLabel(Lexeme labelName);
    void parseInstruction(Lexeme instrName);

    Lexeme pop();
    const Lexeme& peek() const;

    void ensureNext(Lexeme::TokenType, std::string_view);

    std::runtime_error error(std::string_view) const;

  private:
    CF::ClassFile classFile;
    std::queue<Lexeme> lexemes;
};

} //namespace: Jasmin
