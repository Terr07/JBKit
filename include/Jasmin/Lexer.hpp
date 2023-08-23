#pragma once

#include <string_view>
#include <vector>
#include <fstream>
#include <map>
#include <exception>

namespace Jasmin
{

struct Lexeme
{
  enum LexType
  {
    Symbol,
    String,
    Number,
    ArithmeticOperator,
    Newline,
    Colon,
    Dot,
    END
  } Type;

  std::string Value;

  Lexeme(LexType type, std::string val): Type{type}, Value{val} {}
  Lexeme(LexType type, int val): Type{type}, Value{static_cast<char>(val)} {}

  std::string_view GetTypeString() const
  {
    static std::map<LexType, std::string_view> names =
    {
      {LexType::Symbol,             "Symbol"},
      {LexType::String,             "String"},
      {LexType::Number,             "Number"},
      {LexType::ArithmeticOperator, "ArithmeticOperator"},
      {LexType::Newline,            "Newline"},
      {LexType::Colon,              "Colon"},
      {LexType::Dot,                "Dot"},
    };

    return names[this->Type];
  }
};

class Lexer
{
  public:
    Lexer(const char* file) : inputStream{file, std::ios::binary} 
    {
      if(!inputStream.good())
        throw std::runtime_error{"Failed to open file."};
    }

    bool HasMore() 
    {
      return inputStream.peek() != EOF;
    }

    Lexeme LexNext()
    {
      char ch;

      //skip whitespace
      while(true)
      {
        if(!HasMore())
          throw std::runtime_error{"nomore"};

        ch = inputStream.peek();

        if(ch != ' ' && ch != '\t')
          break;

        inputStream.get();
      }

      //skip comments
      if(ch == ';')
      {
        inputStream.get();

        while(true)
        {
          if(!this->HasMore())
            throw std::runtime_error{"nomore"};

          ch = inputStream.peek();

          if(ch == '\n')
            break;

          inputStream.get();
        }
      }

      if(ch == '.')
        return {Lexeme::LexType::Dot, inputStream.get()};

      if(ch == ':')
        return {Lexeme::LexType::Colon, inputStream.get()};

      if(ch == '\n')
        return {Lexeme::LexType::Newline, inputStream.get()};

      if(ch == '+' || ch == '-' || ch == '/' || ch == '*')
        return {Lexeme::LexType::ArithmeticOperator, inputStream.get()};

      if(ch == '"')
      {
        inputStream.get();

        std::string str;
        while(true)
        {
          if(!this->HasMore())
            throw std::runtime_error{"unable to lex string: no closing \" encountered before end of input."};

          ch = inputStream.get();

          if(ch == '"')
            break;

          str += ch;
        }

        return {Lexeme::LexType::String, str};
      }

      if(std::isdigit(ch))
      {
        std::string numStr;
        numStr += inputStream.get();

        while(this->HasMore())
        {
          ch = inputStream.peek();

          if(!std::isdigit(ch))
            break;

          numStr += inputStream.get();
        }

        return {Lexeme::LexType::Number, numStr};
      }


      //TODO: while alphanumerical
      std::string str;
      inputStream >> str;

      return {Lexeme::LexType::Symbol, str};
    }

    std::vector<Lexeme> LexAll()
    {
      std::vector<Lexeme> lexemes;

      while(this->HasMore())
        lexemes.push_back(this->LexNext());

      return lexemes;
    }

  private:
    std::ifstream inputStream;

};

} //namespace: Jasmin

