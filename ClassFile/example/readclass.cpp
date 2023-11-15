/*
 * Parses & prints informations about classfile.
 */

#include <iostream>
#include <fstream>
#include <cassert>
#include <chrono>
#include <string_view>

#include <ClassFile/Error.hpp>

#include <ClassFile/ClassFile.hpp>
#include <ClassFile/Parser.hpp>
#include <ClassFile/Serializer.hpp>

static bool PrintDetails{false};

void PrintConstInfo(size_t i, const ClassFile::ConstantPool& cp)
{
  std::cout << "ConstPool[" << i << "] = " << cp[i]->GetName();
  using Type = ClassFile::CPInfo::Type;

  auto errOrString = cp.LookupString(i);
  auto errOrDesc   = cp.LookupDescriptor(i);

  if(!errOrString.IsError())
    std::cout << " " << errOrString.Get();

  if(!errOrDesc.IsError())
    std::cout << " " << errOrDesc.Get();

  std::cout << '\n';
}

std::vector<std::string_view> GetMethodFlags(const ClassFile::FieldMethodInfo& method)
{
  std::vector<std::string_view> attributes;

  if(method.AccessFlags & 0x0001) attributes.push_back("PUBLIC");
  if(method.AccessFlags & 0x0002) attributes.push_back("PRIVATE");
  if(method.AccessFlags & 0x0004) attributes.push_back("PROTECTED");
  if(method.AccessFlags & 0x0008) attributes.push_back("STATIC");
  if(method.AccessFlags & 0x0010) attributes.push_back("FINAL");
  if(method.AccessFlags & 0x0020) attributes.push_back("SYNCHRONIZED");
  if(method.AccessFlags & 0x0040) attributes.push_back("BRIDGE");
  if(method.AccessFlags & 0x0080) attributes.push_back("VARARGS");
  if(method.AccessFlags & 0x0100) attributes.push_back("NATIVE");
  if(method.AccessFlags & 0x0400) attributes.push_back("ABSTRACT");
  if(method.AccessFlags & 0x0800) attributes.push_back("STRICT");
  if(method.AccessFlags & 0x1000) attributes.push_back("SYNTHETIC");

  return attributes;
}

std::vector<std::string_view> GetClassFlags(ClassFile::U16 flags)
{
  std::vector<std::string_view> attributes;

  if(flags & 0x0001) attributes.push_back("PUBLIC");
  if(flags & 0x0010) attributes.push_back("FINAL");
  if(flags & 0x0020) attributes.push_back("SUPER");
  if(flags & 0x0200) attributes.push_back("INTERFACE");
  if(flags & 0x0400) attributes.push_back("ABSTRACT");
  if(flags & 0x1000) attributes.push_back("SYNTHETIC");
  if(flags & 0x2000) attributes.push_back("ANNOTATION");
  if(flags & 0x4000) attributes.push_back("ENUM");

  return attributes;
}

std::string_view GetOperandTypeName(ClassFile::Instruction::OperandType type)
{
  switch(type)
  {
    using Type = ClassFile::Instruction::OperandType;
    case Type::TypeU8:  return "U8";
    case Type::TypeU16: return "U16";
    case Type::TypeS8:  return "S8";
    case Type::TypeS16: return "S16";
    case Type::TypeS32: return "S32";
  }

  return "UNKNOWN_TYPE";
}

void PrintInstrInfo(const ClassFile::Instruction& instr)
{
  std::cout << instr.GetMnemonic() << " (0x";
  std::cout << std::hex << (int)instr.Op << std::dec << ")";

  if(instr.GetNOperands() > 0)
    std::cout << ": Opreands[";
  else
    std::cout << '\n';

  for(size_t i = 0; i < instr.GetNOperands(); i++)
  {
    std::cout << GetOperandTypeName(instr.GetOperandType(i));
    std::cout << "{";
    std::cout <<  instr.GetOperand(i).Get();
    std::cout << "}";

    if(i + 1 != instr.GetNOperands())
      std::cout << ", ";
    else
      std::cout << "]\n";
  }
}

void PrintConstPool(const ClassFile::ConstantPool& cp)
{

  std::cout << "Const pool entries: " 
    << cp.GetSize()<< '\n';

  if(!PrintDetails)
    return;


  for(auto i{1}; i < cp.GetCount(); i++)
  {
    if(cp[i] == nullptr)
      continue;

    std::cout << "  ";

    PrintConstInfo(i, cp);

    if(i+1 == cp.GetCount())
      std::cout << "\n";
  }

}

void PrintFlags(const ClassFile::ClassFile& cf)
{
  std::cout << "Access flags: 0x" << std::uppercase << std::hex
    << cf.AccessFlags << std::nouppercase << std::dec;

  auto flagStrings = GetClassFlags(cf.AccessFlags);

  if(flagStrings.size() == 0)
  {
    std::cout << '\n';
    return;
  }

  std::cout << " (";

  for(auto i = 0u; i < flagStrings.size(); i++)
  {
    std::cout << flagStrings[i];

    if(i+1 != flagStrings.size())
      std::cout << ", ";
  }

  std::cout << ")\n";
}

