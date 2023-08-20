#include "ClassFile/ConstantPool.hpp"

#include <map>
#include <cassert>

namespace ClassFile
{

std::string_view CPInfo::GetTypeName(Type type)
{
  using namespace std::literals;

  static std::map<Type, std::string_view> typeNames = 
  {
    {Type::Class,              "Class"sv},
    {Type::Fieldref,           "Fieldref"sv},
    {Type::Methodref,          "Methodref"sv},
    {Type::InterfaceMethodref, "InterfaceMethodref"sv},
    {Type::String,             "String"sv},
    {Type::Integer,            "Integer"sv},
    {Type::Float,              "Float"sv},
    {Type::Long,               "Long"sv},
    {Type::Double,             "Double"sv},
    {Type::NameAndType,        "NameAndType"sv},
    {Type::UTF8,               "UTF8"sv},
    {Type::MethodHandle,       "MethodHandle"sv},
    {Type::MethodType,         "MethodType"sv},
    {Type::InvokeDynamic,      "InvokeDynamic"sv},
  };

  auto itr = typeNames.find(type);

  assert(itr != typeNames.end());
  return std::get<1>(*itr);
}

std::string_view CPInfo::GetName() const
{
  return CPInfo::GetTypeName(this->GetType());
}

CPInfo::Type CPInfo::GetType() const
{
  return this->m_type;
}

ConstantPool::ConstantPool(U16 n) 
{
  //constants use 1 based indexing, so we ignore the 0th index
  m_pool.emplace_back( std::unique_ptr<CPInfo>{nullptr} );
}

void ConstantPool::Reserve(U16 n) 
{
  m_pool.reserve(n);
}

std::string_view ConstantPool::GetConstNameOrTypeStr(U16 index) const
{
  auto ptr = m_pool[index].get();

  if(ptr == nullptr)
    return "UNINITIALIZED";

  if(ptr->GetType() == CPInfo::Type::UTF8)
  {
    auto utf8Info = static_cast<UTF8Info*>(ptr);
    return utf8Info->String;
  }

  if(ptr->GetType() == CPInfo::Type::String)
  {
    auto stringInfo = static_cast<StringInfo*>(ptr);
    return this->GetConstNameOrTypeStr(stringInfo->StringIndex);
  }

  if(ptr->GetType() == CPInfo::Type::Class)
  {
    auto classInfo = static_cast<ClassInfo*>(ptr);
    return this->GetConstNameOrTypeStr(classInfo->NameIndex);
  }

  //TODO: implement for more types

  return "";
}

void ConstantPool::Add(std::unique_ptr<CPInfo>&& info) 
{
  m_pool.emplace_back(std::move(info));
}

void ConstantPool::Add(CPInfo* info) 
{
  m_pool.emplace_back( std::unique_ptr<CPInfo>{info} ); 
}

U16 ConstantPool::GetSize() const
{
  return static_cast<U16>(m_pool.size());
}

U16 ConstantPool::GetCount() const
{
  return this->GetSize() + 1;
}

} //namespace ClassFile
