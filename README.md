# CPP Argument Parser (argparse) v2.2.0
A command-line argument parser for C++11 (and later) that mimics python's argparse. This file will focus on how the implementation in C++ differs from python.

# Using cpp-argparse
To get started, move the `argparse.h` file into your working area and include it into your makefile. All that you need to do is add `#include "argparse.h"` at the top of your C++ script, and you can now access all of the classes and enums located in the `argparse` namespace.

We need to access the command-line arguments to be able to parse them, so a template for a simple script would look something like the following snippet.
```C++
#include <iostream>
#include "argparse.h"

int main(int nargs, char** argv){

  //Here is where our argument parsing would take place.

  return 0;
}
```
From here on, we will assume that the number of command-line arguments is `nargs` and the array of c-strings is `argv`.

## ArgumentParser Class
First, we need to create an argument parser instance. This can be created simply using the `ArgumentParser` class's only constructor. (Here we assume the appropriate code surrounding this statement.)
```C++
auto parser = argparse::ArgumentParser(nargs, argv);
```

(Documentation WIP. More will be added soon.)
