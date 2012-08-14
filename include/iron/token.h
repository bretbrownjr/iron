#pragma once

// iron includes
#include "iron/print.h"
#include "iron/range.h"

namespace iron
{

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
  enum class Type
  {
    bad,
    ampersind,
    asterisk,
    at,
    back_slash,
    back_tick,
    bang,
    caret,
    colon,
    comma,
    dollar,
    equals,
    fwd_slash,
    greater_than,
    keyword_fn,
    keyword_ret,
    identifier,
    left_brace,
    left_bracket,
    left_paren,
    less_than,
    minus,
    number,
    octothorpe,
    percent,
    period,
    plus,
    pipe,
    question,
    right_brace,
    right_bracket,
    right_paren,
    semicolon,
    tilde
  };

  /// @brief The type of token
  Type type;
  /// @brief The position of this token in its file
  Pos pos;
  /// @brief The substring of the file for this token
  PtrRange<const byte_t> value;
};

int print(FILE* file, Pos pos)
{
  return print(file, pos.row, ',', pos.col);
}

} // namespace iron

