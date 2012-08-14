// standard includes
#include <cstdlib>

// iron includes
#include "iron/lex.h"

int main(int argc, char* argv[])
{
  using File = iron::File;
  using LexCode = iron::LexCode;

  int status = -1;

  if (getenv("INFO") != nullptr) { iron::infoOn = true; }
  if (getenv("SILENT") != nullptr) { iron::errorOn = false; }

  if (argc < 2)
  {
    iron::errorln("Expected a file name as the first argument to iron.");
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
        iron::errorln("Expected a file name as the first argument to iron.");
        iron::errorln('\'', argv[1], "' is not a valid Iron source file.");
        status = -1;
        break;
      }
      case LexCode::no_match:
      {
        iron::errorln("Warning: No tokens parsed from '", argv[1], '\'');
        status = 0;
        break;
      }
      default:
      {
        iron::errorln("Internal Compiler Error: "
          "Invalid lex status code (", iron::lexCode, ") detected at ",
          __FILE__, ':', __LINE__);
        status = -1;
        break;
      }
    }
  }
  iron::println(stdout, "Thanks for using Iron!");
  return status;
}

