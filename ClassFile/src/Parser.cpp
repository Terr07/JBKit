#include "ClassFile/Parser.hpp"

#include <fmt/core.h>

#include "Util/IO.hpp"
#include "Util/Error.hpp"

#include <cxxabi.h>

#include <iostream>
#include <cassert>
#include <map>
#include <tuple>

namespace ClassFile
{

ErrorOr<ClassFile> Parser::ParseClassFile(std::istream& stream)
{
  ClassFile cf;

  TRY(Read<BigEndian>(stream,
                      cf.Magic,
                      cf.MinorVersion,
                      cf.MajorVersion));

  auto errOrCP = Parser::ParseConstantPool(stream);
  VERIFY(errOrCP);

  cf.ConstPool = errOrCP.Release();

  U16 interfacesCount;
  TRY(Read<BigEndian>(stream,
                      cf.AccessFlags,
                      cf.ThisClass,
                      cf.SuperClass,
                      interfacesCount));

  cf.Interfaces.reserve(interfacesCount);
  for (auto i = 0; i < interfacesCount; i++)
  {
    U16 interfaceIndex;
    TRY( Read<BigEndian>(stream, interfaceIndex));
    cf.Interfaces.emplace_back(interfaceIndex);
  }

  U16 fieldsCount;
  TRY(Read<BigEndian>(stream, fieldsCount));

  cf.Fields.reserve(fieldsCount);
  for (auto i = 0; i < fieldsCount; i++)
  {
    auto errOrField = Parser::ParseFieldMethodInfo(stream, cf.ConstPool);
    VERIFY(errOrField);

    cf.Fields.emplace_back(errOrField.Release());
  }


  U16 methodsCount;
  TRY(Read<BigEndian>(stream, methodsCount));

  cf.Methods.reserve(methodsCount);
  for (auto i = 0; i < methodsCount; i++)
  {
    auto errOrMethod = Parser::ParseFieldMethodInfo(stream, cf.ConstPool);
    VERIFY(errOrMethod);

    cf.Methods.emplace_back(errOrMethod.Release());
  }

  U16 attributesCount;
  TRY(Read<BigEndian>(stream, attributesCount));

  cf.Attributes.reserve(attributesCount);
  for (auto i = 0; i < attributesCount; i++)
  {
    auto errOrAttr = Parser::ParseAttribute(stream, cf.ConstPool);
    VERIFY(errOrAttr);

    cf.Attributes.emplace_back(errOrAttr.Release());
  }

  return cf;
}

ErrorOr<ConstantPool> Parser::ParseConstantPool(std::istream& stream)
{
  ConstantPool cp;

  U16 count;
  TRY(Read<BigEndian>(stream, count));

  cp.Reserve(count);

  //count = number of constants + 1
  for(U16 i = 0; i < count-1; i++)
  {
    auto errOrCPInfo = Parser::ParseConstant(stream);
    VERIFY(errOrCPInfo);

    auto cpInfo = errOrCPInfo.Release();

    CPInfo::Type type = cpInfo->GetType();

    cp.Add( std::move(cpInfo) ); 

    //Long & Double constants require the next index into the constant pool
    //after them be invalid.
    if(type == CPInfo::Type::Long || type == CPInfo::Type::Double)
    {
      cp.Add(nullptr);
      i++;
    }

  }

  return cp;
}

static ErrorOr<void> readConst(std::istream& stream, ClassInfo& info)
{
  TRY(Read<BigEndian>(stream, info.NameIndex));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, FieldrefInfo& info)
{
  TRY(Read<BigEndian>(stream, info.ClassIndex, info.NameAndTypeIndex));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, MethodrefInfo& info)
{
  TRY(Read<BigEndian>(stream, info.ClassIndex, info.NameAndTypeIndex));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, InterfaceMethodrefInfo& info)
{
  TRY(Read<BigEndian>(stream, info.ClassIndex, info.NameAndTypeIndex));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, StringInfo& info)
{
  TRY(Read<BigEndian>(stream, info.StringIndex));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, IntegerInfo& info)
{
  TRY(Read<BigEndian>(stream, info.Bytes));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, FloatInfo& info)
{
  TRY(Read<BigEndian>(stream, info.Bytes));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, LongInfo& info)
{
  TRY(Read<BigEndian>(stream, info.HighBytes, info.LowBytes));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, DoubleInfo& info)
{
  TRY(Read<BigEndian>(stream, info.HighBytes, info.LowBytes));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, NameAndTypeInfo& info)
{
  TRY(Read<BigEndian>(stream, info.NameIndex, info.DescriptorIndex));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, UTF8Info& info)
{
  U16 len;
  TRY(Read<BigEndian>(stream, len));

  //TODO: add IO util func for this
  info.String = std::string(len, '\0');
  stream.read(&info.String[0], len);

  if (stream.bad())
    return Error{fmt::format("Parser::readConst(UTF8Info): failed to read string")};

  return {};
}

static ErrorOr<void> readConst(std::istream& stream, MethodHandleInfo& info)
{
  TRY(Read<BigEndian>(stream, info.ReferenceKind, info.ReferenceIndex));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, MethodTypeInfo& info)
{
  TRY(Read<BigEndian>(stream, info.DescriptorIndex));
  return {};
}

static ErrorOr<void> readConst(std::istream& stream, InvokeDynamicInfo& info)
{
  TRY(Read<BigEndian>(stream, info.BootstrapMethodAttrIndex, info.NameAndTypeIndex));
  return {};
}

template <typename CPInfoT>
static ErrorOr< std::unique_ptr<CPInfo> > parseConstT(std::istream& stream)
{
  CPInfoT* pInfo = new CPInfoT{};
  auto errOrConst = readConst(stream, *pInfo);
  VERIFY(errOrConst);

  return std::unique_ptr<CPInfo>(pInfo);
}

ErrorOr< std::unique_ptr<CPInfo> > Parser::ParseConstant(std::istream& stream)
{
  CPInfo::Type type = static_cast<CPInfo::Type>(stream.get());

  switch(type)
  {
    case CPInfo::Type::Class:       return parseConstT<ClassInfo>(stream);
    case CPInfo::Type::Fieldref:    return parseConstT<FieldrefInfo>(stream);
    case CPInfo::Type::Methodref:   return parseConstT<MethodrefInfo>(stream);
    case CPInfo::Type::InterfaceMethodref: return parseConstT<InterfaceMethodrefInfo>(stream);
    case CPInfo::Type::String:      return parseConstT<StringInfo>(stream);
    case CPInfo::Type::Integer:     return parseConstT<IntegerInfo>(stream);
    case CPInfo::Type::Float:       return parseConstT<FloatInfo>(stream);
    case CPInfo::Type::Long:        return parseConstT<LongInfo>(stream);
    case CPInfo::Type::Double:      return parseConstT<DoubleInfo>(stream);
    case CPInfo::Type::NameAndType: return parseConstT<NameAndTypeInfo>(stream);
    case CPInfo::Type::UTF8:        return parseConstT<UTF8Info>(stream);
    case CPInfo::Type::MethodHandle:  return parseConstT<MethodHandleInfo>(stream);
    case CPInfo::Type::MethodType:    return parseConstT<MethodTypeInfo>(stream);
    case CPInfo::Type::InvokeDynamic: return parseConstT<InvokeDynamicInfo>(stream);
  }

  return Error{fmt::format("Parser::ParseConstant: encountered unknown tag "
      "value \"{}\"", static_cast<U8>(type))};
}

ErrorOr<FieldMethodInfo> Parser::ParseFieldMethodInfo(
    std::istream& stream, const ConstantPool& constPool)
{
  FieldMethodInfo info;

  U16 attributesCount;
  TRY(Read<BigEndian>(stream, info.AccessFlags,
                              info.NameIndex,
                              info.DescriptorIndex,
                              attributesCount));

  info.Attributes.reserve(attributesCount);
  for (auto i = 0; i < attributesCount; i++)
  {
    auto errOrAttr = Parser::ParseAttribute(stream, constPool);
    VERIFY(errOrAttr);

    info.Attributes.emplace_back(errOrAttr.Release());
  }

  return info;
}


static ErrorOr<void> readAttribute(std::istream& stream, 
    const ConstantPool& constPool, ConstantValueAttribute& attr)
{
  TRY(Read<BigEndian>(stream, attr.Index));
  return {};
}

static ErrorOr<void> readAttribute(std::istream& stream, 
    const ConstantPool& constPool, SourceFileAttribute& attr)
{
  TRY(Read<BigEndian>(stream, attr.SourceFileIndex));
  return {};
}

static ErrorOr<void> readAttribute(std::istream& stream, 
    const ConstantPool& constPool, CodeAttribute& attr)
{
  U32 codeLen;
  TRY(Read<BigEndian>(stream, 
                      attr.MaxStack,
                      attr.MaxLocals,
                      codeLen));


  U32 parsedCodeLen{0};
  while(parsedCodeLen < codeLen)
  {
    auto streampos_before = stream.tellg();

    //TODO: handle padding for instructions that require alignment
    auto errOrInstr = Parser::ParseInstruction(stream);
    VERIFY(errOrInstr);

    attr.Code.emplace_back(errOrInstr.Release());

    auto parsed = stream.tellg() - streampos_before;
    assert(parsed > 0);

    parsedCodeLen += parsed;
  }

  if(parsedCodeLen != codeLen)
  {
    return Error{fmt::format("Parser::readAttribute(CodeAttr): "
        "CodeLen field indicates codelen of: {}, "
        "but total code bytes parsed was: {}.", codeLen, parsedCodeLen)};
  }

  //TODO: create a read util function for these sorts of 
  //      "read length, then read array of that length" scenarios
  U16 exceptionTableLen;
  TRY(Read<BigEndian>(stream, exceptionTableLen));

  attr.ExceptionTable.reserve(exceptionTableLen);
  for(auto i = 0; i < exceptionTableLen; i++)
  {
    CodeAttribute::ExceptionHandler handler;
    TRY(Read<BigEndian>(stream, handler.StartPC, 
                                handler.EndPC, 
                                handler.HandlerPC, 
                                handler.CatchType));

    attr.ExceptionTable.emplace_back(handler);
  }

  U16 attributesCount;
  TRY(Read<BigEndian>(stream, attributesCount));

  attr.Attributes.reserve(attributesCount);
  for(auto i = 0; i < attributesCount; i++)
  {
    auto errOrAttr = Parser::ParseAttribute(stream, constPool);
    VERIFY(errOrAttr);

    attr.Attributes.emplace_back( errOrAttr.Release() );
  }

  return {};
}

template <typename AttributeT>
static ErrorOr< std::unique_ptr<AttributeInfo> > parseAttributeT(
    std::istream& stream, const ConstantPool& constPool, U16 nameIndex, U32 len)
{
  AttributeT* attr = new AttributeT();
  attr->NameIndex = nameIndex;

  auto err = readAttribute(stream, constPool, *attr);
  VERIFY(err);

  U32 attrLen = attr->GetLength();
  if(attrLen != len)
  {
    return Error{ fmt::format("Parser::parseAttributeT<{}>(): "
        "AttrLen field indicates len of: {}, "
        "but total len of parsed bytes was: {}", typeid(AttributeT).name(), len, attrLen)};
  }

  return std::unique_ptr<AttributeInfo>(attr);
}

ErrorOr< std::unique_ptr<AttributeInfo> > Parser::ParseAttribute(
    std::istream& stream, const ConstantPool& constPool)
{
  U16 nameIndex;
  U32 len;
  TRY(Read<BigEndian>(stream, nameIndex, len));

  auto errOrName = constPool.LookupString(nameIndex);
  VERIFY(errOrName);

  auto errOrType = AttributeInfo::GetType(errOrName.Get());

  AttributeInfo::Type type;
  if (errOrType.IsError())
  {
    std::cerr << "[WARNING]: " << errOrType.GetError().What;
    std::cerr << " (interpreting as raw attribute instead)\n";
    type = AttributeInfo::Type::Raw;
  }
  else
    type = errOrType.Get();

  switch (type)
  {
    case AttributeInfo::Type::ConstantValue: 
      return parseAttributeT<ConstantValueAttribute>(stream, constPool, nameIndex, len);
    case AttributeInfo::Type::SourceFile: 
      return parseAttributeT<SourceFileAttribute>(stream, constPool, nameIndex, len);
    case AttributeInfo::Type::Code: 
      return parseAttributeT<CodeAttribute>(stream, constPool, nameIndex, len);
  }

  //TODO: remove raws once everything is implemented, or WARN or something idk
  RawAttribute* attr = new RawAttribute{};
  attr->NameIndex = nameIndex;

  //TODO: add ReadArray<> to util
  attr->Bytes.reserve(len);
  for (unsigned i = 0; i < len; i++)
  {
    U8 byte;
    TRY(Read(stream, byte));

    attr->Bytes.emplace_back(byte);
  }

  return std::unique_ptr<AttributeInfo>(attr);
}

template <typename T>
static ErrorOr<void> readOperand(std::istream& stream, Instruction& instr, size_t i)
{
  auto errOrRef = instr.Operand<T>(i);
  VERIFY(errOrRef, 
      fmt::format("failed to access operand {} of \"{}\"", i, instr.GetMnemonic()));

  TRY(Read<BigEndian>(stream, errOrRef.Get().get()));

  return NoError{};
}

ErrorOr<Instruction> Parser::ParseInstruction(std::istream& stream)
{
  Instruction::Opcode op;
  TRY(Read<BigEndian>(stream, (U8&)op));

  Instruction instr = Instruction::MakeInstruction(op).Get();

  if(instr.IsComplex())
  {
    return Error{fmt::format("Parser::ParserInstruction(): "
        "encountered complex instruction: \"{}\", "
        "which parsing is not implemented for yet.", instr.GetMnemonic())};
  }

  for(size_t i{0}; i < instr.GetNOperands(); i++)
  {
    switch( instr.GetOperandType(i) )
    {
      using Type = Instruction::OperandType;
      case Type::TypeS32: TRY(readOperand<S32>(stream, instr, i)) break;
      case Type::TypeS16: TRY(readOperand<S16>(stream, instr, i)) break;
      case Type::TypeS8 : TRY(readOperand<S8 >(stream, instr, i)) break;
      case Type::TypeU16: TRY(readOperand<U16>(stream, instr, i)) break;
      case Type::TypeU8 : TRY(readOperand<U8 >(stream, instr, i)) break;
    }
  }

  return instr;
}

} //namespace ClassFile
