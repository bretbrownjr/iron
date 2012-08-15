// standard includes
#include <cstdlib>

// iron includes
#include "iron/lex.h"
#include "iron/parse.h"

using File = iron::File;
using LexCode = iron::LexCode;
template<typename Ttype>
using Shared = std::shared_ptr<Ttype>;
using Token = iron::Token;
using AstNode = iron::ast::Node;
template<typename Ttype>
using PtrRange = iron::PtrRange<Ttype>;

auto tokenize(Shared<File> file) -> decltype(iron::lex(file))
{
  auto tokens = iron::lex(file);
  if (tokens.isEmpty())
  {
    switch (iron::lexCode)
    {
      case LexCode::ok:
      {
        break;
      }
      case LexCode::bad_file:
      {
        iron::errorln("Expected a file name as the first argument to iron.");
        iron::errorln('\'', file->path(), "' is not a valid Iron source file.");
        break;
      }
      case LexCode::no_match:
      {
        iron::errorln("Warning: No tokens parsed from '", file->path(), '\'');
        break;
      }
      default:
      {
        iron::errorln("Internal Compiler Error: "
          "Invalid lex status code (", iron::lexCode, ") detected at ",
          __FILE__, ':', __LINE__);
        break;
      }
    }
  }
  return std::move(tokens);
}

Shared<AstNode> makeAst(Shared<File> file, PtrRange<Token> tokens)
{
  auto ast = iron::ast::parse(tokens);
  if (!ast)
  {
    iron::errorln("Failed to parse '", file->path(), "'");
  }
  return ast;
}

int main(int argc, char* argv[])
{
  using File = iron::File;

  int status = -1;

  if (getenv("INFO") != nullptr) { iron::infoOn = true; }
  if (getenv("SILENT") != nullptr) { iron::errorOn = false; }

  if (argc < 2)
  {
    iron::errorln("Expected a file name as the first argument to iron.");
    return -1;
  }

  auto file = std::make_shared<File>(File{argv[1]});
  auto tokens = tokenize(file);
  if (tokens.isEmpty()) { return -1; }

  auto ast = makeAst(file, tokens.all());
  if (!ast) { return -1; }

  iron::println(stdout, "Thanks for using Iron!");
  return status;
}

