#pragma once

#include "Defs.hpp"
#include "Error.hpp"

#include <vector>
#include <memory>
#include <functional>
#include <cassert>

namespace ClassFile
{

// Macro to iterate every standard java bytecode instruction. Takes a macro as 
// argument (_DEF) which it calls for every stanard instruction, passing instruction 
// info as arguments to it in this order: OPVALUE, NAME, OPERAND_TYPES+.
//
// See use in OPCODE enum definition.
#define _ITERATE_OPCODES(_DEF)  \
  _DEF(0x00, NOP           )\
  _DEF(0x01, ACONST_NULL   )\
  _DEF(0x02, ICONST_M1     )\
  _DEF(0x03, ICONST_0      )\
  _DEF(0x04, ICONST_1      )\
  _DEF(0x05, ICONST_2      )\
  _DEF(0x06, ICONST_3      )\
  _DEF(0x07, ICONST_4      )\
  _DEF(0x08, ICONST_5      )\
  _DEF(0x09, LCONST_0      )\
  _DEF(0x0A, LCONST_1      )\
  _DEF(0x0B, FCONST_0      )\
  _DEF(0x0C, FCONST_1      )\
  _DEF(0x0D, FCONST_2      )\
  _DEF(0x0E, DCONST_0      )\
  _DEF(0x0F, DCONST_1      )\
  _DEF(0x10, BIPUSH,  S8  )\
  _DEF(0x11, SIPUSH,  S16  )\
  _DEF(0x12, LDC,     U8   )\
  _DEF(0x13, LDC_W,   U16  )\
  _DEF(0x14, LDC2_W,  U16  )\
  _DEF(0x15, ILOAD,   U8   )\
  _DEF(0x16, LLOAD,   U8   )\
  _DEF(0x17, FLOAD,   U8   )\
  _DEF(0x18, DLOAD,   U8   )\
  _DEF(0x19, ALOAD,   U8   )\
  _DEF(0x1A, ILOAD_0       )\
  _DEF(0x1B, ILOAD_1       )\
  _DEF(0x1C, ILOAD_2       )\
  _DEF(0x1D, ILOAD_3       )\
  _DEF(0x1E, LLOAD_0 )\
  _DEF(0x1F, LLOAD_1 )\
  _DEF(0x20, LLOAD_2 )\
  _DEF(0x21, LLOAD_3 )\
  _DEF(0x22, FLOAD_0 )\
  _DEF(0x23, FLOAD_1 )\
  _DEF(0x24, FLOAD_2 )\
  _DEF(0x25, FLOAD_3 )\
  _DEF(0x26, DLOAD_0 )\
  _DEF(0x27, DLOAD_1 )\
  _DEF(0x28, DLOAD_2 )\
  _DEF(0x29, DLOAD_3 )\
  _DEF(0x2A, ALOAD_0 )\
  _DEF(0x2B, ALOAD_1 )\
  _DEF(0x2C, ALOAD_2 )\
  _DEF(0x2D, ALOAD_3 )\
  _DEF(0x2E, IALOAD )\
  _DEF(0x2F, LALOAD )\
  _DEF(0x30, FALOAD )\
  _DEF(0x31, DALOAD )\
  _DEF(0x32, AALOAD )\
  _DEF(0x33, BALOAD )\
  _DEF(0x34, CALOAD )\
  _DEF(0x35, SALOAD )\
  _DEF(0x36, ISTORE, U8 )\
  _DEF(0x37, LSTORE, U8 )\
  _DEF(0x38, FSTORE, U8 )\
  _DEF(0x39, DSTORE, U8 )\
  _DEF(0x3A, ASTORE, U8 )\
  _DEF(0x3B, ISTORE_0 )\
  _DEF(0x3C, ISTORE_1 )\
  _DEF(0x3D, ISTORE_2 )\
  _DEF(0x3E, ISTORE_3 )\
  _DEF(0x3F, LSTORE_0 )\
  _DEF(0x40, LSTORE_1 )\
  _DEF(0x41, LSTORE_2 )\
  _DEF(0x42, LSTORE_3 )\
  _DEF(0x43, FSTORE_0 )\
  _DEF(0x44, FSTORE_1 )\
  _DEF(0x45, FSTORE_2 )\
  _DEF(0x46, FSTORE_3 )\
  _DEF(0x47, DSTORE_0 )\
  _DEF(0x48, DSTORE_1 )\
  _DEF(0x49, DSTORE_2 )\
  _DEF(0x4A, DSTORE_3 )\
  _DEF(0x4B, ASTORE_0 )\
  _DEF(0x4C, ASTORE_1 )\
  _DEF(0x4D, ASTORE_2 )\
  _DEF(0x4E, ASTORE_3 )\
  _DEF(0x4F, IASTORE )\
  _DEF(0x50, LASTORE )\
  _DEF(0x51, FASTORE )\
  _DEF(0x52, DASTORE )\
  _DEF(0x53, AASTORE )\
  _DEF(0x54, BASTORE )\
  _DEF(0x55, CASTORE )\
  _DEF(0x56, SASTORE )\
  _DEF(0x57, POP )\
  _DEF(0x58, POP2 )\
  _DEF(0x59, DUP )\
  _DEF(0x5A, DUP_X1 )\
  _DEF(0x5B, DUP_X2 )\
  _DEF(0x5C, DUP2 )\
  _DEF(0x5D, DUP2_X1 )\
  _DEF(0x5E, DUP2_X2 )\
  _DEF(0x5F, SWAP )\
  _DEF(0x60, IADD )\
  _DEF(0x61, LADD )\
  _DEF(0x62, FADD )\
  _DEF(0x63, DADD )\
  _DEF(0x64, ISUB )\
  _DEF(0x65, LSUB )\
  _DEF(0x66, FSUB )\
  _DEF(0x67, DSUB )\
  _DEF(0x68, IMUL )\
  _DEF(0x69, LMUL )\
  _DEF(0x6A, FMUL )\
  _DEF(0x6B, DMUL )\
  _DEF(0x6C, IDIV )\
  _DEF(0x6D, LDIV )\
  _DEF(0x6E, FDIV )\
  _DEF(0x6F, DDIV )\
  _DEF(0x70, IREM )\
  _DEF(0x71, LREM )\
  _DEF(0x72, FREM )\
  _DEF(0x73, DREM )\
  _DEF(0x74, INEG )\
  _DEF(0x75, LNEG )\
  _DEF(0x76, FNEG )\
  _DEF(0x77, DNEG )\
  _DEF(0x78, ISHL )\
  _DEF(0x79, LSHL )\
  _DEF(0x7A, ISHR )\
  _DEF(0x7B, LSHR )\
  _DEF(0x7C, IUSHR )\
  _DEF(0x7D, LUSHR )\
  _DEF(0x7E, IAND )\
  _DEF(0x7F, LAND )\
  _DEF(0x80, IOR )\
  _DEF(0x81, LOR )\
  _DEF(0x82, IXOR )\
  _DEF(0x83, LXOR )\
  _DEF(0x84, IINC, U8, S8 )\
  _DEF(0x85, I2L )\
  _DEF(0x86, I2F )\
  _DEF(0x87, I2D )\
  _DEF(0x88, L2I )\
  _DEF(0x89, L2F )\
  _DEF(0x8A, L2D )\
  _DEF(0x8B, F2I )\
  _DEF(0x8C, F2L )\
  _DEF(0x8D, F2D )\
  _DEF(0x8E, D2I )\
  _DEF(0x8F, D2L )\
  _DEF(0x90, D2F )\
  _DEF(0x91, I2B )\
  _DEF(0x92, I2C )\
  _DEF(0x93, I2S )\
  _DEF(0x94, LCMP )\
  _DEF(0x95, FCMPL )\
  _DEF(0x96, FCMPG )\
  _DEF(0x97, DCMPL )\
  _DEF(0x98, DCMPG )\
  _DEF(0x99, IFEQ, S16)\
  _DEF(0x9A, IFNE, S16)\
  _DEF(0x9B, IFLT, S16 )\
  _DEF(0x9C, IFGE, S16 )\
  _DEF(0x9D, IFGT, S16 )\
  _DEF(0x9E, IFLE, S16 )\
  _DEF(0x9F, IF_ICMPEQ, S16 )\
  _DEF(0xA0, IF_ICMPNE, S16 )\
  _DEF(0xA1, IF_ICMPLT, S16 )\
  _DEF(0xA2, IF_ICMPGE, S16 )\
  _DEF(0xA3, IF_ICMPGT, S16 )\
  _DEF(0xA4, IF_ICMPLE, S16 )\
  _DEF(0xA5, IF_ACMPEQ, S16 )\
  _DEF(0xA6, IF_ACMPNE, S16 )\
  _DEF(0xA7, GOTO, S16 )\
  _DEF(0xA8, JSR, S16 )\
  _DEF(0xA9, RET, U8)\
  /*_DEF(0xAA, TABLESWITCH )*/\
  /*_DEF(0xAB, LOOKUPSWITCH )*/\
  _DEF(0xAC, IRETURN )\
  _DEF(0xAD, LRETURN )\
  _DEF(0xAE, FRETURN )\
  _DEF(0xAF, DRETURN )\
  _DEF(0xB0, ARETURN )\
  _DEF(0xB1, RETURN )\
  _DEF(0xB2, GETSTATIC, U16)\
  _DEF(0xB3, PUTSTATIC, U16 )\
  _DEF(0xB4, GETFIELD, U16 )\
  _DEF(0xB5, PUTFIELD, U16 )\
  _DEF(0xB6, INVOKEVIRTUAL, U16 )\
  _DEF(0xB7, INVOKESPECIAL, U16 )\
  _DEF(0xB8, INVOKESTATIC, U16 )\
  _DEF(0xB9, INVOKEINTERFACE, U16, U8, U8)\
  _DEF(0xBA, INVOKEDYNAMIC, U16, U8, U8)\
  _DEF(0xBB, NEW, U16)\
  _DEF(0xBC, NEWARRAY, U8 )\
  _DEF(0xBD, ANEWARRAY, U16 )\
  _DEF(0xBE, ARRAYLENGTH )\
  _DEF(0xBF, ATHROW )\
  _DEF(0xC0, CHECKCAST, U16)\
  _DEF(0xC1, INSTANCEOF, U16)\
  _DEF(0xC2, MONITORENTER )\
  _DEF(0xC3, MONITOREXIT )\
  /*_DEF(0xC4, WIDE )*/\
  _DEF(0xC5, MULTIANEWARRAY, U16, U8)\
  _DEF(0xC6, IFNULL, U16 )\
  _DEF(0xC7, IFNONNULL, U16)\
  _DEF(0xC8, GOTO_W, S32)\
  _DEF(0xC9, JSR_W, S32)\
  _DEF(0xCA, BREAKPOINT )\
  _DEF(0xFE, IMPDEP1 )\
  _DEF(0xFF, IMPDEP2 )


enum class OPCODE : U8 
{
  #define _ENUM_ENTRY(OPVALUE, NAME, ...) NAME = OPVALUE,
  _ITERATE_OPCODES(_ENUM_ENTRY)
  #undef _ENUM_ENTRY

