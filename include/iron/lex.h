#pragma once

// standard includes
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

// iron includes
#include "iron/file.h"

namespace iron
{
template<typename Ttype>
using Shared = std::shared_ptr<Ttype>;

using String = std::string;

template<typename Ttype>
using Vector = std::vector<Ttype>;

/// @brief The position of a substring in a file
struct Pos
{
  /// @brief column number
  size_t col;
  /// @brief row (line) number
  size_t row;
};

/// @brief A syntactic unit of the Iron language
struct Token
{
  /// @brief The file this token was found in
  Shared<File> file;
  /// @brief The position of this token in its file
  Pos pos;
};

enum class LexCode
{
  ok,
  bad_file,
  no_tokens
};

/// @brief The status code for the last run of @ref lex
/// @todo TODO: Make this variable thread-local
LexCode lexCode;

/// @brief Breaks a file up into Tokens
Vector<Token> lex(Shared<File> file)
{
  Vector<Token> tokens;
  lexCode = LexCode::bad_file;

  if (!file->isEmpty() && file->isValid())
  {
    const auto bytes = file->all();
    fprintf(stdout, "'%s' has %lu bytes.\n", file->path().c_str(), bytes.size());
    lexCode = LexCode::ok;
  }

  return std::move(tokens);
}

} // namespace iron

