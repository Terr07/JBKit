#pragma once 

#include "Defs.hpp"

#include <string>
#include <variant>
#include <memory>

#include <iostream>

namespace ClassFile
{

struct Error
{
  std::string What;
};

struct ParseError : public Error
{
  size_t pos;
};

template <typename ValueT, typename ErrorT=Error>
class ErrorOr
{
  public:
    template <typename T>
    ErrorOr(T&& value) : m_errorOrValue { std::forward<T>(value) } {}


    bool IsError()
    {
      return std::holds_alternative<ErrorT>(m_errorOrValue);
    }
  
    ErrorT& GetError()
    {
      return std::get<ErrorT>(m_errorOrValue);
    }
  
    ValueT& Get()
    {
      return std::get<ValueT>(m_errorOrValue);
    }
  
    ValueT Release()
    {
      return std::move(std::get<ValueT>(m_errorOrValue));
    }
  
  private:
    std::variant<ErrorT, ValueT> m_errorOrValue;

};

template <>
class ErrorOr<void> : public ErrorOr<std::monostate>
{
  public:
  using ErrorOr<std::monostate>::ErrorOr;
  ErrorOr() : ErrorOr<std::monostate>{ std::monostate{} } {}
};

using NoError = std::monostate;

} //namespace ClassFile
