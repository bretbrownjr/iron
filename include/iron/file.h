#pragma once

/// standard includes
#include <string>

/// iron includes
#include "iron/range.h"

namespace iron
{
using ubyte_t = unsigned char;
static_assert(sizeof(ubyte_t) == 1, "ubyte_t must be 1 byte long.");
using String = std::string;

class File
{
private:
  const ubyte_t* _buffer;
  FILE* _handle;
  const String _path;
  const size_t _size;

public:
  File() = delete;
  File(const File&) = delete;
  File(File&& moveThis) : _path(moveThis._path), _size(moveThis._size)
  {
    _handle = moveThis._handle;
    moveThis._handle = nullptr;
    _buffer = moveThis._buffer;
    moveThis._buffer = nullptr;
  }
  File(String path) : _handle(fopen(path.c_str(), "r")), _path(path), _size(initSize())
  {
    _buffer = reinterpret_cast<const ubyte_t*>(malloc(_size));;
    // TODO: Handle cstdio error codes
    fread(const_cast<ubyte_t*>(_buffer), sizeof(ubyte_t), _size, _handle);
  }
  ~File()
  {
    close();
    if (_buffer != nullptr)
    {
      free(const_cast<ubyte_t*>(_buffer));
      _buffer = nullptr;
    }
  }

  PtrRange<const ubyte_t> all() const
  {
    return isEmpty() ?
        PtrRange<const ubyte_t>{} :
        PtrRange<const ubyte_t>{_buffer, _buffer + _size - 1};
  }

  void close()
  {
    if (isOpen()) { fclose(_handle); _handle = nullptr; }
  }
  bool isEmpty() const { return _size == 0; }
  bool isOpen() const { return _handle != nullptr; }
  bool isValid() const { return true; }
  String path() const { return _path; }
  bool size() const { return _size; }

private:
  /// Only call this in a constructor. It will trash the seek pointer for _handle.
  /// @pre @p this is open (@ref isOpen).
  size_t initSize()
  {
// TODO: Report cstdio error codes
    fseek (_handle, 0, SEEK_END);
// TODO: Report cstdio error codes
    const auto size = ftell(_handle);
// TODO: Report cstdio error codes
    fseek (_handle, 0, SEEK_SET);
    return static_cast<size_t>(size);
  }
};

} // namespace iron

