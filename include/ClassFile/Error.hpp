#pragma once 

#include "Defs.hpp"

#include <string>
#include <variant>
#include <memory>

#include <iostream>

namespace ClassFile
{

class Error
{
  public:
    template <typename... Args>
    static Error FromFormatStr(const char* format, Args... args)
    {
      int bufsize = std::snprintf(nullptr, 0, format, args...);

      std::unique_ptr<char[]> buf{ new char[bufsize+1] };
      std::snprintf(buf.get(), bufsize+1, format, args...);

      return Error{ std::string{buf.get(), buf.get()+bufsize}};
    }

    static Error FromLiteralStr(const char* msg)
    {
      return Error{msg};
    }

    static Error FromErrCode(int code)
    {
      return Error{code};
    }
  
    bool IsCode()
    {
      return m_message.empty();
    }
  
    int GetCode()
    {
      return m_code;
    }
  
    std::string GetMessage()
    {
      return m_message;
    }

  protected:
    Error(int code) : m_code{ code } {}
    Error(std::string message) : m_message{message} {}
  
  private:
    int m_code;
    std::string m_message;
};

template <typename ValueT>
class ErrorOr
{
  public:
    template <typename T>
    ErrorOr(T&& value) : m_errorOrValue { std::forward<T>(value) } {}


    bool IsError()
    {
      return std::holds_alternative<Error>(m_errorOrValue);
    }
  
    Error& GetError()
    {
      return std::get<Error>(m_errorOrValue);
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
    std::variant<Error, ValueT> m_errorOrValue;

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
