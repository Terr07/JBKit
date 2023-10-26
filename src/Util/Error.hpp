#pragma once

#include <fmt/core.h>

#define TRY_2( expr, throwMsg )\
{\
  auto errOr = (expr);\
\
  if(errOr.IsError())\
    return Error{ fmt::format("{} threw: {}\n  {}", __func__, throwMsg, errOr.GetError().What)};\
}

#define VERIFY_2( errOr, throwMsg )\
{\
  if (errOr.IsError())\
    return Error{ fmt::format("{} threw: {}\n  {}", __func__, throwMsg, errOr.GetError().What)};\
}

#define TRY_1( expr ) TRY_2(expr, "")
#define VERIFY_1( errOr ) VERIFY_2(errOr, "")


#define GET_MACRO(_1, _2, MACRO, ...) MACRO

#define TRY(...) GET_MACRO(__VA_ARGS__, TRY_2, TRY_1)(__VA_ARGS__)
#define VERIFY(...) GET_MACRO(__VA_ARGS__, VERIFY_2, VERIFY_1)(__VA_ARGS__)

