#pragma once

// standard includes
#include <cerrno>
#include <cstdio>
#include <string>

// iron includes
#include "iron/range.h"

namespace iron
{

inline int print(FILE* file) { (void) file; return 0; }

inline int print(FILE* file, Ascii str)
{
  for (size_t i=0; i<str.size(); ++i)
  {
    fputc(str[i], file);
  }
  return 0;
}

inline int print(FILE* file, const char* c_str)
{
  if (0 < fprintf(file, "%s", c_str))
  {
    return errno;
  }
  return 0;
}

inline int print(FILE* file, std::string&& str)
{
  if (0 < fprintf(file, "%s", str.c_str()))
  {
    return errno;
  }
  return 0;
}

inline int print(FILE* file, const std::string& str)
{
  if (0 < fprintf(file, "%s", str.c_str()))
  {
    return errno;
  }
  return 0;
}

inline int print(FILE* file, char c)
{
  if (0 < fprintf(file, "%c", c))
  {
    return errno;
  }
  return 0;
}

inline int print(FILE* file, size_t number)
{
  if (0 < fprintf(file, "%lu", number))
  {
    return errno;
  }
  return 0;
}

template<typename Ttype, typename... Ttypes>
inline int print(FILE* file, Ttype&& firstArg, Ttypes&&... otherArgs)
{
  auto code = print(file, std::forward<Ttype>(firstArg));
  if (code == 0)
  {
    code = print(file, std::forward<Ttypes>(otherArgs)...);
  }
  return code;
}

template<typename... Ttypes>
inline int println(FILE* file, Ttypes&&... args)
{
  return print(file, std::forward<Ttypes>(args)..., "\n");
}

extern bool infoOn;
extern bool errorOn;

template<typename... Ttypes>
inline int errorln(Ttypes&&... args)
{
  return errorOn ?
    println(stderr, "Error: ", std::forward<Ttypes>(args)...) :
    0;
}

template<typename... Ttypes>
inline int infoln(Ttypes&&... args)
{
  return infoOn ?
    println(stdout, std::forward<Ttypes>(args)...) :
    0;
}

} // namespace iron

