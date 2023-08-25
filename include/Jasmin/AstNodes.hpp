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
struct ImmediateValue final : public Value<T>
{
  T Value;

  ImmediateValue(T value) : Value{value} {}

  T GetValue() const override { return Value; }

  std::string ToString() const 
  { 
    std::stringstream ss;
    ss << "ImmediateValue{";
    ss << Value;
    ss << '}';
    return ss.str();
  }
};

enum class ArithmeticOperation
{
  Add,
  Sub,
  Div,
  Mul
};

template <typename LhsT, typename RhsT, typename CommonType = typename std::common_type<LhsT, RhsT>::type >
struct BinaryExpression final: public Value<CommonType>
{
  ArithmeticOperation Op;
  uPtr< Value<LhsT> > Lhs;
  uPtr< Value<RhsT> > Rhs;

  BinaryExpression(ArithmeticOperation op, Value<LhsT>* lhs, Value<RhsT>* rhs) 
    : Op{op}, Lhs{lhs}, Rhs{rhs} {}

  CommonType GetValue() const override
  {
    switch(this->Op)
    {
      using OP = ArithmeticOperation;
      case OP::Add: return this->Lhs->GetValue() + this->Rhs->GetValue();
      case OP::Sub: return this->Lhs->GetValue() - this->Rhs->GetValue();
      case OP::Div: return this->Lhs->GetValue() / this->Rhs->GetValue();
      case OP::Mul: return this->Lhs->GetValue() * this->Rhs->GetValue();

      default: throw std::runtime_error{"unhandled arithmetic operation type"};
    };
  }

  virtual std::string ToString() const 
  { 
    static std::unordered_map<ArithmeticOperation, std::string_view> ops
    {
      {ArithmeticOperation::Add, "+"},
      {ArithmeticOperation::Sub, "-"},
      {ArithmeticOperation::Div, "/"},
      {ArithmeticOperation::Mul, "*"},
    };

    std::stringstream ss;
    ss << "BinaryExpression{ ";
    ss << Lhs->ToString();
    ss << ' ';
    ss << ops[this->Op];
    ss << ' ';
    ss << Rhs->ToString();
    ss << " }";
    return ss.str();
  }

};


} //namespace: AstNodes
} //namespace: Jasmin
