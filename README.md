# CPP Argument Parser (argparse)

A command-line argument parser for C++11 (and later) that mimics python's `argparse` module using an instance of `argparse::ArgumentParser`. This file
will mostly describe differences between this package and python's module. Familiarity with the module is expected and can be found
[here](https://docs.python.org/3/library/argparse.html).

## Table of Contents

- [Using `cpp-argparse`](#using-cpp-argparse)
- [Creating an `ArgumentParser`](#creating-an-argumentparser)
	- [Chain modifiers](#chain-modifiers)
 - [Adding arguments with `add_argument<T>()`](#adding-arguments-with-add_argumentt)
	- [The `nargs()` chain modifier](#the-nargs-chain-modifier)
 	- [Actions](#actions)
  - [The `parse_args()` method](#the-parse_args-method)
  - [Other utilities](#other-utilities)
  	- [Sub-commands](#sub-commands)
   	- [Parser defaults](#parser-defaults) 

## Using `cpp-argparse`

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

## Creating an `ArgumentParser`

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
method is templated and can accept common built-in types (`int`, `float`, `double`, `bool`, and `std::string`). By default, if no type is provided, the
method will add a string argument.
```C++
parser.add_argument<int>("integers").metavar("N").nargs("+")
	.help("an integer for the accumulator");
parser.add_argument<bool>("--sum").dest("accumulate")
	.constant("sum").def("max")
	.help("sum the integers (default: find the max)");
```
Note that the `action` keyword argument in python's `argparse` module is replaced by different chain modifiers and the keyword argument `default` is
replaced by the modifier `Argument::def(string)`.
Finally, we need to parse command-line arguments. For this example, we will instead feed in arguments into the function itself. (Note that the 
seperator `--` is used to denote the end of optional arguments. This will help the parser recognize `-1` as a negative number and not an 
optional argument.)
```C++
auto args = parser.parse_args(vector<string>{"--sum", "--", "7", "-1", "2"});
argparse::print_args(args);
// Prints: Namespace(accumulate=[sum],integers=[7,-1,2])
```
If no argument is passed into `ArgumentParser::parse_args(vector<string>)`, then the command-line arguments provided in the constructor will be used as expected.
Note that this parser cannot yet recognize abbreviations of long options.

### Chain modifiers

Chain modifiers that are not yet implemented include `prefix_chars()`, `exit_on_error()`, and `allow_abrev()`. For now, exitting on errors is done by
default and abbreviation recognition are not yet supported. 

The `parents` keyword argument is now the `ArgumentParser::parent(const ArgumentParser&)` chain modifier. Rather than accepting a list of 
parser instances, this will only take one and add the optional and positional arguments from it.

## Adding arguments with `add_argument<T>()`

Using this method, we can define how command-line arguments should be parsed. The `ArgumentParser::add_argument<T>(...)` method returns a reference to
an `Argument` instance, allowing the use of the class's own chain modifiers. Once again, these chain modifiers replace the use of keyword arguments in
python's module. For example, if we wanted to add an optional integer argument that accepts three numbers, we could write the following code.
```C++
auto parser = argparse::ArgumentParser(nargs, argv);
parser.add_argument<int>("-n", "--num").nargs(3).help("your favorite numbers");
parser.parse_args(vector<string>{"--num", "1", "2", "3"});
```
Note that instead of using a chain modifier for the `type` keyword argument, instead the function is templated for common built-in types. Another
notable difference is default values are provided by `Argument::def(string)` instead of `default`. The chain modifiers `metavar(...)`, `constant(...)`,
and `choices(...)` all take a delimited string as the first argument and an optional delimiter character (defaulted to `,`).

### The `nargs()` chain modifier

There are three ways to provide the number of arguments to parse to an `Argument` instance: `Argument::nargs(int)`, `Argument::nargs(char)`, and
`Argument::nargs(string)`. Currently, the python's `argparse` module's `argparse.REMAINDER` is not fully implemented. While it can be provided, it
will be set to `'*'` when parsing.

### Actions

This parser does not yet support all actions present in python's `argparse` module. Additionally, there is no chain modifier for the `action` keyword
argument. The current supported actions are `Store` (the default action), `StoreConst`, `Count`, `Version`, and `Help`. The following list explains
how to set the argument to each of the actions.

* `StoreConst`: Provide the constants using the `Argument::constant(string, char=',')` chain modifier. The method accepts a delimited
string and an optional delimiter character (defaulted to `,`). The value(s) will be stored when the flag is present, rather than storing provided
arguments from the command-line. 
* `Count`: Use the `Argument::count()` chain modifier.
* `Version`: Use the `Argument::version(string) chain modifier and provide the appropriate version string.
* `Help`: Use the `Argument::print_help()` chain modifier. Note that the `Argument::help(string)` chain modifier is for providing help text for the
  argument.

More actions will be implemented. Feel free to create an issue requesting certain actions.

## The `parse_args()` Method

Placeholder text.

## Other utilities

Not all of python's `argparse` module has been re-created here. Those that have been added are explained below. If a feature is not mentioned, it is
safe to assume it has not yet been implemented.

### Sub-commands

Sub-commands work essentially the same as in python. A notable difference is that there is no option to change the "parser class" of the subparsers,
so they are all restricted to the `ArgumentParser` class. There are no plans to add this currently.

### Parser defaults

To set parser-level defaults, you can use the `ArgumentParser::set_defaults(map<string,string>)` method. Instead of arbitrarily-named keyword
arguments, the method takes in a mapping of a string to a string to set the defaults. For example, you could set a parser-level default with the
following code.
```C++
auto parser = argparse::ArgumentParser(nargs, argv);
parser.add_argument("-f", "--foo").def("bar").help("foo help");
parser.set_defaults(map<string,string>{ {"foo", "spam"} });
auto args = parser.parse_args(vector<string>{""});
//Namespace(foo=[spam])
```
Retrieving the defaults of arguments can be done with the `ArgumentParser::get_default(std::string)` method. This will return an `ArgumentValueList`,
just like from `ArgumentParser::parse_args()`.

(Documentation WIP. More will be added soon.)
