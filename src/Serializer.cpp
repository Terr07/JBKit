#include "ClassFile/Serializer.hpp"

#include "Util/IO.hpp"
#include "Util/Error.hpp"

namespace ClassFile
{

ErrorOr<void> Serializer::SerializeClassFile(std::ostream& stream, const ClassFile& cf)
{
  TRY(Write<BigEndian>(stream, cf.Magic,
                               cf.MinorVersion,
                               cf.MajorVersion));

  TRY( Serializer::SerializeConstantPool(stream, cf.ConstPool) );

  TRY(Write<BigEndian>(stream, cf.AccessFlags,
                               cf.ThisClass,
                               cf.SuperClass,
                               static_cast<U16>(cf.Interfaces.size())));

  for(U16 interface : cf.Interfaces)
    TRY(Write<BigEndian>(stream, interface));

  TRY( Write<BigEndian>(stream, static_cast<U16>(cf.Fields.size())) );

  for(const auto& field : cf.Fields)
    TRY( Serializer::SerializeFieldMethod(stream, field) );

  TRY( Write<BigEndian>(stream, static_cast<U16>(cf.Methods.size())) );

  for(const auto& method: cf.Methods)
    TRY( Serializer::SerializeFieldMethod(stream, method) );

  TRY( Write<BigEndian>(stream, static_cast<U16>(cf.Attributes.size())) );

  for(const auto& pAttr: cf.Attributes)
    TRY( Serializer::SerializeAttribute(stream, *pAttr) );

  return {};
}

ErrorOr<void> Serializer::SerializeConstantPool(std::ostream& stream, const ConstantPool& cp)
{

  TRY(Write<BigEndian>(stream, cp.GetCount()));

  for(auto i = 0; i < cp.GetCount(); i++)
  {
    const CPInfo* ptr = cp[i];

    //TODO: maybe the "filler" consts shouldn't be nullptr. Instead add a 
    //non-standard FillInfo struct
    if(ptr == nullptr)
      continue;

    TRY(Serializer::SerializeConstant(stream, *ptr));
  }

  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const InvokeDynamicInfo& info)
{
  TRY(Write<BigEndian>(stream, info.BootstrapMethodAttrIndex,
                               info.NameAndTypeIndex));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const MethodTypeInfo& info)
{
  TRY(Write<BigEndian>(stream, info.DescriptorIndex));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const MethodHandleInfo& info)
{
  TRY(Write<BigEndian>(stream, info.ReferenceKind,
                               info.ReferenceIndex));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const UTF8Info& info)
{
  TRY(Write<BigEndian>(stream, static_cast<U16>( info.String.length() )));
  stream << info.String;
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const NameAndTypeInfo& info)
{
  TRY(Write<BigEndian>(stream, info.NameIndex,
                               info.DescriptorIndex));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const DoubleInfo& info)
{
  TRY(Write<BigEndian>(stream, info.HighBytes,
                               info.LowBytes));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const LongInfo& info)
{
  TRY(Write<BigEndian>(stream, info.HighBytes,
                               info.LowBytes));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const FloatInfo& info)
{
  TRY(Write<BigEndian>(stream, info.Bytes));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const IntegerInfo& info)
{
  TRY(Write<BigEndian>(stream, info.Bytes));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const StringInfo& info)
{
  TRY(Write<BigEndian>(stream, info.StringIndex));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const InterfaceMethodrefInfo& info)
{
  TRY(Write<BigEndian>(stream, info.ClassIndex, 
                               info.NameAndTypeIndex));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const MethodrefInfo& info)
{
  TRY(Write<BigEndian>(stream, info.ClassIndex, 
                               info.NameAndTypeIndex));
  return {};
}

static ErrorOr<void> writeConst(std::ostream& stream, const FieldrefInfo& info)
{
  TRY(Write<BigEndian>(stream, info.ClassIndex, 
                               info.NameAndTypeIndex));
  return {};
}


static ErrorOr<void> writeConst(std::ostream& stream, const ClassInfo& info)
{
  TRY(Write<BigEndian>(stream, info.NameIndex));
  return {};
}

template <typename T>
static ErrorOr<void> writeConstT(std::ostream& stream, const CPInfo& info)
{
  return writeConst(stream, static_cast<const T&>(info));
}

ErrorOr<void> Serializer::SerializeConstant(std::ostream& stream, const CPInfo& info)
{
  U8 tag = static_cast<U8>(info.GetType());
  TRY(Write<BigEndian>(stream, tag));

  switch(info.GetType())
  {
    case CPInfo::Type::Class:         return writeConstT<ClassInfo>(stream, info);
    case CPInfo::Type::Fieldref:      return writeConstT<FieldrefInfo>(stream, info);
    case CPInfo::Type::Methodref:     return writeConstT<MethodrefInfo>(stream, info);
    case CPInfo::Type::InterfaceMethodref: return writeConstT<InterfaceMethodrefInfo>(stream, info);
    case CPInfo::Type::String:        return writeConstT<StringInfo>(stream, info);
    case CPInfo::Type::Integer:       return writeConstT<IntegerInfo>(stream, info);
    case CPInfo::Type::Float:         return writeConstT<FloatInfo>(stream, info);
    case CPInfo::Type::Long:          return writeConstT<LongInfo>(stream, info);
    case CPInfo::Type::Double:        return writeConstT<DoubleInfo>(stream, info);
    case CPInfo::Type::NameAndType:   return writeConstT<NameAndTypeInfo>(stream, info);
    case CPInfo::Type::UTF8:          return writeConstT<UTF8Info>(stream, info);
    case CPInfo::Type::MethodHandle:  return writeConstT<MethodHandleInfo>(stream, info);
    case CPInfo::Type::MethodType:    return writeConstT<MethodTypeInfo>(stream, info);
    case CPInfo::Type::InvokeDynamic: return writeConstT<InvokeDynamicInfo>(stream, info);
  }

  return Error::FromFormatStr("WriteConstant: write func not implemented for const with tag %hhu", tag);
}

ErrorOr<void> Serializer::SerializeFieldMethod(std::ostream& stream, const FieldMethodInfo& info)
{
  TRY( Write<BigEndian>(stream, info.AccessFlags,
                                info.NameIndex,
                                info.DescriptorIndex,
                                static_cast<U16>(info.Attributes.size())) );

  for(const auto& pAttr : info.Attributes)
    TRY( Serializer::SerializeAttribute(stream, *pAttr) );

  return {};
}

static ErrorOr<void> writeAttr(std::ostream& stream, const RawAttribute& attr)
{
  //TODO: add array writing IO util funcs
  stream.write(reinterpret_cast<const char*>(attr.Bytes.data()), attr.GetLength());

  if(stream.bad())
    return Error::FromFormatStr("failed to write attribute, stream badbit set: %s in %s", __func__, __FILE__);

  return {};
}

static ErrorOr<void> writeAttr(std::ostream& stream, const SourceFileAttribute& attr)
{
  TRY( Write<BigEndian>(stream, attr.SourceFileIndex) );
  return {};
}

static ErrorOr<void> writeAttr(std::ostream& stream, const CodeAttribute& attr)
{
  /*
  TRY( Write<BigEndian>(stream, attr.MaxStack,
                                attr.MaxLocals) );

  U32 codeLen{0};

  //TODO: handle padding for instructions that require alignment
  for(const auto&  pInstr: attr.Code)
    codeLen += pInstr->GetLength();

  TRY( Write<BigEndian>(stream, codeLen) );

  //TODO: handle padding for instructions that require alignment
  for(const auto& pInstr : attr.Code)
    TRY( Serializer::SerializeInstruction(stream, *pInstr.get()) );

  if(stream.bad())
    return Error::FromFormatStr("failed to write code attr, stream badbit set: %s in %s", __func__, __FILE__);

  TRY( Write<BigEndian>(stream, static_cast<U16>(attr.ExceptionTable.size())) );

  for(auto handler : attr.ExceptionTable)
  {
    TRY( Write<BigEndian>(stream, handler.StartPC, 
                                  handler.EndPC,
                                  handler.HandlerPC,
                                  handler.CatchType) );
  }

  TRY( Write<BigEndian>(stream, static_cast<U16>(attr.Attributes.size())) );

  for(const auto& pAttr : attr.Attributes)
    TRY ( Serializer::SerializeAttribute(stream, *pAttr) );

  return {};
  */

  return Error::FromLiteralStr("writeAttr for CodeAttribute not imlemented.");
}

static ErrorOr<void> writeAttr(std::ostream& stream, const ConstantValueAttribute& attr)
{
  TRY( Write<BigEndian>(stream, attr.Index) );
  return {};
}

template <typename T>
static ErrorOr<void> writeAttrT(std::ostream& stream, const AttributeInfo& info)
{
  return writeAttr(stream, static_cast<const T&>(info));
}

ErrorOr<void> Serializer::SerializeAttribute(std::ostream& stream, const AttributeInfo& info)
{
  TRY( Write<BigEndian>(stream, info.NameIndex, info.GetLength()) );

  switch(info.GetType())
  {
    case AttributeInfo::Type::ConstantValue: return writeAttrT<ConstantValueAttribute>(stream, info);
    case AttributeInfo::Type::Code:          return writeAttrT<CodeAttribute>(stream, info);
    case AttributeInfo::Type::SourceFile:    return writeAttrT<SourceFileAttribute>(stream, info);

    case AttributeInfo::Type::Raw:           return writeAttrT<RawAttribute>(stream, info);
  }

  //TODO: stop using old c printf for formattting, as it isn't compatible with
  //things like non-null terminated std::string_view
  return Error::FromFormatStr("WriteAttribute: write func not implemented for attribute with name \"%.*s\"", info.GetName().length(), info.GetName().data());
}

template <typename T>
static ErrorOr<void> writeOperand(std::ostream& stream, const Instruction& instr, size_t i)
{
  auto errOrOperandValue = instr.GetOperand(i);
  VERIFY(errOrOperandValue);

  T value = static_cast<T>(errOrOperandValue.Get());

  TRY(Write<BigEndian>(stream, value));
  return {};
}

static ErrorOr<void> writeOperands(std::ostream& stream, const Instruction& instr)
{
  //TODO: handle padding etc etc

  for(size_t i = 0; i < instr.GetNOperands(); i++)
  {
    auto errOrType = instr.GetOperandType(i);
    VERIFY(errOrType);

    switch(errOrType.Get())
    {
      case Instruction::OperandType::U8:  TRY(writeOperand<U8>( stream, instr, i));break;
      case Instruction::OperandType::U16: TRY(writeOperand<U16>(stream, instr, i));break;
      case Instruction::OperandType::S8:  TRY(writeOperand<S8>( stream, instr, i));break;
      case Instruction::OperandType::S16: TRY(writeOperand<S16>(stream, instr, i));break;
      case Instruction::OperandType::S32: TRY(writeOperand<S32>(stream, instr, i));break;
      default: return Error::FromLiteralStr("SerializeInstruction encountered unknown operand type.");
    }

  }

  return {};
}

ErrorOr<void> Serializer::SerializeInstruction(std::ostream& stream, const Instruction& instr)
{
  TRY(Write<BigEndian>(stream, instr.GetOpCode()));
  TRY(writeOperands(stream, instr));
  return {};

  /*

  for(size_t i = 0; i < instr.GetNOperands(); ++i)
  {
    auto errOrOpr = instr.GetOperand(i);
    VERIFY(errOrOpr);

    auto operandSerializer = [&](auto& operand)
    {
      return Write<BigEndian>(stream, operand);
    };

    TRY(std::visit(operandSerializer, errOrOpr.Get()));
  }

  return {};
  */
}

} //namespace ClassFile
