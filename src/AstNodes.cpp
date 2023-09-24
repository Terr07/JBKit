#include "Jasmin/AstNodes.hpp"

using namespace Jasmin::AstNodes;

Directive::Directive(std::string name, std::vector<std::string> params)
  : Name{name}, Parameters{params}  
{

}

std::string Directive::ToString() const
{
  std::stringstream ss;
  ss << "Directive{ Name: ";
  ss << this->Name << ", Params: ";

  for(std::string_view param : this->Parameters)
    ss << param << ' ';

  ss << '}';
  return ss.str();
}

Instruction::Instruction(std::string name) 
  : Name{name} 
{}

std::string Instruction::ToString() const 
{
  std::stringstream ss;
  ss << "Instruction{ ";
  ss << this->Name;

  if(Args.size() > 0)
  {
    ss << ", Args: ";

    for(const auto& argVar : this->Args)
    {
      std::visit([&ss](const auto& pStr){
          ss << pStr->ToString() << ' ';
          },argVar);
    }
  }
  else
  {
    ss << ' ';
  }

  ss << "}";
  return ss.str();
}

Label::Label(std::string name) 
: Name{name} 
{}


std::string Label::ToString() const 
{
  std::stringstream ss;
  ss << "Label{ Name: " << this->Name << ", Body: {";

  for(const auto& pNode: this->Body)
    ss << "\n   " << pNode->ToString();

  if(Body.size() > 0 ) 
    ss << '\n';

  ss << "}}";
  return ss.str();
}



