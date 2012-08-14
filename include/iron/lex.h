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

enum class LexCode
{
  /// @brief Success
  ok,
  /// @brief Failed to read the file
  bad_file,
  /// @brief Could not create a Token. Not necessarily an error.
  no_match,
  /// @brief Internal error.
  lex_error
};

/// @brief The status code for the last run of @ref lex
/// @todo TODO: Make this variable thread-local
LexCode lexCode;

LexCode lexExact(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos,
    Token::Type type, Ascii str)
{
  if (bytes.startsWith(str))
  {
    fprintf(stdout, "Lexed '");
    for (size_t i=0; i<str.size(); ++i)
    {
      fputc(str.at(i), stdout);
    }
    fprintf(stdout, "' at %lu:%lu\n", pos.row, pos.col);
    const auto size = str.size();
    tokens.emplace_back(Token{type, pos, bytes.first(size)});
    pos.col += size;
    bytes.pop(size);
    return LexCode::ok;
  }
  return LexCode::no_match;
}

LexCode lexKeyword(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  auto code = lexExact(tokens, bytes, pos, Token::Type::keyword_fn, "fn"_ascii);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexExact(tokens, bytes, pos, Token::Type::keyword_ret, "ret"_ascii);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  return LexCode::no_match;
}

LexCode lexIdentifier(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  size_t size = 0;
  auto c = bytes.at(size);
  while (size < bytes.size() &&
      (isalnum(c) || c == '_'))
  {
    ++size;
    c = bytes.at(size);
  }
  if (size > 1)
  {
    auto substr = bytes.first(size);
    bytes.pop(size);
    fprintf(stdout, "Lexed '");
    for (size_t i=0; i<substr.size(); ++i)
    {
      fputc(substr.at(i), stdout);
    }
    fprintf(stdout, "' as a symbol at %lu:%lu\n", pos.row, pos.col);
    tokens.emplace_back(Token{Token::Type::identifier, pos, substr});
    pos.col += size;
    return LexCode::ok;
  }
  return LexCode::no_match;
}

LexCode lexNumberLiteral(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  size_t size = 0;
  auto c = bytes.at(size);
  while (size < bytes.size() && isdigit(c))
  {
fprintf(stdout, "%s:%d\n", __FILE__, __LINE__);
    ++size;
    c = bytes.at(size);
  }
  if (size > 0)
  {
    auto substr = bytes.first(size);
    bytes.pop(size);
    fprintf(stdout, "Lexed '");
    for (size_t i=0; i<substr.size(); ++i)
    {
      fputc(substr.at(i), stdout);
    }
    fprintf(stdout, "' as a number at %lu:%lu\n", pos.row, pos.col);
    tokens.emplace_back(Token{Token::Type::number, pos, substr});
    pos.col += size;
    return LexCode::ok;
  }
  return LexCode::no_match;
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexStringLiteral(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexCharLiteral(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexSingleChar(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos,
  char c, Token::Type type)
{
  if (bytes.front() == c)
  {
    fprintf(stdout, "Lexed a '%c' at %lu:%lu\n", c, pos.row, pos.col);
    tokens.emplace_back(Token{type, pos, bytes.first(1)});
    ++pos.col;
    bytes.pop();
    return LexCode::ok;
  }
  return LexCode::no_match;
}

LexCode lexPunctuation(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  auto code = lexSingleChar(tokens, bytes, pos, '{', Token::Type::left_brace);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '}', Token::Type::right_brace);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '(', Token::Type::left_paren);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, ')', Token::Type::right_paren);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '[', Token::Type::left_bracket);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, ']', Token::Type::right_bracket);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, ',', Token::Type::comma);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '.', Token::Type::period);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, ':', Token::Type::colon);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, ';', Token::Type::semicolon);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '-', Token::Type::minus);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '+', Token::Type::plus);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '*', Token::Type::asterisk);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '!', Token::Type::bang);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '=', Token::Type::equals);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '<', Token::Type::less_than);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '>', Token::Type::greater_than);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '/', Token::Type::fwd_slash);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '\\', Token::Type::back_slash);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '|', Token::Type::pipe);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '?', Token::Type::question);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '@', Token::Type::at);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '$', Token::Type::dollar);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '%', Token::Type::percent);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '^', Token::Type::caret);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '&', Token::Type::ampersind);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '#', Token::Type::octothorpe);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '~', Token::Type::tilde);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }
  code = lexSingleChar(tokens, bytes, pos, '`', Token::Type::back_tick);
  if (code == LexCode::ok || code != LexCode::no_match)
  {
    return code;
  }

  return LexCode::no_match;
}

LexCode lexToken(Vector<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  const auto& c = bytes.front();
  if (c == '\n')
  {
    fprintf(stdout, "Lexed a newline at %lu:%lu\n", pos.row, pos.col);
    ++pos.row;
    pos.col = 1;
    bytes.pop();
    return LexCode::ok;
  }
  else if (c == ' ' || c == '\t')
  {
    fprintf(stdout, "Lexed whitespace at %lu:%lu\n", pos.row, pos.col);
    ++pos.col;
    bytes.pop();
    return LexCode::ok;
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

    fprintf(stderr, "Error: Token at %lu:%lu starts with an alphabet "
      "character, so it should be a keyword or an identifier.\n",
      pos.row, pos.col);  
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
  Pos pos = {1, 1};
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

