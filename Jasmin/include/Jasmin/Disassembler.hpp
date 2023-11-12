#pragma once

#include <ClassFile/ClassFile.hpp>

#include "Common.hpp"
#include "Lexer.hpp"

namespace Jasmin
{

namespace CF = ClassFile;

struct DisassemblerConfig
{
  bool DisableHeaderComments = false;
};

class Disassembler
{
  using Config = DisassemblerConfig;

  public:
    static void Disassemble(const CF::ClassFile&, std::ostream&, Config = {});

  private:
    Disassembler(const CF::ClassFile&, std::ostream&, Config);

    void dismHeader();
    void dismSourceDirective();

    void dismFields();
    void dismMethods();

    std::runtime_error error(std::string_view);

  private:
    const CF::ClassFile& cf;
    std::ostream& out;
    const Config config;
};

} //namespace: Jasmin