    /*
  TABLESWITCH  = 0xAA,
  LOOKUPSWITCH = 0xAB,
  WIDE         = 0xC4,
  */
};

const char* GetMnemonicFromOpcode(OPCODE);
ErrorOr<OPCODE> GetOpcodeFromMnemonic(std::string_view);

struct Instruction
{
  virtual OPCODE GetOpCode() const = 0;
  virtual size_t GetNOperands() const = 0;

  enum class OperandType
  {
    U8,
    U16,
    S8,
    S16,
    S32,
  };
  virtual ErrorOr<OperandType> GetOperandType(size_t) const = 0;

  virtual ErrorOr<S32> GetOperand(size_t) const = 0;
  virtual ErrorOr<void> SetOperand(size_t, S32) = 0;

  std::string_view GetMnemonic() const;

  virtual U32 GetLength() const
  {
    U32 len{1}; //opcode

    for(size_t i{0}; i < this->GetNOperands(); i++)
    {
      OperandType type = this->GetOperandType(i).Get();

      switch(type)
      {
        case OperandType::U8: 
        case OperandType::S8:  len += 1; break;
        case OperandType::U16: 
        case OperandType::S16: len += 2; break;
        case OperandType::S32: len += 4; break;
      }
    }

    return len;
  }

  static ErrorOr<std::unique_ptr<Instruction>> NewInstruction(OPCODE);

