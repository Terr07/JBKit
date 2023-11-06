#pragma once

#include "Instruction.hpp"
#include "Defs.hpp"
#include "Error.hpp"

#include <string_view>
#include <cassert>
#include <vector>

namespace ClassFile
{


struct AttributeInfo
{
  public:
    enum class Type
    {
      ConstantValue,
      Code,

      SourceFile,
  
      Raw, //Non standard 
    };
  
    static ErrorOr<Type> GetType(std::string_view);
    static std::string_view GetTypeName(Type);
  
    std::string_view GetName() const;
    Type GetType() const;
  
    U16 NameIndex;

    //NOTE: doesn't include potentially required padding
    virtual U32 GetLength() const = 0;

    //returns the size of a base AttributeInfo that always preceeds an attribute 
    //(U16 NameIndex & U32 length). The serialized attribute_length, or in this 
    //codebase the returned value from GetLength() does NOT include these 6 bytes
    //hence why this funciton can be useful if you want to calculate the full
    //size of a serialized attribute: 
    //GetHeaderLength() + GetLength() = actual serialized length of attribute
    static U32 GetHeaderLength() { return 6; }
  
    virtual ~AttributeInfo() = default;
  
  protected:
    AttributeInfo(Type type) : m_type{type}{}

  private:
    Type m_type;
};


struct ConstantValueAttribute : public AttributeInfo
{
  ConstantValueAttribute() : AttributeInfo(Type::ConstantValue) {}
  U32 GetLength() const override { return 2;  }

  U16 Index;
};

struct SourceFileAttribute : public AttributeInfo
{
  SourceFileAttribute() : AttributeInfo(Type::SourceFile) {}
  U32 GetLength() const override { return 2;  }

  U16 SourceFileIndex;
};

struct CodeAttribute : public AttributeInfo
{
  CodeAttribute() : AttributeInfo(Type::Code) {}

  U16 MaxStack;
  U16 MaxLocals;
  std::vector<Instruction> Code;

  struct ExceptionHandler
  {
    U16 StartPC;
    U16 EndPC;
    U16 HandlerPC;
    U16 CatchType;
  };
  std::vector<ExceptionHandler> ExceptionTable;

  std::vector< std::unique_ptr<AttributeInfo> > Attributes;

  U32 GetLength() const override 
  { 
    U32 len{0};
    len += sizeof(MaxStack);
    len += sizeof(MaxLocals);

    len += sizeof(U32); //serialized field: "code_length" 

    for(const Instruction& instr : Code)
      len += instr.GetLength();

    len += sizeof(U16); //serialized field: "exception_table_length" 
    len += ExceptionTable.size() * sizeof(U16) * 4;
    len += sizeof(U16); //serialized field: "attributes_count" 

    for(const auto& attr : Attributes)
    {
      len += AttributeInfo::GetHeaderLength();
      len += attr->GetLength();
    }

    return len;
  }

};


//Non standard attribute type, used for parsing unknown or unimplemented attributes as a byte array
struct RawAttribute : public AttributeInfo
{
  RawAttribute() : AttributeInfo(Type::Raw) {}
  U32 GetLength() const override { return static_cast<U32>(Bytes.size());  }

  std::vector<U8> Bytes;
};

} //namespace ClassFile
