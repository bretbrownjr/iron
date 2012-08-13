#pragma once

// standard includes
#include <cctype>
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
  enum class Type
  {
    bad
  };

  /// @brief The type of token
  Type type;
  /// @brief The position of this token in its file
  Pos pos;
  /// @brief The substring of the file for this token
  PtrRange<const ubyte_t> value;
};

enum class LexCode
{
  ok,
  bad_file,
  no_match
};

/// @brief The status code for the last run of @ref lex
/// @todo TODO: Make this variable thread-local
LexCode lexCode;

LexCode lexKeyword(Vector<Token>& tokens, PtrRange<const ubyte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexIdentifier(Vector<Token>& tokens, PtrRange<const ubyte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexNumberLiteral(Vector<Token>& tokens, PtrRange<const ubyte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexStringLiteral(Vector<Token>& tokens, PtrRange<const ubyte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexCharLiteral(Vector<Token>& tokens, PtrRange<const ubyte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexPunctuation(Vector<Token>& tokens, PtrRange<const ubyte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexToken(Vector<Token>& tokens, PtrRange<const ubyte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  const auto& c = bytes.front();
  if (c == '\n')
  {
    ++pos.row;
    pos.col = 0;
  }
  else if (c == ' ' || c == '\t')
  {
    ++pos.col;
  }
  else if (isalpha(c))
  {
    const auto lexKeywordCode = lexKeyword(tokens, bytes, pos);
    if (lexKeywordCode == LexCode::ok ||
        lexKeywordCode != LexCode::no_match)
    {
      return lexKeywordCode;
    }

    const auto lexIdentifierCode = lexIdentifier(tokens, bytes, pos);
    if (lexIdentifierCode == LexCode::ok ||
        lexIdentifierCode != LexCode::no_match)
    {
      return lexIdentifierCode;
    }
  }
  else if (isdigit(c))
  {
    return lexNumberLiteral(tokens, bytes, pos);
  }
  else if (c == '\"')
  {
    return lexStringLiteral(tokens, bytes, pos);
  }
  else if (c == '\'')
  {
    return lexCharLiteral(tokens, bytes, pos);
  }
  else
  {
    return lexPunctuation(tokens, bytes, pos);
  }

  return LexCode::no_match;
}

/// @brief Breaks a file up into Tokens
Vector<Token> lex(Shared<File> file)
{
  lexCode = LexCode::bad_file;

  if (file->isEmpty() || file->isValid() == false)
  {
    return {};
  }

  Vector<Token> tokens;
  fprintf(stdout, "Lexing '%s'\n", file->path().c_str());
  auto bytes = file->all();
  fprintf(stdout, "Read %lu bytes from '%s'\n", bytes.size(), file->path().c_str());
  Pos pos = {0, 0};
  while (!bytes.isEmpty())
  {
    fprintf(stdout, "Attempting to lex a token at %s:%lu,%lu\n",
        file->path().c_str(), pos.row, pos.col);
    const auto code = lexToken(tokens, bytes, pos);
    if (code != LexCode::ok)
    {
      lexCode = code;
      fprintf(stderr, "Error: Failed to lex a token at %s:%lu,%lu\n",
        file->path().c_str(), pos.row, pos.col);
      return {};
    }
  }

  // Report a successful lex
  lexCode = LexCode::ok;
  return std::move(tokens);
}

} // namespace iron