  //TODO: static alignment array & function
};


template <OPCODE OP, typename... Ts>
struct InstructionT: public Instruction
{
  public:
    std::tuple<Ts...> Operands;

    OPCODE GetOpCode() const override{ return OP; }
    size_t GetNOperands() const override { return std::tuple_size<decltype(Operands)>::value; }

    ErrorOr<OperandType> GetOperandType(size_t i) const override 
    {
      auto errOrTuple = this->dynamicGet(i, std::integral_constant<size_t, 0>());

      if(errOrTuple.IsError())
        return errOrTuple.GetError();

      return std::get<1>(errOrTuple.Get());
    }

    ErrorOr<S32> GetOperand(size_t i) const override
    {
      auto errOrTuple = this->dynamicGet(i, std::integral_constant<size_t, 0>());

      if(errOrTuple.IsError())
        return errOrTuple.GetError();

      return std::get<0>(errOrTuple.Get());
    }

    ErrorOr<void> SetOperand(size_t i, S32 val) override
    {
      return this->dynamicSet(i, val, std::integral_constant<size_t, 0>());
    }

  private:
    //basically a dynamic std::get<n> implementation where N doesn't have to be a constexpr value
    //instantiates itself over and over again incrementing the integral_constant each time
    //
    //If N becomes greater than the tuple size, this template function will stop getting instantiated and instead
    //the catch-all function below gets called which returns an error.
    template <size_t N, typename = std::enable_if_t<std::tuple_size<decltype(Operands)>::value != N> >
    ErrorOr<std::tuple<S32, OperandType>> dynamicGet(size_t i, std::integral_constant<size_t, N>) const
    {
      if(i == N)
      {
        auto getTypeEnum = [](const auto& operand) -> OperandType
        {
          if(typeid(operand) == typeid(U8))  return OperandType::U8;
          if(typeid(operand) == typeid(U16)) return OperandType::U16;
          if(typeid(operand) == typeid(S8))  return OperandType::S8;
          if(typeid(operand) == typeid(S16)) return OperandType::S16;
          if(typeid(operand) == typeid(S32)) return OperandType::S32;

          assert(false);
          return OperandType::U8;
        };

        auto operand = std::get<N>(this->Operands);
        return std::tuple<S32, OperandType>{static_cast<S32>(operand), getTypeEnum(operand)};
      }
      else
        return dynamicGet(i, std::integral_constant<size_t, N+1>());
    }

