#define _DONT_UNDEF_ITERATE_OpcodeS
#include "ClassFile/Instruction.hpp"

#include <fmt/core.h>

#include "Util/IO.hpp"
#include "Util/Error.hpp"

using namespace ClassFile;

//                                    mnemonic           format
using instrInfo = const std::tuple<std::string_view, std::string_view>;
constexpr std::array<instrInfo, ClassFile::Instruction::Opcode::_N> infoTable = 
{{
  {"nop",            ""},
  {"aconst_null",    ""},
  {"iconst_m1",      ""},
  {"iconst_0",       ""},
  {"iconst_1",       ""},
  {"iconst_2",       ""},
  {"iconst_3",       ""},
  {"iconst_4",       ""},
  {"iconst_5",       ""},
  {"lconst_0",       ""},
  {"lconst_1",       ""},
  {"fconst_0",       ""},
  {"fconst_1",       ""},
  {"fconst_2",       ""},
  {"dconst_0",       ""},
  {"dconst_1",       ""},
  {"bipush",         "B"},
  {"sipush",         "S"},
  {"ldc",            "b"},
  {"ldc_w",          "s"},
  {"ldc2_w",         "s"},
  {"iload",          "b"},
  {"lload",          "b"},
  {"fload",          "b"},
  {"dload",          "b"},
  {"aload",          "b"},
  {"iload_0",        ""},
  {"iload_1",        ""},
  {"iload_2",        ""},
  {"iload_3",        ""},
  {"lload_0",        ""},
  {"lload_1",        ""},
  {"lload_2",        ""},
  {"lload_3",        ""},
  {"fload_0",        ""},
  {"fload_1",        ""},
  {"fload_2",        ""},
  {"fload_3",        ""},
  {"dload_0",        ""},
  {"dload_1",        ""},
  {"dload_2",        ""},
  {"dload_3",        ""},
  {"aload_0",        ""},
  {"aload_1",        ""},
  {"aload_2",        ""},
  {"aload_3",        ""},
  {"iaload",         ""},
  {"laload",         ""},
  {"faload",         ""},
  {"daload",         ""},
  {"aaload",         ""},
  {"baload",         ""},
  {"caload",         ""},
  {"saload",         ""},
  {"istore",         "b"},
  {"lstore",         "b"},
  {"fstore",         "b"},
  {"dstore",         "b"},
  {"astore",         "b"},
  {"istore_0",       ""},
  {"istore_1",       ""},
  {"istore_2",       ""},
  {"istore_3",       ""},
  {"lstore_0",       ""},
  {"lstore_1",       ""},
  {"lstore_2",       ""},
  {"lstore_3",       ""},
  {"fstore_0",       ""},
  {"fstore_1",       ""},
  {"fstore_2",       ""},
  {"fstore_3",       ""},
  {"dstore_0",       ""},
  {"dstore_1",       ""},
  {"dstore_2",       ""},
  {"dstore_3",       ""},
  {"astore_0",       ""},
  {"astore_1",       ""},
  {"astore_2",       ""},
  {"astore_3",       ""},
  {"iastore",        ""},
  {"lastore",        ""},
  {"fastore",        ""},
  {"dastore",        ""},
  {"aastore",        ""},
  {"bastore",        ""},
  {"castore",        ""},
  {"sastore",        ""},
  {"pop",            ""},
  {"pop2",           ""},
  {"dup",            ""},
  {"dup_x1",         ""},
  {"dup_x2",         ""},
  {"dup2",           ""},
  {"dup2_x1",        ""},
  {"dup2_x2",        ""},
  {"swap",           ""},
  {"iadd",           ""},
  {"ladd",           ""},
  {"fadd",           ""},
  {"dadd",           ""},
  {"isub",           ""},
  {"lsub",           ""},
  {"fsub",           ""},
  {"dsub",           ""},
  {"imul",           ""},
  {"lmul",           ""},
  {"fmul",           ""},
  {"dmul",           ""},
  {"idiv",           ""},
  {"ldiv",           ""},
  {"fdiv",           ""},
  {"ddiv",           ""},
  {"irem",           ""},
  {"lrem",           ""},
  {"frem",           ""},
  {"drem",           ""},
  {"ineg",           ""},
  {"lneg",           ""},
  {"fneg",           ""},
  {"dneg",           ""},
  {"ishl",           ""},
  {"lshl",           ""},
  {"ishr",           ""},
  {"lshr",           ""},
  {"iushr",          ""},
  {"lushr",          ""},
  {"iand",           ""},
  {"land",           ""},
  {"ior",            ""},
  {"lor",            ""},
  {"ixor",           ""},
  {"lxor",           ""},
  {"iinc",           "sS"},
  {"i2l",            ""},
  {"i2f",            ""},
  {"i2d",            ""},
  {"l2i",            ""},
  {"l2f",            ""},
  {"l2d",            ""},
  {"f2i",            ""},
  {"f2l",            ""},
  {"f2d",            ""},
  {"d2i",            ""},
  {"d2l",            ""},
  {"d2f",            ""},
  {"i2b",            ""},
  {"i2c",            ""},
  {"i2s",            ""},
  {"lcmp",           ""},
  {"fcmpl",          ""},
  {"fcmpg",          ""},
  {"dcmpl",          ""},
  {"dcmpg",          ""},
  {"ifeq",           "S"},
  {"ifne",           "S"},
  {"iflt",           "S"},
  {"ifge",           "S"},
  {"ifgt",           "S"},
  {"ifle",           "S"},
  {"if_icmpeq",      "S"},
  {"if_icmpne",      "S"},
  {"if_icmplt",      "S"},
  {"if_icmpge",      "S"},
  {"if_icmpgt",      "S"},
  {"if_icmple",      "S"},
  {"if_acmpeq",      "S"},
  {"if_acmpne",      "S"},
  {"goto",           "S"},
  {"jsr",            "S"},
  {"ret",            "b"},
  {"tableswitch",    "c"}, //complex
  {"lookupswitch",   "c"}, //complex
  {"ireturn",        ""},
  {"lreturn",        ""},
  {"freturn",        ""},
  {"dreturn",        ""},
  {"areturn",        ""},
  {"return",         ""},
  {"getstatic",      "s"},
  {"putstatic",      "s"},
  {"getfield",       "s"},
  {"putfield",       "s"},
  {"invokevirtual",  "s"},
  {"invokespecial",  "s"},
  {"invokestatic",   "s"},
  {"invokeinterface","sbb"},
  {"invokedynamic",  "sbb"},
  {"new",            "s"},
  {"newarray",       "b"},
  {"anewarray",      "s"},
  {"arraylength",    ""},
  {"athrow",         ""},
  {"checkcast",      "s"},
  {"instanceof",     "s"},
  {"monitorenter",   ""},
  {"monitorexit",    ""},
  {"wide",           "c"}, //TODO
  {"multianewarray", "sb"},
  {"ifnull",         "s"},
  {"ifnonnull",      "s"},
  {"goto_w",         "I"},
  {"jsr_w",          "I"},
  {"breakpoint",     ""},
 }};

