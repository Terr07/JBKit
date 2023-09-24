#pragma once

#include "Common.hpp"

namespace Jasmin
{
namespace AstNodes
{

struct Node 
{
  virtual std::string ToString() const = 0;
  virtual ~Node() = default;
};

template <typename T>
struct Value: public Node
{
  virtual T GetValue() const = 0;
  virtual std::string ToString() const = 0;
};

template <typename T>
struct ImmediateValue : public Value<T>
{
  T Value;

  ImmediateValue(T value) : Value{value} {}

  T GetValue() const override { return Value; }

  std::string ToString() const
  {
    std::stringstream ss;
    ss << "ImmediateValue{";
    ss << this->Value;
    ss << '}';
    return ss.str();
  }
};

struct Directive: public Node
{
  std::string Name;
  std::vector<std::string> Parameters;

  Directive(std::string name, std::vector<std::string> params);
  std::string ToString() const override;
};

struct Instruction: public Node
{
  std::string Name;

  using ArgT = std::variant<uPtr<Value<std::string>>, uPtr<Value<double>>>;
  std::vector<ArgT> Args;

  Instruction(std::string name);
  std::string ToString() const override;
};

struct Label: public Node
{
  std::string Name;
  std::vector< uPtr<Node> > Body;

  Label(std::string name);
  std::string ToString() const override;
};


} //namespace: AstNodes
} //namespace: Jasmin
