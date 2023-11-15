#include "ClassFile/ConstantPool.hpp"
#include "Util/Error.hpp"

#include <fmt/core.h>

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

void ConstantPool::Reserve(U16 n) 
{
  m_pool.reserve(n);
}

template <typename T>
static ErrorOr<std::string_view> getName(U16 index, const ConstantPool& cp)
{
  auto errOrPtr = cp.Get<T>(index);
  VERIFY(errOrPtr);

  auto errOrSV = cp.LookupString(errOrPtr.Get()->NameIndex);
  VERIFY(errOrSV);

  return errOrSV.Get();
}

template <typename T>
static ErrorOr<std::string_view> getNameByNameAndTypeIndex(U16 index, const ConstantPool& cp)
{
  auto errOrPtr = cp.Get<T>(index);
  VERIFY(errOrPtr);

  return getName<NameAndTypeInfo>(errOrPtr.Get()->NameAndTypeIndex, cp);
}

ErrorOr<std::string_view> ConstantPool::LookupString(U16 index) const
{
  TRY(ensureValid(index));

  switch(m_pool[index]->GetType())
  {
    case CPInfo::Type::String:
    case CPInfo::Type::UTF8:
      return lookupStringOrUTF8(index);

    case CPInfo::Type::Class:
      return getName<ClassInfo>(index, *this);
    case CPInfo::Type::NameAndType:
      return getName<NameAndTypeInfo>(index, *this);

    case CPInfo::Type::Fieldref:
      return getNameByNameAndTypeIndex<FieldrefInfo>(index, *this);

    case CPInfo::Type::Methodref:
      return getNameByNameAndTypeIndex<MethodrefInfo>(index, *this);

    case CPInfo::Type::InterfaceMethodref:
      return getNameByNameAndTypeIndex<InterfaceMethodrefInfo>(index, *this);

    case CPInfo::Type::InvokeDynamic:
      return getNameByNameAndTypeIndex<InvokeDynamicInfo>(index, *this);
  }

  return Error{fmt::format("ConstantPool: Failed to lookup name "
      "string for constant info entry and index {} (type: {})", 
      index, m_pool[index]->GetName())};
}

template <typename T>
static ErrorOr<std::string_view> getDescriptor(U16 index, const ConstantPool& cp)
{
  auto errOrPtr = cp.Get<T>(index);
  VERIFY(errOrPtr);

  auto errOrSV = cp.LookupString(errOrPtr.Get()->DescriptorIndex);
  VERIFY(errOrSV);

  return errOrSV.Get();
}

template <typename T>
static ErrorOr<std::string_view> getDescriptorByNameAndTypeIndex(U16 index, const ConstantPool& cp)
{
  auto errOrPtr = cp.Get<T>(index);
  VERIFY(errOrPtr);

  return getDescriptor<NameAndTypeInfo>(errOrPtr.Get()->NameAndTypeIndex, cp);
}

ErrorOr<std::string_view> ConstantPool::LookupDescriptor(U16 index) const
{
  TRY(ensureValid(index));

  switch(m_pool[index]->GetType())
  {
    case CPInfo::Type::MethodType:
      return getDescriptor<MethodTypeInfo>(index, *this);

    case CPInfo::Type::NameAndType:
      return getDescriptor<NameAndTypeInfo>(index, *this);

    case CPInfo::Type::Fieldref:
      return getDescriptorByNameAndTypeIndex<FieldrefInfo>(index, *this);

    case CPInfo::Type::Methodref:
      return getDescriptorByNameAndTypeIndex<MethodrefInfo>(index, *this);

    case CPInfo::Type::InterfaceMethodref:
      return getDescriptorByNameAndTypeIndex<InterfaceMethodrefInfo>(index, *this);

    case CPInfo::Type::InvokeDynamic:
      return getDescriptorByNameAndTypeIndex<InvokeDynamicInfo>(index, *this);
  }

  return Error{fmt::format("ConstantPool: Failed to lookup descriptor "
      "string for constant info entry and index {} (type: {})", 
      index, m_pool[index]->GetName())};
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

CPInfo* ConstantPool::operator[](U16 index) 
{
  --index;
  if(index > this->GetSize())
    return nullptr;

  return m_pool[index].get();
}

const CPInfo* ConstantPool::operator[](U16 index) const
{
  --index;
  if(index > this->GetSize())
    return nullptr;

  return m_pool[index].get();
}

ErrorOr<std::string_view> ConstantPool::lookupStringOrUTF8(U16 index) const
{
  TRY(ensureValid(index));

  if(m_pool[index]->GetType() == CPInfo::Type::String)
    return LookupString(this->Get<StringInfo>(index).Get()->StringIndex);

  auto errOrPtr = this->Get<UTF8Info>(index);
  VERIFY(errOrPtr, fmt::format("ConstantPool: Failed to lookup string value for "
        "constant info entry at index {} (type: {})", index, m_pool[index]->GetName()));

  return std::string_view{errOrPtr.Get()->String};
}

ErrorOr<void> ConstantPool::ensureValid(U16 index) const
{
  if(index >= m_pool.size() || index == 0)
  {
    return Error{fmt::format("ConstantPool: "
        "out-of-bounds access at index {}, valid index range for "
        "pool is 1-{}", index, this->GetCount())};
  }

  if(m_pool[index].get() == nullptr)
    return Error{fmt::format("ConstantPool: pool[{}] is nullptr", index)};

  return NoError{};
}

Error ConstantPool::failedCastError(U16 index, std::string_view castToName) const
{
  return Error{fmt::format("ConstantPool: " 
      "invalid type cast access at index {}"
      ", failed to cast to type \"{}\"", index, castToName)};
}

} //namespace ClassFile
