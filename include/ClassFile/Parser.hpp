#pragma once

#include "ClassFile.hpp"
#include "Error.hpp"

namespace ClassFile
{

class Parser
{
  public:
    static ErrorOr<ClassFile> ParseClassFile(std::istream&);
    static ErrorOr<ConstantPool> ParseConstantPool(std::istream&);
    static ErrorOr< std::unique_ptr<CPInfo> > ParseConstant(std::istream&);

    static ErrorOr<FieldMethodInfo> ParseFieldMethodInfo(std::istream&, const ConstantPool&);
    static ErrorOr< std::unique_ptr<AttributeInfo> > ParseAttribute(std::istream&, const ConstantPool&);

    static ErrorOr<Instruction> ParseInstruction(std::istream&);
};


} //namespace ClassFile
