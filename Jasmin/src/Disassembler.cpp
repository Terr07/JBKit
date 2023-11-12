#include <Jasmin/Disassembler.hpp>

#include <fmt/core.h>

#include <algorithm>

using namespace Jasmin;

void Disassembler::Disassemble(const CF::ClassFile& cf, std::ostream& stream, Config conf)
{
  Disassembler dis{cf, stream, conf};
  dis.dismHeader();
  dis.dismFields();
  dis.dismMethods();
}

Disassembler::Disassembler(const CF::ClassFile& c, std::ostream& s, Config conf)
: cf{c}
, out{s}
, config{conf}
{
  if(!config.DisableHeaderComments)
    out << "; Disassembled by JBKit\n";
}

void Disassembler::dismHeader()
{
  out << ".bytecode " << cf.MajorVersion << '.' << cf.MinorVersion << '\n';
  dismSourceDirective();

  out << "; <class-spec>\n";
  out << "; <super-spec>\n";
  out << "; <implements>\n";
  out << "; <implements>\n";

  out << "; [.signature \"<signature>\"]\n";
  out << "; [.enclosing method <method_name>]\n";
  out << "; [.debug \"<debug_source_extension>\"]*\n";
  out << "; [.inner class [<access>] [<name>] [inner <classname>] [outer <name>]]*\n";
  out << "; [.inner interface [<access>] [<name>] [inner <classname>] [outer <name>]]*\n";

  out << '\n';
}

void Disassembler::dismSourceDirective()
{
  auto attrItr = std::find_if(cf.Attributes.begin(), cf.Attributes.end(),
      [](const auto& pAttr){ return pAttr->GetType() == CF::AttributeInfo::Type::SourceFile; });

  if(attrItr == cf.Attributes.end())
    return;

  auto sourceAttr = dynamic_cast<const CF::SourceFileAttribute*>((*attrItr).get());
  assert(sourceAttr != nullptr);

  std::string_view source = 
    cf.ConstPool.GetConstNameOrTypeStr(sourceAttr->SourceFileIndex);

  out << ".source " << source << '\n';
}

void Disassembler::dismFields()
{
  out << "; [<field>]*\n";
  out << '\n';
}

void Disassembler::dismMethods()
{
  out << "; [<method>]*\n";
}

std::runtime_error Disassembler::error(std::string_view msg)
{
  return std::runtime_error{fmt::format("Disassembler: {}", msg)};
}
