#pragma once

#include "Defs.hpp"
#include "Error.hpp"

#include <vector>
#include <memory>
#include <string>
#include <string_view>

namespace ClassFile
{

struct CPInfo
{
  enum class Type : U8
  {
    Class              = 7,
    Fieldref           = 9,
    Methodref          = 10,
    InterfaceMethodref = 11,
    String             = 8,
    Integer            = 3,
    Float              = 4,
    Long               = 5,
    Double             = 6,
    NameAndType        = 12,
    UTF8               = 1,
    MethodHandle       = 15,
    MethodType         = 16,
    InvokeDynamic      = 18,
  };
  static std::string_view GetTypeName(Type type);

  std::string_view GetName() const;
  Type GetType() const;

  virtual ~CPInfo() = default;

  protected:
  CPInfo(Type type) : m_type{type} {}

  private:
  const Type m_type;
};

struct ClassInfo : public CPInfo
{
  ClassInfo() : CPInfo(Type::Class) {}
  U16 NameIndex;
};

struct FieldrefInfo : public CPInfo
{
  FieldrefInfo() : CPInfo(Type::Fieldref) {}
  U16 ClassIndex;
  U16 NameAndTypeIndex;
};

struct MethodrefInfo : public CPInfo
{
  MethodrefInfo() : CPInfo(Type::Methodref) {}
  U16 ClassIndex;
  U16 NameAndTypeIndex;
};

struct InterfaceMethodrefInfo : public CPInfo
{
  InterfaceMethodrefInfo() : CPInfo(Type::InterfaceMethodref) {}
  U16 ClassIndex;
  U16 NameAndTypeIndex;
};

struct StringInfo : public CPInfo
{
  StringInfo() : CPInfo(Type::String) {}
  U16 StringIndex;
};

struct IntegerInfo : public CPInfo
{
  IntegerInfo() : CPInfo(Type::Integer) {}
  U32 Bytes;
};

struct FloatInfo : public CPInfo
{
  FloatInfo() : CPInfo(Type::Float) {}
  U32 Bytes;
};

struct LongInfo : public CPInfo
{
  LongInfo() : CPInfo(Type::Long) {}
  U32 HighBytes;
  U32 LowBytes;
};

struct DoubleInfo : public CPInfo
{
  DoubleInfo() : CPInfo(Type::Double) {}
  U32 HighBytes;
  U32 LowBytes;
};

struct NameAndTypeInfo : public CPInfo
{
  NameAndTypeInfo() : CPInfo(Type::NameAndType) {}
  U16 NameIndex;
  U16 DescriptorIndex;
};

struct UTF8Info : public CPInfo
{
  UTF8Info() : CPInfo(Type::UTF8) {}
  std::string String;
};

struct MethodHandleInfo : public CPInfo
{
  MethodHandleInfo() : CPInfo(Type::MethodHandle) {}
  U8 ReferenceKind;
  U16 ReferenceIndex;
};

struct MethodTypeInfo : public CPInfo
{
  MethodTypeInfo() : CPInfo(Type::MethodType) {}
  U16 DescriptorIndex;
};

struct InvokeDynamicInfo : public CPInfo
{
  InvokeDynamicInfo() : CPInfo(Type::InvokeDynamic) {}
  U16 BootstrapMethodAttrIndex;
  U16 NameAndTypeIndex;
};

class ConstantPool
{
  public:
    ConstantPool(U16 n=0);
    void Reserve(U16 n);

    //Returns either the name of the constant, if the given const type has a 
    //name (such as in the case of UTF8 or any class that has a name field 
    //pointing to a UTF8), OR returns the stringified version of the const type
    std::string_view GetConstNameOrTypeStr(U16 index) const;

    void Add(std::unique_ptr<CPInfo>&& info);
    void Add(CPInfo* info);

    template <class T = CPInfo>
    ErrorOr< std::reference_wrapper<T> > Get(U16 index) const
    {
      if(index >= m_pool.size() || index == 0)
      {
        return Error::FromFormatStr("ConstantPool.Get(%u) failed because given index is outside the pools range (1-%u)", 
            static_cast<unsigned int>(index), 
            static_cast<unsigned int>(this->GetCount()));
      }

      if (m_pool[index].get() == nullptr)
      {
        return Error::FromFormatStr("ConstantPool.Get(%u) failed because constant at given index is nullptr", 
            static_cast<unsigned int>(index));
      }

      T* ptr = dynamic_cast<T*>( m_pool[index].get() );

      if (ptr == nullptr)
      {
        return Error::FromFormatStr("ConstantPool.Get<%s>(%u) failed to convert CPInfo to requested type (dynamic cast failed)", 
            typeid(T).name(), 
            static_cast<unsigned int>(index));
      }

      return *ptr;
    }

    CPInfo* operator[](U16 index)
    {
      return m_pool[index].get();
    }

    const CPInfo* operator[](U16 index) const
    {
      return m_pool[index].get();
    }

    U16 GetSize() const;
    U16 GetCount() const;
  private:
    std::vector< std::unique_ptr<CPInfo> > m_pool;
};

}  //namespace ClassFile
