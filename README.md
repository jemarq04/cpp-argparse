# CPP Argument Parser (argparse)

A command-line argument parser for C++11 (and later) that mimics python's `argparse` module using an instance of `argparse::ArgumentParser`. This file
will mostly describe differences between this package and python's module. Familiarity with the module is expected and can be found
[here](https://docs.python.org/3/library/argparse.html).

## Using cpp-argparse

To get started, move the `argparse.h` release file into your working area and add `#include "argparse.h"` at the top of your C++ script. Now you 
can access all of the classes and enums located in the `argparse` namespace. (If you'd prefer `argparse.h` in a different location, just be sure 
to include its directory when compiling.)

We need to access the command-line arguments to be able to parse them, so be sure to access them within your main function. An example template 
would be the following snippet.
```C++
#include "argparse.h"

int main(int nargs, char** argv){
  //Here is where our argument parsing would take place.
  return 0;
}
```

# Creating an ArgumentParser Class

First, we need to create an argument parser instance. This can be created simply using the `ArgumentParser` class's only constructor. The variables 
passed into the constructor are the same as in the template in the the [previous section](#using-cpp-argparse).
```C++
auto parser = argparse::ArgumentParser(nargs, argv);
```
Most keyword arguments available in python's `argparse` module are available as _chain modifiers_ provided in the `ArgumentParser` class. For example,
we can modify more of the help text by creating a parser with the following.
```C++
auto parser = argparse::ArgumentParser(nargs, argv).prog("ProgramName")
	.description("Detailed description of the script.")
	.epilog("Text at the bottom of the help screen.");
```
Afterwards, we would want to add arguments to this example parser. We would do this by calling the `ArgumentParser::add_argument<T>(...)` method. This
method is templated and can accept most built-in types (`int`, `float`, `double`, `bool`, and `std::string`). By default, if no type is provided, the
method will add a string argument.
```C++
parser.add_argument<int>("integers").metavar("N").nargs("+")
	.help("an integer for the accumulator");
parser.add_argument<bool>("--sum").dest("accumulate")
	.constant("sum").def("max")
	.help("sum the integers (default: find the max)");
```
Note that the `action` keyword argument in python's `argparse` module is replaced by different chain modifiers and the keyword argument `default` is
replaced by the modifier `Argument::def(...)`. This will be described further in the [following section](#placeholder) on `Argument`s.
Finally, we need to parse command-line arguments. For this example, we will instead feed in arguments into the function itself. (Note that the 
seperator `--` is used to denote the end of optional arguments. This will help the parser recognize `-1` as a negative number and not an 
optional argument.)
```C++
auto args = parser.parse_args(vector<string>{"--sum", "--", "7", "-1", "2"});
argparse::print_args(args);
// Prints: Namespace(accumulate=[sum],integers=[7,-1,2])
```
If no argument is passed into `ArgumentParser::parse_args(...)`, then the command-line arguments provided in the constructor will be used as expected.

## Chain Modifiers

# Placeholder

(Documentation WIP. More will be added soon.)
