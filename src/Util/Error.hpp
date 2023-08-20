#pragma once

#define TRY( expr )\
{\
  auto errOr = (expr);\
\
  if(errOr.IsError())\
    return errOr.GetError();\
}

#define VERIFY( errOr )\
{\
  if (errOr.IsError())\
    return errOr.GetError();\
}
