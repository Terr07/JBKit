#include "ClassFile/Attribute.hpp"

#include <map>
#include <cassert>

namespace ClassFile
{


using namespace std::literals;

static std::map<AttributeInfo::Type, std::string_view> typeNames =
{
  {AttributeInfo::Type::ConstantValue, "ConstantValue"sv},
  {AttributeInfo::Type::Code,          "Code"sv},
  {AttributeInfo::Type::SourceFile,    "SourceFile"sv},

  {AttributeInfo::Type::Raw, "_Raw"sv}
};

std::string_view AttributeInfo::GetTypeName(AttributeInfo::Type type) 
{
  auto itr = typeNames.find(type);

  assert(itr != typeNames.end());
  return std::get<1>(*itr);
}

ErrorOr<AttributeInfo::Type> AttributeInfo::GetType(std::string_view str) 
{
  for (auto itr : typeNames)
  {
    if (str == std::get<1>(itr))
      return std::get<0>(itr);
  }

  return Error::FromFormatStr("AttributeInfo::GetType called with unknown type name \"%.*s\"", 
      str.size(), str.data());
}

std::string_view AttributeInfo::GetName() const
{
  return AttributeInfo::GetTypeName(this->GetType());
}

AttributeInfo::Type AttributeInfo::GetType() const
{
  return m_type;
}


} //namespace ClassFile
