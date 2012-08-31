#pragma once

// iron includes
#include "iron/range.h"

namespace iron
{

/// @brief dynamic (resizeable) array
template<typename Ttype>
class Darray
{
private :
  byte_t* _buffer;
  size_t _bufferSize;
  size_t _count;

public :
  Darray() : _buffer(nullptr), _bufferSize(0), _count(0) {}
  /// @brief copying disabled (for now)
  Darray(const Darray&) = delete;
  Darray(Darray&& moveThis) : Darray()
  {
    swap(moveThis);
  }
  ~Darray()
  {
    if (_buffer != nullptr) { free(_buffer); }
  }

  PtrRange<Ttype> all() { return {ptr(), ptr() + _count - 1}; }
  PtrRange<Ttype> all() const { return {ptr(), ptr() + _count - 1}; }
  size_t count() const { return _count; }

  bool isEmpty() const { return _count == 0; }

  void pushBack(const Ttype& copyThis)
  {
    if ((sizeof(Ttype) * (_count + 1)) > _bufferSize) { grow(); }
    (void) new (ptr(_count)) Ttype(copyThis);
    ++_count;
  }

  void pushBack(Ttype&& moveThis)
  {
    if ((sizeof(Ttype) * (_count + 1)) > _bufferSize) { grow(); }
    (void) new (ptr(_count)) Ttype(std::move(moveThis));
    ++_count;
  }

  void swap(Darray& swapThis)
  {
    std::swap(_buffer, swapThis._buffer);
    std::swap(_bufferSize, swapThis._bufferSize);
    std::swap(_count, swapThis._count);
  }

private :
  Ttype* ptr(size_t index = 0)
  {
    return reinterpret_cast<Ttype*>(_buffer) + index;
  }

  Ttype* ptr(size_t index = 0) const
  {
    return reinterpret_cast<Ttype*>(_buffer) + index;
  }

  void grow()
  {
    _bufferSize = (_bufferSize > 0) ? (_bufferSize * 2) : sizeof(Ttype);
    _buffer = reinterpret_cast<byte_t*>(realloc(_buffer, _bufferSize));
  }
};

} // namespace iron