void PrintMethods(const ClassFile::ClassFile& cf)
{
  std::cout << "Methods: " << cf.Methods.size();

  for(const auto& method : cf.Methods)
  {
    std::cout << "\n  ";
    std::cout << cf.ConstPool.LookupString(method.DescriptorIndex).Get();
    std::cout << " - ";
    std::cout << cf.ConstPool.LookupString(method.NameIndex).Get();
    std::cout << " [";

    auto flagStrings = GetMethodFlags(method);

    for(auto i = 0u; i < flagStrings.size(); i++)
    {
      std::cout << flagStrings[i];

      if(i+1 != flagStrings.size())
        std::cout << ", ";
    }

    std::cout << "]";

    if(!PrintDetails)
      continue;

    for(size_t i = 0; i < method.Attributes.size(); i++)
    {
      if(method.Attributes[i] == nullptr)
        continue;

      if(method.Attributes[i]->GetType() != ClassFile::AttributeInfo::Type::Code)
        continue;

      const auto& codeAttr = static_cast<const ClassFile::CodeAttribute&>(*method.Attributes[i]);

      for(size_t j = 0; j < codeAttr.Code.size(); j++)
      {
        if(j == 0)
          std::cout << ":\n";

        std::cout << "    ";
        PrintInstrInfo(codeAttr.Code[j]);
      }

    }

    std::cout << "\n";
  }

}

void PrintFields(const ClassFile::ClassFile& cf)
{
  std::cout << "\nFields: " << cf.Fields.size();

  for(const auto& field : cf.Fields)
  {
    std::cout << "\n  ";
    std::cout << cf.ConstPool.LookupString(field.NameIndex).Get();
    std::cout << "(";

    //                 same struct, same flags
    auto flagStrings = GetMethodFlags(field);

    for(auto i = 0u; i < flagStrings.size(); i++)
    {
      std::cout << flagStrings[i];

      if(i+1 != flagStrings.size())
        std::cout << ", ";
    }

    std::cout << "): ";
    std::cout << cf.ConstPool.LookupString(field.DescriptorIndex).Get();
  }

  std::cout << "\n";
}

void PrintClassInfo(const ClassFile::ClassFile& cf)
{
  std::cout << "Magic number: 0x" << std::uppercase << std::hex
    << cf.Magic << '\n' << std::nouppercase << std::dec;

  std::cout << "Classfile version: " 
    << cf.MajorVersion << '.' << cf.MinorVersion << '\n';

  PrintConstPool(cf.ConstPool);
  PrintFlags(cf);

  std::cout << "ThisClass: ";
  std::cout << cf.ConstPool.LookupString(cf.ThisClass).Get() << '\n';

  std::cout << "SuperClass: ";
  std::cout << cf.ConstPool.LookupString(cf.SuperClass).Get() << '\n';

  std::cout << "Interfaces count: " << cf.Interfaces.size();

  if(PrintDetails)
  {
    for(size_t i = 0; i < cf.Interfaces.size(); i++)
    {
      std::cout << "\n  Interfaces[" << i << "] = " <<cf.Interfaces[i];

      if(i+1 == cf.Interfaces.size())
        std::cout << '\n';
    }
  }

  std::cout << '\n';

  PrintMethods(cf);
  PrintFields(cf);

}

int main(int argc, char** argv)
{
  if(argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <classfile> (--details)\n";
    return -1;
  }

  for(auto i = 2; i < argc; i++)
  {
    using namespace std::literals;

    if("--details"sv == argv[i]) 
    {
      PrintDetails = true;
      continue;
    }

    std::cout << "Unknown flag / argument: \"" << argv[i] << "\"\n";
    return -2;
  }

  std::ifstream infile{argv[1], std::ios::ate};

  if(!infile.good())
  {
    std::cout << "Unable to open file \"" << argv[1] << "\"\n";
    return -3;
  }

  auto fileSize = infile.tellg();
  infile.seekg(0);


  auto before = std::chrono::high_resolution_clock::now();
  auto errOrClass = ClassFile::Parser::ParseClassFile(infile);
  auto after = std::chrono::high_resolution_clock::now();

  if(errOrClass.IsError())
  {
    std::cout << "ERROR: " << errOrClass.GetError().What << '\n';
    return -1;
  }

  std::cout << "Parsed " << infile.tellg() << '/' << fileSize << " bytes ";
  std::cout << "in ~" << std::chrono::duration_cast<std::chrono::nanoseconds>(after-before).count() / 1000000.0f << " milliseconds\n\n";

  ClassFile::ClassFile cf = errOrClass.Release();
  PrintClassInfo(cf);

  return 0;
}
