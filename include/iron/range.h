#pragma once

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

  constexpr bool isEmpty() { return _begin < _end; }
  constexpr size_t size() { return _end - _begin + 1; }
};

} // namespace iron

