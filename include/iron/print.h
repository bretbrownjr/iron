#pragma once

// iron includes
#include "iron/range.h"

namespace iron
{

int info() { return 0; }

int info(Ascii str)
{
  for (size_t i=0; i<str.size(); ++i)
  {
    fputc(str[i], stdout);
  }
  return 0;
}

int info(const char* c_str)
{
  if (0 < fprintf(stdout, "%s", c_str))
  {
    return errno;
  }
  return 0;
}

int info(std::string&& str)
{
  if (0 < fprintf(stdout, "%s", str.c_str()))
  {
    return errno;
  }
  return 0;
}

int info(const std::string& str)
{
  if (0 < fprintf(stdout, "%s", str.c_str()))
  {
    return errno;
  }
  return 0;
}

int info(char c)
{
  if (0 < fprintf(stdout, "%c", c))
  {
    return errno;
  }
  return 0;
}

int info(size_t number)
{
  if (0 < fprintf(stdout, "%lu", number))
  {
    return errno;
  }
  return 0;
}

template<typename Ttype, typename... Ttypes>
int info(Ttype&& firstArg, Ttypes&&... otherArgs)
{
  auto code = info(std::forward<Ttype>(firstArg));
  if (code == 0)
  {
    code = info(std::forward<Ttypes>(otherArgs)...);
  }
  return code;
}

template<typename... Ttypes>
int infoln(Ttypes&&... args)
{
  return info(std::forward<Ttypes>(args)..., "\n");
}

} // namespace iron