static constexpr std::string_view mnemonic(Instruction::Opcode op)
{
  assert(static_cast<int>(op) < Instruction::Opcode::_N);
  return std::get<0>(infoTable[op]);
}

static constexpr std::string_view format(Instruction::Opcode op)
{
  assert(static_cast<int>(op) < Instruction::Opcode::_N);
  return std::get<1>(infoTable[op]);
}

ErrorOr<Instruction> Instruction::MakeInstruction(Opcode op)
{
  Instruction instr;
  instr.Op = op;

  size_t totalOperandSize{0};
  for(size_t i{0}; i < Instruction::GetNOperands(op); ++i)
    totalOperandSize += Instruction::GetOperandSize(op, i);

  instr.operandBytes.resize(totalOperandSize);

  return instr;
}

std::string_view Instruction::GetMnemonic(Instruction::Opcode op)
{
  return mnemonic(op);
}

size_t Instruction::GetNOperands(Instruction::Opcode op)
{
  return format(op).size();
}

Instruction::OperandType Instruction::GetOperandType(Instruction::Opcode op, size_t index) 
{
  assert( format(op).empty()       ||
          format(op)[index] == 'I' ||
          format(op)[index] == 'S' ||
          format(op)[index] == 'B' ||
          format(op)[index] == 's' ||
          format(op)[index] == 'b'   );

  return static_cast<Instruction::OperandType>(format(op)[index]);
}

