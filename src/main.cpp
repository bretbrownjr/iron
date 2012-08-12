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
    lexCode = LexCode::ok;
  }

  return std::move(tokens);
}

} // namespace iron

int main(int argc, char* argv[])
{
  using File = iron::File;
  using LexCode = iron::LexCode;

  int status = -1;

  if (argc < 2)
  {
    fprintf(stderr, "Expected a file name as the first argument to iron.\n");
    return -1;
  }

  auto file = std::make_shared<File>(File{argv[1]});
  auto tokens = lex(file);
  if (tokens.empty())
  {
    switch (iron::lexCode)
    {
      case LexCode::ok:
      {
        status = 0;
        break;
      }
      case LexCode::bad_file:
      {
        fprintf(stderr, "'%s' is not a valid Iron source file.\n", argv[1]);
        status = -1;
        break;
      }
      case LexCode::no_tokens:
      {
        fprintf(stdout, "Warning: No tokens parsed from '%s'\n", argv[1]);
        status = 0;
        break;
      }
      default:
      {
        fprintf(stderr, "Internal Compiler Error: "
          "Bad lex status code %d detected at %s:%d\n", iron::lexCode,__FILE__,__LINE__);
        status = -1;
        break;
      }
    }
  }
  fprintf(stdout,"Thanks for using Iron!\n");
  return status;
}

