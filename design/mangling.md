# Iron Name Mangling #

## Background ##

Iron will include a namespace/module feature, though the nomenclature and
detailed design of this feature is still in development. This file is intended
to capture thoughts about that development until a detailed design document can
be developed.

## Justification ##

Every sufficiently complex program needs the ability to differentiate between
similar parts of a program. Take, for example, printing character strings to
the standard output stream.

In C:
```c
#include <stdio.h>
int main(int argc, char* argv[])
{
  /* printf? Why not print? */
  printf("Hello, world!\n");
  return 0;
}
```
... C defines printf as a standard library function because it is a formatted
print method. The only way in C to differentiate between formatted and
unformatted print methods is to change the name of the function. What's more,
renaming is the only way to differentiate between developer-defined functions
and standard library functions! If a library wanted a print function that
printed a string as if it were formatted with markdown, it is not an option
to also name that function ```print```.

Therefore, languages like C implicitly reserve all function names in the
standard library. In practice, commonly-used libraries also reserve all their
function names. As a result, most C headers prefix all externally-visible
symbols with a prefix to avoid name collisions. In essence, C programmers are
creating their own namespaces through naming conventions.

Seeing this common issue, newer languages have been baking in some sort of
namespacing or module feature. This allows nice features like being able to
import or use namespaces to avoid redundant scoping prefixes. For example:

```c
#include <stdio.h>
#include "mymodule.h"
int main(int argc, char* argv[])
{
  /* There are a lot of characters that don't communicate information to the
     development team. Instead, they communicate namespacing information
     to the compiler and linker. */
  const char c_msg[] = "Hello, world!\n";
  mymodule_string_t* message = MyModule_AllocateFromCString(c_msg);
  printf("%s", MyModule_ToCString(message));
  MyModule_Free(message);
  return 0;
}
```
...becomes...
```c++
#include <cstdio>
#include "mymodule.h"
int main(int argc, char* argv[])
{
  /* Much cleaner! */
  using mymodule;
  const char c_msg[] = "Hello, world!\n";
  string_t* message = AllocateFromCString(c_msg);
  printf("%s", ToCString(message);
  Free(message);
  return 0;
}
```

