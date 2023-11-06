#pragma once

#include "fmt/core.h"

#include "ClassFile/Defs.hpp"
#include "ClassFile/Error.hpp"

using namespace ClassFile;

#if defined(__BYTE_ORDER__) 
  #define HOST_BYTEORDER_IS_LE (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__)
#else
  static_assert(false, "Unable to determine host endianness");
#endif
  

enum ByteOrder
{
  LittleEndian,
  BigEndian
};

constexpr ByteOrder GetHostByteOrder()
{
  return HOST_BYTEORDER_IS_LE ? ByteOrder::LittleEndian : ByteOrder::BigEndian;
}

template <typename T>
void SwapByteOrder(T& t)
{
  char* bytes = reinterpret_cast<char*>(&t);
  size_t len = sizeof(t);

  char tmp;
  for (size_t i = 0; i < (len / 2); i++)
  {
    tmp = bytes[i];
    bytes[i] = bytes[len - 1 - i];
    bytes[len - 1 - i] = tmp;
  }

}

template <ByteOrder Order = LittleEndian, typename T>
ErrorOr<void> Read(std::istream& stream, T& t)
{
  stream.read(reinterpret_cast<char*>(&t), sizeof(T));

  if (stream.bad())
  {
    return Error{fmt::format("Read: stream went bad at 0x{0:x} after trying to "
        "read a \"{}\" ({} bytes)", static_cast<size_t>(stream.tellg()), typeid(T).name(), sizeof(T))};
  }

  if (Order != GetHostByteOrder())
    SwapByteOrder(t);

  return {};
}

template <ByteOrder Order = LittleEndian, typename T>
ErrorOr<void> Write(std::ostream& stream, const T& t)
{
  if (Order != GetHostByteOrder())
  {
    T temp(t);
    SwapByteOrder(temp);
    stream.write(reinterpret_cast<char*>(&temp), sizeof(T));
  }
  else
  {
    stream.write(reinterpret_cast<const char*>(&t), sizeof(T));
  }

  if (stream.bad())
  {
    return Error{fmt::format("Write: stream went bad at 0x{0:x} after trying to "
        "write a \"{}\" ({} bytes)", static_cast<size_t>(stream.tellp()), typeid(T).name(), sizeof(T))};
  }

  return {};
}

template <ByteOrder Order = LittleEndian, typename... Args>
ErrorOr<void> Read(std::istream& stream, Args&... args)
{
  return (Read<Order>(stream, args), ...);
}

template <ByteOrder Order = LittleEndian, typename... Args>
ErrorOr<void> Write(std::ostream& stream, const Args&... args)
{
  return (Write<Order>(stream, args), ...);
}
