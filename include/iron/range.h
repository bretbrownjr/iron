#pragma once

// iron includes
#include "iron/types.h"

namespace iron
{

template<typename Ttype>
class PtrRange
{
private :
  /// @brief begin pointer
  Ttype* _begin;
  /// @brief end pointer
  Ttype* _end;

public :
  constexpr PtrRange() :
      _begin(reinterpret_cast<Ttype*>(1)), _end(reinterpret_cast<Ttype*>(0))
  {}
  constexpr PtrRange(Ttype* b, Ttype* e) : _begin(b), _end(e) {}
  constexpr PtrRange(const PtrRange& copyThis) :
      _begin(copyThis._begin), _end(copyThis._end)
  {}
  PtrRange(PtrRange&& moveThis) : _begin(moveThis._begin), _end(moveThis._end) {}
  ~PtrRange() = default;

  constexpr Ttype& at(size_t size) { return *(_begin+size); }
  constexpr PtrRange<Ttype> first(size_t size) { return {_begin, _begin + size - 1}; }
  Ttype& front() { return *_begin; }
  constexpr bool isEmpty() { return _begin > _end; }
  void pop(size_t size = 1) { _begin += size; }
  constexpr size_t size() { return _end - _begin + 1; }
  bool startsWith(PtrRange<Ttype>&& rhs) const
  {
    return startsWith(rhs);
  }
  bool startsWith(const PtrRange<Ttype>& rhs) const
  {
    if (rhs.size() > size()) { return false; }
    for (size_t i=0; i<rhs.size(); ++i)
    {
      if (rhs.at(i) != at(i)) { return false; }
    }
    return true;
  }
};

using Ascii = PtrRange<const char>;

} // namespace iron

constexpr iron::Ascii operator "" _ascii(const char* str, size_t length)
{
  return iron::Ascii{str, str + length - 1};
}

