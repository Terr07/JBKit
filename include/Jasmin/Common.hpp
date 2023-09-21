#pragma once

#include <memory>
#include <stdexcept>
#include <string_view>
#include <queue>
#include <variant>
#include <fstream>
#include <unordered_map>
#include <exception>
#include <unordered_set>
#include <sstream>
#include <tuple>
#include <functional>

namespace Jasmin
{

template <typename T>
using uPtr = std::unique_ptr<T>;


} //namespace: Jasmin
