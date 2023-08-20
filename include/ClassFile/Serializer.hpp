#pragma once

#include "ClassFile.hpp"
#include "Error.hpp"

namespace ClassFile
{

class Serializer
{
  public:
    static ErrorOr<void> SerializeClassFile(std::ostream&, const ClassFile&);
    static ErrorOr<void> SerializeConstantPool(std::ostream&, const ConstantPool&);
    static ErrorOr<void> SerializeConstant(std::ostream&, const CPInfo&);

    static ErrorOr<void> SerializeFieldMethod(std::ostream&, const FieldMethodInfo&);
    static ErrorOr<void> SerializeAttribute(std::ostream&, const AttributeInfo&);

    static ErrorOr<void> SerializeInstruction(std::ostream&, const Instruction&);
};

} //namespace ClassFile