size_t Instruction::GetOperandSize(Instruction::Opcode op, size_t index) 
{
  size_t size;

  switch( Instruction::GetOperandType(op, index) )
  {
    case 'I': size = sizeof(S32); break;
    case 'S': size = sizeof(S16); break;
    case 'B': size = sizeof(S8);  break;
    case 's': size = sizeof(U16); break;
    case 'b': size = sizeof(U8);  break;
  }

  return size;
}

size_t Instruction::GetLength(Instruction::Opcode op)
{
  assert(!Instruction::IsComplex(op));

  //opcode is 1 byte
  size_t len{1};

  for(size_t i{0}; i < Instruction::GetNOperands(op); ++i)
  {
    switch( Instruction::GetOperandType(op, i) )
    {
      case 'I': len+=sizeof(S32); break;
      case 'S': len+=sizeof(S16); break;
      case 'B': len+=sizeof(S8);  break;
      case 's': len+=sizeof(U16); break;
      case 'b': len+=sizeof(U8);  break;
    }
  }

  return len;
}

bool Instruction::IsComplex(Instruction::Opcode op) 
{
  return format(op)[0] == 'c';
}

std::string_view Instruction::GetMnemonic() const
{
  return Instruction::GetMnemonic(this->Op);
}

size_t Instruction::GetNOperands() const
{
  return Instruction::GetNOperands(this->Op);
}

Instruction::OperandType Instruction::GetOperandType(size_t index) const
{
  return Instruction::GetOperandType(this->Op, index);
}

size_t Instruction::GetOperandSize(size_t index) const
{
  return Instruction::GetOperandSize(this->Op, index);
}

size_t Instruction::GetLength() const
{
  return Instruction::GetLength(this->Op);
}

bool Instruction::IsComplex() const
{
  return Instruction::IsComplex(this->Op);
}

template <typename T>
static ErrorOr<S32> verifyGetOpr(size_t index, const Instruction* instr)
{
  auto errOrRef = instr->Operand<T>(index);
  VERIFY(errOrRef);

  return S32{ errOrRef.Get().get() };
}

ErrorOr<S32> Instruction::GetOperand(size_t index) const
{
  switch( this->GetOperandType(index) )
  {
    case 'I': return verifyGetOpr<S32>(index, this);
    case 'S': return verifyGetOpr<S16>(index, this);
    case 'B': return verifyGetOpr<S8 >(index, this);
    case 's': return verifyGetOpr<U16>(index, this);
    case 'b': return verifyGetOpr<U8 >(index, this);

    default: assert(false);
  }
}

template <typename T>
static ErrorOr<void> verifySetOpr(size_t index, S32 value, Instruction* instr)
{
  auto errOrRef = instr->Operand<T>(index);
  VERIFY(errOrRef);

  T& ref = errOrRef.Get().get();
  ref = static_cast<T>(value);

  return NoError{};
}

ErrorOr<void> Instruction::SetOperand(size_t index, S32 value) 
{
  switch( this->GetOperandType(index) )
  {
    case 'I': return verifySetOpr<S32>(index, value, this);
    case 'S': return verifySetOpr<S16>(index, value, this);
    case 'B': return verifySetOpr<S8 >(index, value, this);
    case 's': return verifySetOpr<U16>(index, value, this);
    case 'b': return verifySetOpr<U8 >(index, value, this);

    default: assert(false);
  }
}

Error Instruction::oobError(size_t index) const
{
  return Error{ fmt::format("Instruction: out-of-bounds operand access "
      "at index {}, instruction has {} operand(s).", index, this->GetNOperands())};
}

Error Instruction::castError(size_t index, std::string_view type) const
{
  return Error{ fmt::format("Instruction: invalid operand type cast at index {}, "
      "failed to cast to \"{}\"", index, type)};
}