    ErrorOr<std::tuple<S32, OperandType>> dynamicGet(size_t i, size_t n) const
    { return Error::FromLiteralStr("Invalid operand index requested.");}

    template <size_t N, typename = std::enable_if_t<std::tuple_size<decltype(Operands)>::value != N> >
    ErrorOr<void> dynamicSet(size_t i, S32 val, std::integral_constant<size_t, N>) 
    {
      if(i == N)
      {
        auto& operand = std::get<N>(this->Operands);
        operand = static_cast< std::remove_reference_t<decltype(operand)>>(val);
        return {};
      }
      else
        return dynamicSet(i, val, std::integral_constant<size_t, N+1>());
    }

    ErrorOr<void> dynamicSet(size_t i, S32 val, size_t n) const
    { return Error::FromLiteralStr("Invalid operand index requested.");}
};

namespace Instructions
{

//this bullshit just results in: "using BIPUSH = InstructionT<OPCODE::BIPUSH, U8>;"
//for each defined instruction as defined in _ITERATE_OPCODES. Kinda disgusting 
//macro use, but i prefer it to typing out the instructions 250+ times here + 
//all the other places i would have to repeat the list as well if i didnt use
//the preprocessor at all. Blame c++'s lack of refleciton.
#define _GET_FIRST(FIRST, ...) FIRST
#define _USING_STATEMENT(OPVALUE, ...) \
  using _GET_FIRST(__VA_ARGS__) = InstructionT<OPCODE::__VA_ARGS__>;\

_ITERATE_OPCODES( _USING_STATEMENT )

#undef _GET_FIRST
#undef _USING_STATEMENT

//defiend in the source file to prevent the _ITERATE_OPCODES macro from being
//undefined, as the source file needs it to generate some code from it aswell. 
#ifndef _DONT_UNDEF_ITERATE_OPCODES
  #undef _ITERATE_OPCODES
#endif

} //namespace Instructions

} //namespace ClassFile

