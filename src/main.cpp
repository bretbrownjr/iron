// iron includes
#include "iron/lex.h"

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
      case LexCode::no_match:
      {
        fprintf(stdout, "Warning: No tokens parsed from '%s'\n", argv[1]);
        status = 0;
        break;
      }
      default:
      {
        fprintf(stderr, "Internal Compiler Error: "
          "Invalid lex status code %d detected at %s:%d\n", iron::lexCode,
          __FILE__, __LINE__);
        status = -1;
        break;
      }
    }
  }
  fprintf(stdout,"Thanks for using Iron!\n");
  return status;
}

