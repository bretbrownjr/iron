// standard includes
#include <cstdlib>
#include <unistd.h>
#include <vector>

// iron includes
#include "iron/generate.h"
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
using String = std::string;
template<typename Ttype>
using Vector = std::vector<Ttype>;

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

struct Options
{
  Vector<String> ins;
  String out;

  static Options parse(int argc, char* argv[])
  {
    opterr = 0;
    const char options[] = "-o:";

    Options opts;
    int flag = getopt(argc, argv, options);
    while (flag != -1)
    {
      switch (flag)
      {
        case 1 :
        {
          // No flag was given for this option. It is a file name.
          opts.ins.emplace_back(optarg);
          break;
        }
        case 'o' :
        {
          opts.out = String(optarg);
          break;
        }
        default :
        {
          iron::errorln("Unhandled option: ", (char) optopt);
          break;
        }
      }
      flag = getopt(argc, argv, options);
    }

    return std::move(opts);
  }
};

int main(int argc, char* argv[])
{
  using File = iron::File;

  if (getenv("INFO") != nullptr) { iron::infoOn = true; }
  if (getenv("SILENT") != nullptr) { iron::errorOn = false; }

  auto options = Options::parse(argc, argv);

  if (options.ins.empty())
  {
    iron::errorln("The Iron compiler needs a file name to operate on.");
    return -1;
  }
  else if (options.ins.size() > 1)
  {
    iron::errorln("The Iron compiler can only handle one input file at this time.");
    return -1;
  }

  auto file = std::make_shared<File>(options.ins.front());
  auto tokens = tokenize(file);
  if (tokens.isEmpty()) { return -1; }

  auto ast = makeAst(file, tokens.all());
  if (!ast) { return -1; }

  iron::generate(ast, options.out);

  iron::println(stdout, "Thanks for using Iron!");
  return 0;
}

