#pragma once

// iron includes
#include "iron/range.h"

namespace iron
{

int print(FILE* file) { (void) file; return 0; }

int print(FILE* file, Ascii str)
{
  for (size_t i=0; i<str.size(); ++i)
  {
    fputc(str[i], file);
  }
  return 0;
}

int print(FILE* file, const char* c_str)
{
  if (0 < fprintf(file, "%s", c_str))
  {
    return errno;
  }
  return 0;
}

int print(FILE* file, std::string&& str)
{
  if (0 < fprintf(file, "%s", str.c_str()))
  {
    return errno;
  }
  return 0;
}

int print(FILE* file, const std::string& str)
{
  if (0 < fprintf(file, "%s", str.c_str()))
  {
    return errno;
  }
  return 0;
}

int print(FILE* file, char c)
{
  if (0 < fprintf(file, "%c", c))
  {
    return errno;
  }
  return 0;
}

int print(FILE* file, size_t number)
{
  if (0 < fprintf(file, "%lu", number))
  {
    return errno;
  }
  return 0;
}

template<typename Ttype, typename... Ttypes>
int print(FILE* file, Ttype&& firstArg, Ttypes&&... otherArgs)
{
  auto code = print(file, std::forward<Ttype>(firstArg));
  if (code == 0)
  {
    code = print(file, std::forward<Ttypes>(otherArgs)...);
  }
  return code;
}

template<typename... Ttypes>
int println(FILE* file, Ttypes&&... args)
{
  return print(file, std::forward<Ttypes>(args)..., "\n");
}

template<typename... Ttypes>
int errorln(Ttypes&&... args)
{
  return println(stderr, "Error: ", std::forward<Ttypes>(args)...);
}

template<typename... Ttypes>
int infoln(Ttypes&&... args)
{
  return println(stdout, std::forward<Ttypes>(args)...);
}

} // namespace iron

