#pragma once

// standard includes
#include <cctype>
#include <cstdio>
#include <memory>
#include <string>

// iron includes
#include "iron/darray.h"
#include "iron/file.h"
#include "iron/token.h"

namespace iron
{
template<typename Ttype>
using Shared = std::shared_ptr<Ttype>;

using String = std::string;

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

LexCode lexExact(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos,
    Token::Type type, Ascii str)
{
  if (bytes.startsWith(str))
  {
    infoln("Lexed '", str, "' at ", pos);
    const auto size = str.size();
    tokens.pushBack(Token{type, pos, bytes.first(size)});
    pos.col += size;
    bytes.pop(size);
    return LexCode::ok;
  }
  return LexCode::no_match;
}

LexCode lexKeyword(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
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

LexCode lexIdentifier(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  size_t size = 0;
  auto c = bytes.at(size);
  while (size < bytes.size() &&
      (isalnum(c) || c == '_'))
  {
    ++size;
    c = bytes.at(size);
  }
  if (size > 0)
  {
    auto substr = bytes.first(size);
    bytes.pop(size);
    infoln("Lexed '", substr, "' as a symbol at ", pos);
    tokens.pushBack(Token{Token::Type::identifier, pos, substr});
    pos.col += size;
    return LexCode::ok;
  }
  return LexCode::no_match;
}

LexCode lexNumberLiteral(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  size_t size = 0;
  auto c = bytes.at(size);
  while (size < bytes.size() && isdigit(c))
  {
    ++size;
    c = bytes.at(size);
  }
  if (size > 0)
  {
    auto substr = bytes.first(size);
    bytes.pop(size);
    infoln("Lexed '", substr, "' as a number at ", pos);
    tokens.pushBack(Token{Token::Type::number, pos, substr});
    pos.col += size;
    return LexCode::ok;
  }
  return LexCode::no_match;
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexStringLiteral(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexCharLiteral(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  return LexCode::no_match;
}

LexCode lexSingleChar(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos,
  char c, Token::Type type)
{
  if (bytes.front() == c)
  {
    infoln("Lexed a '", c, "' at ", pos);
    tokens.pushBack(Token{type, pos, bytes.first(1)});
    ++pos.col;
    bytes.pop();
    return LexCode::ok;
  }
  return LexCode::no_match;
}

LexCode lexPunctuation(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
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
  code = lexExact(tokens, bytes, pos, Token::Type::map, "=>"_ascii);
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

LexCode lexToken(Darray<Token>& tokens, PtrRange<const byte_t>& bytes, Pos& pos)
{
  (void) tokens; (void) bytes; (void) pos;
  const auto& c = bytes.front();
  if (c == '\n')
  {
    infoln("Lexed a newline at ", pos);
    ++pos.row;
    pos.col = 1;
    bytes.pop();
    return LexCode::ok;
  }
  else if (c == ' ' || c == '\t')
  {
    infoln("Lexed whitespace at ", pos);
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

    errorln("Token at ", pos, " starts with an alphabet character, so it "
      "should be a keyword or an identifier.");
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
Darray<Token> lex(Shared<File> file)
{
  lexCode = LexCode::bad_file;

  if (file->isEmpty() || file->isValid() == false)
  {
    return {};
  }

  Darray<Token> tokens;
  infoln("Lexing '", file->path());
  auto bytes = file->all();
  infoln("Read ", bytes.size(), " bytes from '", file->path(), "'");
  Pos pos = {1, 1};
  while (!bytes.isEmpty())
  {
    infoln("Attempting to lex a token at ", file->path(), ':', pos);
    const auto code = lexToken(tokens, bytes, pos);
    if (code != LexCode::ok)
    {
      lexCode = code;
      errorln("Failed to lex a token at ", file->path(), ':', pos);
      return {};
    }
  }

  // Report a successful lex
  lexCode = LexCode::ok;
  return std::move(tokens);
}

} // namespace iron

