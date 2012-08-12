#pragma once

/// standard includes
#include <string>

namespace iron
{
using String = std::string;

class File
{
private:
  FILE* _handle;
  const String _path;
  size_t _size;

public:
  File() = delete;
  File(const File&) = delete;
  File(File&& moveThis) : File(moveThis._path)
  {
    _handle = moveThis._handle;
    moveThis._handle = nullptr;
    _size = moveThis._size;
  }
  File(String path) : _handle(nullptr), _path(path)
  {
    _handle = fopen(path.c_str(), "r");
    _size = initSize();
  }
  ~File()
  {
    close();
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

