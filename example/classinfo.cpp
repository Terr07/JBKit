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

void PrintConstInfo(size_t i, const ClassFile::CPInfo& info, const ClassFile::ConstantPool& cp)
{
  std::cout << "ConstPool[" << i << "] = " << info.GetName();
  using Type = ClassFile::CPInfo::Type;

  if(info.GetType() == Type::UTF8)
  {
    std::cout << ": \"";

    const auto& utfInfo = static_cast<const ClassFile::UTF8Info&>(info);

    std::cout << utfInfo.String;
    std::cout << "\"\n";
    return;
  }

  if(info.GetType() == Type::Class)
  {
    std::cout << ": \"";

    const auto& classInfo = static_cast<const ClassFile::ClassInfo&>(info);
    const auto* pConst = cp[classInfo.NameIndex]; assert(pConst != nullptr);

    const auto& UTF8Info = static_cast<const ClassFile::UTF8Info&>(*pConst);

    std::cout << UTF8Info.String;
    std::cout << "\"\n";
    return;
  }

  if(info.GetType() == Type::String)
  {
    std::cout << ": \"";

    const auto& stringInfo = static_cast<const ClassFile::StringInfo&>(info);
    const auto* pConst = cp[stringInfo.StringIndex]; assert(pConst != nullptr);

    const auto& UTF8Info = static_cast<const ClassFile::UTF8Info&>(*pConst);

    std::cout << UTF8Info.String;
    std::cout << "\"\n";
    return;
  }

  if(info.GetType() == Type::NameAndType)
  {
    std::cout << ": \"";

    const auto& nameAndTypeInfo = static_cast<const ClassFile::NameAndTypeInfo&>(info);
    std::cout << cp.GetConstNameOrTypeStr(nameAndTypeInfo.NameIndex);
    std::cout << "\" ";
    std::cout << cp.GetConstNameOrTypeStr(nameAndTypeInfo.DescriptorIndex);
    std::cout << '\n';
    return;
  }

  if(info.GetType() == Type::Fieldref)
  {
    std::cout << ": ";

    const auto& fieldrefInfo = static_cast<const ClassFile::FieldrefInfo&>(info);
    std::cout << fieldrefInfo.ClassIndex;
    std::cout << " (" << cp.GetConstNameOrTypeStr(fieldrefInfo.ClassIndex) << ")\n";
    return;
  }

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
    case Type::U8:  return "U8";
    case Type::U16: return "U16";
    case Type::S8:  return "S8";
    case Type::S16: return "S16";
    case Type::S32: return "S32";
  }

  return "UNKNOWN_TYPE";
}

void PrintInstrInfo(const ClassFile::Instruction& instr)
{
  std::cout << instr.GetMnemonic() << " (0x";
  std::cout << std::hex << (int)instr.GetOpCode() << std::dec << ")";

  if(instr.GetNOperands() > 0)
    std::cout << ": Opreands[";
  else
    std::cout << '\n';

  for(size_t i = 0; i < instr.GetNOperands(); i++)
  {
    std::cout << GetOperandTypeName(instr.GetOperandType(i).Get());
    std::cout << "{";
    std::cout <<  instr.GetOperand(i).Get();
    std::cout << "}";

    if(i + 1 != instr.GetNOperands())
      std::cout << ", ";
    else
      std::cout << "]\n";
  }
}

void PrintClassInfo(const ClassFile::ClassFile& cf)
{
  std::cout << "Magic number: 0x" << std::uppercase << std::hex
            << cf.Magic << '\n' << std::nouppercase << std::dec;

  std::cout << "Classfile version: " 
            << cf.MajorVersion << '.' << cf.MinorVersion << '\n';

  std::cout << "Const pool size: " 
            << cf.ConstPool.GetSize()<< '\n';

  if(PrintDetails)
  {
    for(auto i{0}; i < cf.ConstPool.GetSize(); i++)
    {
      if(cf.ConstPool[i] == nullptr)
        continue;

      std::cout << "  ";

      PrintConstInfo(i, *cf.ConstPool[i], cf.ConstPool);
    }

    std::cout << "\n";
  }

  std::cout << "Access flags: 0x" << std::uppercase << std::hex
            << cf.AccessFlags << std::nouppercase << std::dec;

  std::cout << " (";
  {
    auto flagStrings = GetClassFlags(cf.AccessFlags);

    for(auto i = 0u; i < flagStrings.size(); i++)
    {
      std::cout << flagStrings[i];

      if(i+1 != flagStrings.size())
        std::cout << ", ";
    }
  }
  std::cout << ")\n";

  std::cout << "ThisClass: ";
  std::cout << cf.ConstPool.GetConstNameOrTypeStr(cf.ThisClass) << '\n';

  std::cout << "SuperClass: ";
  std::cout << cf.ConstPool.GetConstNameOrTypeStr(cf.SuperClass) << '\n';

  std::cout << "Interfaces count: " << cf.Interfaces.size();

  if(PrintDetails)
  {
    for(size_t i = 0; i < cf.Interfaces.size(); i++)
    {
      std::cout << "\n  Interfaces[" << i << "] = " <<cf.Interfaces[i];
    }
  }

  std::cout << "\n";

  std::cout << "\nMethods:\n";

  for(const auto& method : cf.Methods)
  {
    std::cout << "  ";
    std::cout << cf.ConstPool.GetConstNameOrTypeStr(method.DescriptorIndex);
    std::cout << " - ";
    std::cout << cf.ConstPool.GetConstNameOrTypeStr(method.NameIndex);
    std::cout << " [";

    auto flagStrings = GetMethodFlags(method);

    for(auto i = 0u; i < flagStrings.size(); i++)
    {
      std::cout << flagStrings[i];

      if(i+1 != flagStrings.size())
        std::cout << ", ";
    }

    std::cout << "]";

    if(PrintDetails)
    {
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
          PrintInstrInfo(*codeAttr.Code[j]);
        }

      }
    }

    std::cout << "\n";
  }

  std::cout << "\nFields:\n";

  for(const auto& field : cf.Fields)
  {
    std::cout << "  ";
    std::cout << cf.ConstPool.GetConstNameOrTypeStr(field.NameIndex);
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
    std::cout << cf.ConstPool.GetConstNameOrTypeStr(field.DescriptorIndex);
    std::cout << "\n";
  }


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
    std::cout << "ERROR: " << errOrClass.GetError().GetMessage() << '\n';
    return -1;
  }

  std::cout << "Parsed " << infile.tellg() << '/' << fileSize << " bytes ";
  std::cout << "in ~" << std::chrono::duration_cast<std::chrono::nanoseconds>(after-before).count() / 1000000.0f << " milliseconds\n\n";

  ClassFile::ClassFile cf = errOrClass.Release();
  PrintClassInfo(cf);

  return 0;
}
