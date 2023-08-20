#pragma once

#include "Defs.hpp"
#include "ConstantPool.hpp"
#include "Attribute.hpp"

namespace ClassFile
{

struct FieldMethodInfo
{
  U16 AccessFlags;
  U16 NameIndex;
  U16 DescriptorIndex;
  std::vector< std::unique_ptr<AttributeInfo> > Attributes;
};

struct ClassFile 
{
  U32 Magic;
  U16 MinorVersion;
  U16 MajorVersion;
  ConstantPool ConstPool;
  U16 AccessFlags;
  U16 ThisClass;
  U16 SuperClass;
  std::vector<U16> Interfaces;
  std::vector<FieldMethodInfo> Fields;
  std::vector<FieldMethodInfo> Methods;
  std::vector< std::unique_ptr<AttributeInfo> > Attributes;
};

} //namespace ClassFile
