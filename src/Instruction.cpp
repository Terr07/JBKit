#define _DONT_UNDEF_ITERATE_OPCODES
#include "ClassFile/Instruction.hpp"

#include "Util/IO.hpp"
#include "Util/Error.hpp"

#include <cassert>
#include <unordered_map>

namespace ClassFile
{


const char* GetMnemonicFromOpcode(OPCODE op)
{

  switch(op)
  {
    #define _CASE_ENTRY(OPVALUE, NAME, ...) case OPCODE::NAME: return #NAME;
    _ITERATE_OPCODES( _CASE_ENTRY )
    #undef _CASE_ENTRY
  }

  assert(false);
  return "";
}

ErrorOr<OPCODE> GetOpcodeFromMnemonic(std::string_view mnemonic)
{
  static std::unordered_map<std::string_view, OPCODE> opcodeNames
  {

    #define _MAP_ENTRY(OPVALUE, NAME, ...) {#NAME, OPCODE::NAME},

    _ITERATE_OPCODES( _MAP_ENTRY )

    #undef _MAP_ENTRY

  };

  if(auto search = opcodeNames.find(mnemonic); search != opcodeNames.end())
    return search->second;


  return Error::FromLiteralStr("unknown opcode mnemonic");
}

std::string_view Instruction::GetMnemonic() const
{
    return GetMnemonicFromOpcode(this->GetOpCode());
}

ErrorOr<std::unique_ptr<Instruction>> Instruction::NewInstruction(OPCODE op) 
{
  switch(op)
  {

    #define _CASE_ENTRY(OPVALUE, NAME, ...) \
      case OPCODE::NAME: return std::make_unique<Instructions::NAME>();

    _ITERATE_OPCODES( _CASE_ENTRY )

    #undef _CASE_ENTRY
  }

  std::cerr << "UNKNOWN OP: " << std::hex << (int)op << std::dec << '\n';
  return Error::FromLiteralStr("FAILED TO MAKE INSTRUCTION, PROBABLY UNKONWN OPCODE");
}



#undef _DONT_UNDEF_ITERATE_OPCODES
#undef _ITERATE_OPCODES

} //namespace ClassFile
