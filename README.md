# CPP Argument Parser (argparse)
A command-line argument parser for C++11 (and later) that mimics python's `argparse` module using an instance of `argparse::ArgumentParser`.

## Using cpp-argparse
To get started, move the `argparse.h` release file into your working area and add `#include "argparse.h"` at the top of your C++ script. Now you can access all of the classes and enums located in the `argparse` namespace. (If you'd prefer `argparse.h` in a different location, just be sure to include its directory when compiling.)

We need to access the command-line arguments to be able to parse them, so be sure to access them within your main function. An example template would be the following snippet.
```C++
#include "argparse.h"

int main(int nargs, char** argv){
  //Here is where our argument parsing would take place.
  return 0;
}
```

# Creating and using the ArgumentParser Class
First, we need to create an argument parser instance. This can be created simply using the `ArgumentParser` class's only constructor. The variables passed into the constructor are the same as in the template in the the [previous section](#using-cpp-argparse).
```C++
auto parser = argparse::ArgumentParser(nargs, argv);
```

(Documentation WIP. More will be added soon.)
