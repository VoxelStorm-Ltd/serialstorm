#pragma once
#include <type_traits>

/// Cast a value to the target type, performing a static_cast only when the
/// source and destination types differ.  This allows callers to write generic
/// code without incurring unnecessary (or compiler-warned) self-casts when the
/// types are already the same.
template<typename To, typename From>
inline To cast_if_required(From const &value) {
  if constexpr(std::is_same_v<To, From>) {
    return value;
  } else {
    return static_cast<To>(value);
  }
}
