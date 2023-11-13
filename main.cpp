#include <iostream>
#include "argparse.h"
using namespace std;

void print_namespace(map<string, vector<string>> args);

int main(int nargs, char** argv){
	auto parser = argparse::ArgumentParser(nargs, argv)
		.argument_default(argparse::SUPPRESS)
		.formatter_class(argparse::HelpFormatter::ArgumentDefaults);
	
	parser.add_argument<bool>("-a", "--alpha").help("a help");
	parser.add_argument<bool>("-b").help("b help");
	parser.add_argument("-f", "--foo").nargs(argparse::REMAINDER).help("foo help");
	parser.add_argument("-B", "--bar").nargs("*").help("bar help");
	parser.add_argument<int>("-n", "--num").help("num help");
	
	parser.add_argument("first").help("first help");
	parser.add_argument("second").help("second help");
	
	auto args = parser.parse_args();

	argparse::print_args(args);
	
	return 0;
}
