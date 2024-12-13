#include "argparse.h"
using namespace argparse;

// === ARGUMENT VALUES === {{{1
//
// Assignment Operators {{{2
template <>
ArgumentValue& ArgumentValue::operator=<bool>(const bool& other){
	*this = other? TRUE : FALSE;
	return *this;
}

// Casting Operators {{{2
ArgumentValue::operator int() const{return strtol(c_str(), nullptr, 10);}
ArgumentValue::operator float() const{return strtof(c_str(), nullptr);}
ArgumentValue::operator double() const{return strtod(c_str(), nullptr);}
ArgumentValue::operator bool() const{return *this == TRUE;}

// Comparison Operators {{{2
bool ArgumentValue::operator==(const char* other) const{
	return *this == std::string(other);
}
bool ArgumentValue::operator> (const char* other) const{
	return *this >  std::string(other);
}
bool ArgumentValue::operator< (const char* other) const{
	return *this <  std::string(other);
}
bool ArgumentValue::operator!=(const char* other) const{
	return *this != std::string(other);
}
bool ArgumentValue::operator>=(const char* other) const{
	return *this >= std::string(other);
}
bool ArgumentValue::operator<=(const char* other) const{
	return *this <= std::string(other);
}

// Arithmetic Operators {{{2
ArgumentValue ArgumentValue::operator+(const char* other) const{
	return (std::string)c_str() + other;
}

// Increment/Decrement Operators {{{2
ArgumentValue& ArgumentValue::operator++(){
	return *this = *this + 1;
}
ArgumentValue& ArgumentValue::operator--(){
	return *this = *this - 1;
}
ArgumentValue  ArgumentValue::operator++(int){
	ArgumentValue temp = *this;
	*this = *this + 1;
	return temp;
}
ArgumentValue  ArgumentValue::operator--(int){
	ArgumentValue temp = *this;
	*this = *this - 1;
	return temp;
}

// Accessors {{{2
bool ArgumentValue::is_none() const{return *this == NONE;}
bool ArgumentValue::is_true() const{return *this == TRUE;}
bool ArgumentValue::is_false() const{return *this != TRUE;}

// === ARGUMENT VALUE LIST === {{{1
//
// Constructors {{{2
ArgumentValueList::ArgumentValueList(std::initializer_list<ArgumentValue> vals){
	for (auto& val : vals) push_back(val);
}
ArgumentValueList::ArgumentValueList(std::vector<std::string> vals){
	for (auto& val : vals) push_back(val);
}

// Assignment Operators {{{2
ArgumentValueList& ArgumentValueList::operator=(const std::vector<std::string>& other){
	clear();
	for (auto& val : other) push_back(val);
	return *this;
}

// Casting Operators {{{2
ArgumentValueList::operator int() const{return strtol(at(0).c_str(), nullptr, 10);}
ArgumentValueList::operator float() const{return strtof(at(0).c_str(), nullptr);}
ArgumentValueList::operator double() const{return strtod(at(0).c_str(), nullptr);}
ArgumentValueList::operator bool() const{return at(0) == TRUE;}
ArgumentValueList::operator std::string() const{return at(0);}

// Comparison Operators {{{2
bool ArgumentValueList::operator==(const char* other) const{
	return at(0) == std::string(other);
}
bool ArgumentValueList::operator> (const char* other) const{
	return at(0) >  std::string(other);
}
bool ArgumentValueList::operator< (const char* other) const{
	return at(0) <  std::string(other);
}
bool ArgumentValueList::operator!=(const char* other) const{
	return at(0) != std::string(other);
}
bool ArgumentValueList::operator>=(const char* other) const{
	return at(0) >= std::string(other);
}
bool ArgumentValueList::operator<=(const char* other) const{
	return at(0) <= std::string(other);
}

// Arithmetic Operators {{{2
ArgumentValueList ArgumentValueList::operator+(const char* other) const{
	ArgumentValueList temp = {(std::string)at(0).c_str() + other};
	return temp;
}

// Increment/Decrement Operators {{{2
ArgumentValueList& ArgumentValueList::operator++(){
	return *this = *this + 1;
}
ArgumentValueList& ArgumentValueList::operator--(){
	return *this = *this - 1;
}
ArgumentValueList  ArgumentValueList::operator++(int){
	ArgumentValueList temp = *this;
	*this = *this + 1;
	return temp;
}
ArgumentValueList  ArgumentValueList::operator--(int){
	ArgumentValueList temp = *this;
	*this = *this - 1;
	return temp;
}	

// Stream Operators {{{2
std::ostream& operator<<(std::ostream& os, const ArgumentValueList& arglist){
	os << arglist.str();
	return os;
}

// Modifiers {{{2
void ArgumentValueList::str(const std::string& val){at(0) = val;}

// Accessors {{{2
std::string ArgumentValueList::str() const{return at(0);}
const char* ArgumentValueList::c_str() const{return at(0).c_str();}
bool ArgumentValueList::is_none() const{return at(0) == NONE;}
bool ArgumentValueList::is_true() const{return at(0) == TRUE;}
bool ArgumentValueList::is_false() const{return at(0) != TRUE;}
std::vector<std::string> ArgumentValueList::vec() const{
	std::vector<std::string> result;
	for (auto& val : *this) result.push_back((std::string)val);
	return result;
}


// === HELPERS === {{{1
std::string argparse::format_args(ArgumentMap args){
	std::string result = "Namespace(";
	bool first = true;
	for (const auto& it : args){
		if (first) first = false;
		else result += ",";
		result +=  it.first + "=";
		if (it.second.size() == 0)
			result += "[]";
		else if (it.second[0] == NONE)
			result += "None";
		else{
			result += "[" + it.second[0];
			for (int i=1; i<it.second.size(); i++)
				result += "," + it.second[i];
			result += "]";
		}
	}
	result += ")";
	return result;
}
void argparse::print_args(ArgumentMap args, std::ostream& out){
	out << format_args(args) << std::endl;
}

// === ARGUMENT PARSER === {{{1
// 
// Constructor {{{2
ArgumentParser::ArgumentParser(int nargs, char **args) 
		: _nargs(nargs-1), _req_posargs(0), _base_req_posargs(0),
		_help_indent(0), _subparser_index(0), _none_str(NONE){
	// Set booleans
	_resolve = _added_help =  false;
	_parsed = _has_digit_opt = _multiline_usage = false;
	_override_usage = _override_prog = _subparser = false;
	_add_help = true;
	
	// Set program name and _args.
	size_t pos = ((std::string)args[0]).rfind('/');
	if (pos != std::string::npos)
		_prog = ((std::string)args[0]).substr(pos+1);
	else _prog = args[0];
	for (int i=1; i<nargs; i++)
		_args.push_back(args[i]);

	// Format usage
	_usage = "usage: " + _prog;
	_usage_line_len = _usage.size();
	_preusage_len = _usage_line_len;
	
	// Set the linecap for line-wrapping from terminal width
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	_linecap = w.ws_col-2;
}

// Error Handling {{{2
void ArgumentParser::error(std::string msg){
	if (_parsed) print_usage(std::cerr);
	
	std::cerr << _prog << ": error";
	if (msg != "") std::cerr << ": " << msg;
	std::cerr << std::endl;
	exit(0);
}

// Modifiers {{{2
template <>
ArgumentParser::Argument& ArgumentParser::add_argument<int>(std::string sname, std::string lname){
	Argument arg(ArgType::Optarg, ValType::Int, sname, lname);
	check_conflict(arg, ArgType::Optarg);
	
	_optlist.push_back(arg);
	return _optlist.back();
}
template <>
ArgumentParser::Argument& ArgumentParser::add_argument<float>(std::string sname, std::string lname){
	Argument arg(ArgType::Optarg, ValType::Float, sname, lname);
	check_conflict(arg, ArgType::Optarg);
	
	_optlist.push_back(arg);
	return _optlist.back();
}
template <>
ArgumentParser::Argument& ArgumentParser::add_argument<double>(std::string sname, std::string lname){
	return add_argument<float>(sname, lname);
}
template <>
ArgumentParser::Argument& ArgumentParser::add_argument<bool>(std::string sname, std::string lname){
	Argument arg(ArgType::Optarg, ValType::Bool, sname, lname);
	check_conflict(arg, ArgType::Optarg);
	
	_optlist.push_back(arg);
	return _optlist.back();
}
ArgumentParser::Argument& ArgumentParser::add_argument(std::string sname, std::string lname){
	Argument arg(ArgType::Optarg, ValType::String, sname, lname);
	check_conflict(arg, ArgType::Optarg);
	
	_optlist.push_back(arg);
	return _optlist.back();
}

template <>
ArgumentParser::Argument& ArgumentParser::add_argument<int>(std::string name){
	std::string sname = "", lname = "";
	ArgType type = (name[0] == '-')? ArgType::Optarg : ArgType::Posarg;
	if (type == ArgType::Optarg && name[1] == '-') lname = name;
	else sname = name;

	Argument arg(type, ValType::Int, sname, lname);
	check_conflict(arg, type);
	if (type == ArgType::Optarg){
		_optlist.push_back(arg);
		return _optlist.back();
	}
	else{
		_arglist.push_back(arg);
		return _arglist.back();
	}
}
template <>
ArgumentParser::Argument& ArgumentParser::add_argument<float>(std::string name){
	std::string sname = "", lname = "";
	ArgType type = (name[0] == '-')? ArgType::Optarg : ArgType::Posarg;
	if (type == ArgType::Optarg && name[1] == '-') lname = name;
	else sname = name;

	Argument arg(type, ValType::Float, sname, lname);
	check_conflict(arg, type);
	if (type == ArgType::Optarg){
		_optlist.push_back(arg);
		return _optlist.back();
	}
	else{
		_arglist.push_back(arg);
		return _arglist.back();
	}
}
template <>
ArgumentParser::Argument& ArgumentParser::add_argument<double>(std::string name){
	return add_argument<float>(name);
}
template <>
ArgumentParser::Argument& ArgumentParser::add_argument<bool>(std::string name){
	std::string sname = "", lname = "";
	if (name[0] != '-') error("invalid positional argument type: bool");
	if (name[1] == '-') lname = name;
	else sname = name;

	Argument arg(ArgType::Optarg, ValType::Bool, sname, lname);
	check_conflict(arg, ArgType::Optarg);
	_optlist.push_back(arg);
	return _optlist.back();
}
ArgumentParser::Argument& ArgumentParser::add_argument(std::string name){
	std::string sname = "", lname = "";
	ArgType type = (name[0] == '-')? ArgType::Optarg : ArgType::Posarg;
	if (type == ArgType::Optarg && name[1] == '-') lname = name;
	else sname = name;

	Argument arg(type, ValType::String, sname, lname);
	check_conflict(arg, type);
	if (type == ArgType::Optarg){
		_optlist.push_back(arg);
		return _optlist.back();
	}
	else{
		_arglist.push_back(arg);
		return _arglist.back();
	}
}

ArgumentParser::SubparserList& ArgumentParser::add_subparsers(){
	if (_subparsers.valid()) error("cannot have multiple subparser arguments");
	_subparsers.add(this);
	return _subparsers;
}

// Chain Modifiers {{{2
ArgumentParser& ArgumentParser::prog(std::string prog){
	if (!_override_prog){
		_prog = prog;
		_override_prog = true;
		_usage = "usage: " + _prog;
		_usage_line_len = _usage.size(); 
		_preusage_len = _usage_line_len;
	}
	return *this;
}
ArgumentParser& ArgumentParser::usage(std::string usage){
	if (!_override_usage){
		_usage = "usage: " + usage;
		_override_usage = true;
	}
	return *this;
}

ArgumentParser& ArgumentParser::description(std::string desc){
	if (_desc == "") _desc = desc;
	return *this;
	}
ArgumentParser& ArgumentParser::epilog(std::string epilog){
	if (_epilog == "") _epilog = epilog;
	return *this;
}
ArgumentParser& ArgumentParser::add_help(bool add_help){
	_add_help = add_help;
	return *this;
}
ArgumentParser& ArgumentParser::argument_default(std::string def){
	if (_none_str == NONE) _none_str = def;
	return *this;
}
ArgumentParser& ArgumentParser::parent(const ArgumentParser& parser){
	for (int i=0; i<parser._optlist.size(); i++){
		Argument arg = parser._optlist[i];
		if (arg._valtype == ValType::Int)
			add_argument<int>(arg._sname, arg._lname);
		else if (arg._valtype == ValType::Float)
			add_argument<double>(arg._sname, arg._lname);
		else if (arg._valtype == ValType::String)
			add_argument(arg._sname, arg._lname);
		else if (arg._valtype == ValType::Bool)
			add_argument<bool>(arg._sname, arg._lname);
		_optlist.back() = arg;
	}
	for (int i=0; i<parser._arglist.size(); i++){
		Argument arg = parser._arglist[i];
		if (arg._valtype == ValType::Int)
			add_argument<int>(arg._sname, arg._lname);
		else if (arg._valtype == ValType::Float)
			add_argument<double>(arg._sname, arg._lname);
		else if (arg._valtype == ValType::String)
			add_argument(arg._sname, arg._lname);
		_arglist.back() = arg;
	}
	return *this;
}
ArgumentParser& ArgumentParser::conflict_handler(std::string name){
	for (int i=0; i<name.size(); i++)
		name[i] = tolower(name[i]);
	if (name == "resolve") _resolve = true;
	return *this;
}
ArgumentParser& ArgumentParser::formatter_class(HelpFormatter format){
	_format = format;
	return *this;
}
ArgumentParser& ArgumentParser::fromfile_prefix_chars(std::string prefix){
	if (_fromfile_prefix == "") _fromfile_prefix = prefix;
	return *this;
}
ArgumentParser& ArgumentParser::set_defaults(
		std::map<std::string, std::string> defaults){
	if (_defaults.size() == 0) _defaults = defaults;
	return *this;
}

ArgumentParser& ArgumentParser::help(std::string help){
	if (_subparser && help != "") _subparser_help = trim(help);
	return *this;
}

ArgumentParser& ArgumentParser::set_linecap(int linecap){
	if (linecap > 0) _linecap = linecap;
	return *this;
}

// Accessors {{{2
std::string ArgumentParser::get_prog() const{return _prog;}
ArgumentValueList ArgumentParser::get_default(std::string name) const{
	for (const auto& it : _defaults)
		if (it.first == name)
			return std::vector<std::string>{it.second};
	for (const auto& arg : _optlist)
		if (arg._dest == name)
			return arg._def;
	for (const auto& arg : _arglist)
		if (arg._dest == name)
			return arg._def;
	return std::vector<std::string>{};
}
std::string ArgumentParser::format_usage(){
	load_helpstring();
	return _usage;
}
std::string ArgumentParser::format_help(){
	load_helpstring();
	std::string helpstr;
	helpstr += _usage;
	if (_desc != "") helpstr += "\n\n" + wrap(_desc);
	if (_arghelp != "") helpstr += "\n\n" + _arghelp;
	if (_opthelp != "") helpstr += "\n\n" + _opthelp;
	if (_subhelp != "") helpstr += "\n\n" + _subhelp;
	if (_epilog != "") helpstr += "\n\n" + wrap(_epilog);
	return helpstr;
}
void ArgumentParser::print_usage(std::ostream& out){
	out << format_usage() << std::endl;
}
void ArgumentParser::print_help(std::ostream& out){
	out << format_help() << std::endl;
}

// Parser {{{2
ArgumentMap ArgumentParser::parse_args(std::vector<std::string> argv){
	if (argv.size() != 0){
		_args = argv;
		_nargs = argv.size();
	}
	if (_fromfile_prefix != ""){
		_args = read_args_from_files(_args);
		_nargs = _args.size();
	}
	ArgumentMap args;
	for (const auto& it : _defaults)
		args[it.first] = std::vector<std::string>{it.second};
	
	load_helpstring();

	_parsed = true;
	std::vector<Argument*> flags;
	for (auto& arg : _optlist){
		if (arg._valtype == ValType::Bool){
			if (arg._action == Action::Help)
				_helpargs.push_back(arg);
			flags.push_back(&arg);
		}
	}
	
	int first_collection = -1, first_remainder = -1;
	for (int i=0; i<_arglist.size(); i++){
		if (_arglist[i]._required) _req_posargs++;
		if (first_collection < 0 && (_arglist[i]._more_nargs == '*' || _arglist[i]._more_nargs == '+'))
			first_collection = i;
		if (first_remainder < 0 && _arglist[i]._more_nargs == '.')
			first_remainder = i;
	}
	
	int help_index = -1, print_version = -1;
	bool only_posargs = false;
	std::vector<std::string> posargs;
	std::vector<std::string> others;
	std::string unrecognized = "";
	int shift = 1;
	for (int i=0; i<_nargs; i+=shift){
		shift = 1;
		std::string given = _args[i];
		bool found = false;
		if (given == "--"){
			only_posargs = true;
			continue;
		}
		
		if (given[0] != '-' || only_posargs || (_subparsers.valid() && posargs.size() > _subparser_index))
			posargs.push_back(given);
		else{
			for (int j=0; j<_helpargs.size(); j++){
				char sname = (_helpargs[j]._sname != "")? 
					_helpargs[j]._sname[1] : '\0';
				std::string lname = _helpargs[j]._lname;
				
				if (contains(given, sname, true) || given == lname){
					help_index = i;
				}
			}
			for (int j=0; j<_optlist.size(); j++){
				char sname = (_optlist[j]._sname != "")? 
					_optlist[j]._sname[1] : '\0';
				std::string lname = _optlist[j]._lname;
				
				if (_optlist[j]._action == Action::Version){
					if (contains(given, sname, true) || given == lname)
						print_version = j;
				}
				if (_optlist[j]._action != Action::Store) continue;
				
				// Bool Flags
				if (_optlist[j]._valtype == ValType::Bool) continue;
				// Other Flags
				if (contains(given, sname)){
					// Short form concatenation
					if (_optlist[j]._nargs != 1 && _optlist[j]._nargs != -1)
						error(_optlist[j].error(2));
					std::string temp = given.substr(given.find(sname)+1);
					
					int setresult = _optlist[j].set(temp, _has_digit_opt, false);
					if (setresult == 1)
						error(_optlist[j].error(3, temp));
					
					char *ending;
					if (_optlist[j]._valtype == ValType::Int){
						int value = strtol(temp.c_str(), &ending, 10);
						if (std::string(ending) != "" || temp == "")
							error(_optlist[j].error(1, temp));
					}
					else if (_optlist[j]._valtype == ValType::Float){
						double value = strtod(temp.c_str(), &ending);
						if (std::string(ending) != "" || temp == "")
							error(_optlist[j].error(1, temp));
					}
					
					given = given.substr(0, given.find(sname));
					given += sname;
					if (_optlist[j]._valtype == ValType::String){
						if (help_index == i){
							for (int k=0; k<_helpargs.size(); k++){
								char h_sname = (_helpargs[k]._sname != "")? 
									_helpargs[k]._sname[1] : '\0';
								std::string h_lname = _helpargs[k]._lname;

								if (contains(_optlist[j]._val.back(), h_sname, true) && !contains(given, h_sname, true)){
									help_index = -1;
								}
							}
						}
					}

					found = true;
				}
				else if (find_arg(_optlist[j], given)){
					// Lone argument OR long form concatenation
					if (_optlist[j]._nargs == 1 || _optlist[j]._more_nargs == '?'){
						// One narg argument
						std::string temp = (lname.size()>1 && lname[1] == '-')? given.substr(given.find('=')+1) : given;
						if (given == temp){
							// Lone long form
							if (i+1 >= _nargs){
								if (_optlist[j]._more_nargs == '?'){
									_optlist[j]._val = _optlist[j]._const;
									_optlist[j]._found = true;
								}
								else
									error(_optlist[j].error(2));
							}
							else{
								shift = 2;
								int setresult = _optlist[j]
									.set(_args[i+1], _has_digit_opt);
								if (setresult == 1)
									error(_optlist[j].error(3, _args[i+1]));
								else if (setresult == 2){
									if (_optlist[j]._more_nargs != '?')
										error(_optlist[j].error(2));
									else shift = 1;
								}
								char *ending;
								if (_optlist[j]._valtype == ValType::Int){
									int value = strtol(_args[i+1].c_str(), &ending, 10);
									if (std::string(ending) != "" || _args[i+1] == "")
										error(_optlist[j].error(1, _args[i+1]));
								}
								else if (_optlist[j]._valtype == ValType::Float){
									double value = strtod(_args[i+1].c_str(), &ending);
									if (std::string(ending) != "" || _args[i+1] == "")
										error(_optlist[j].error(1, _args[i+1]));
								}
					
							}
						}
						else{
							int setresult = _optlist[j].set(temp, _has_digit_opt, false);
							if (setresult == 1)
								error(_optlist[j].error(3, temp));
							
							char *ending;
							if (_optlist[j]._valtype == ValType::Int){
								int value = strtol(temp.c_str(), &ending, 10);
								if (std::string(ending) != "" || temp == "")
									error(_optlist[j].error(1, temp));
							}
							else if (_optlist[j]._valtype == ValType::Float){
								double value = strtod(temp.c_str(), &ending);
								if (std::string(ending) != "" || temp == "")
									error(_optlist[j].error(1, temp));
							}
						}
					}
					else{
						// Arguments with >1 nargs
						if (_optlist[j]._more_nargs == '\0'){
							if (i+_optlist[j]._nargs >= _nargs)
								error(_optlist[j].error(2));
							shift = _optlist[j]._nargs + 1;
							for (int k=1; k<=_optlist[j]._nargs; k++){
								int setresult = _optlist[j].set(_args[i+k], _has_digit_opt);
								if (setresult == 1)
									error(_optlist[j].error(3, _args[i+k]));
								else if (setresult == 2)
									error(_optlist[j].error(2));
								
								char *ending;
								if (_optlist[j]._valtype == ValType::Int){
									int value = strtol(_args[i+k].c_str(), &ending, 10);
									if (std::string(ending) != "" || _args[i+k] == "")
										error(_optlist[j].error(1, _args[i+k]));
								}
								else if (_optlist[j]._valtype == ValType::Float){
									double value = strtod(_args[i+k].c_str(), &ending);
									if (std::string(ending) != "" || _args[i+k] == "")
										error(_optlist[j].error(1, _args[i+k]));
								}
							}
						}
						else if (_optlist[j]._more_nargs == '*' || _optlist[j]._more_nargs == '+'){
							std::string temp = (lname.size()>1 && lname[1] == '-')? given.substr(given.find('=')+1) : given;
							if (given == temp){
								int captured = 0;
								for (int k=i+1; k<_nargs; k++){
									int setresult = _optlist[j].set(_args[k], _has_digit_opt);
									if (setresult == 1)
										error(_optlist[j].error(3, _args[k]));
									else if (setresult == 2)
										break;
									captured++;
									
									char *ending;
									if (_optlist[j]._valtype == ValType::Int){
										int value = strtol(_args[k].c_str(), &ending, 10);
										if (std::string(ending) != "" || _args[k] == "")
											error(_optlist[j].error(1, _args[k]));
									}
									else if (_optlist[j]._valtype == ValType::Float){
										double value = strtod(_args[k].c_str(), &ending);
										if (std::string(ending) != "" || _args[k] == "")
											error(_optlist[j].error(1, _args[k]));
									}
								}
								if (captured == 0){
									if (_optlist[j]._more_nargs == '+')
										error(_optlist[j].error(2));
									_optlist[j]._found = true;
								}
								shift = captured + 1;
							}
							else{
								int setresult = _optlist[j].set(temp, _has_digit_opt, false);
								if (setresult == 1)
									error(_optlist[j].error(3, temp));
								
								char *ending;
								if (_optlist[j]._valtype == ValType::Int){
									int value = strtol(temp.c_str(), &ending, 10);
									if (std::string(ending) != "" || temp == "")
										error(_optlist[j].error(1, temp));
								}
								else if (_optlist[j]._valtype == ValType::Float){
									double value = strtod(temp.c_str(), &ending);
									if (std::string(ending) != "" || temp == "")
										error(_optlist[j].error(1, temp));
								}
							}
						}
						else if (_optlist[j]._more_nargs == '.'){
							std::string temp = (lname.size()>1 && lname[1] == '-')? given.substr(given.find('=')+1) : given;
							if (given == temp){
								int captured = 0;
								for (int k=i+1; k<_nargs; k++){
									int setresult = _optlist[j].set(_args[k], _has_digit_opt, false);
									if (setresult == 1)
										error(_optlist[j].error(3, _args[k]));
									captured++;
									
									char *ending;
									if (_optlist[j]._valtype == ValType::Int){
										int value = strtol(_args[k].c_str(), &ending, 10);
										if (std::string(ending) != "" || _args[k] == "")
											error(_optlist[j].error(1, _args[k]));
									}
									else if (_optlist[j]._valtype == ValType::Float){
										double value = strtod(_args[k].c_str(), &ending);
										if (std::string(ending) != "" || _args[k] == "")
											error(_optlist[j].error(1, _args[k]));
									}
								}
								if (captured == 0) _optlist[j]._found = true;
								shift = captured + 1;
							}
							else{
								int setresult = _optlist[j].set(temp, _has_digit_opt, false);
								if (setresult == 1)
									error(_optlist[j].error(3, temp));
								
								char *ending;
								if (_optlist[j]._valtype == ValType::Int){
									int value = strtol(temp.c_str(), &ending, 10);
									if (std::string(ending) != "" || temp == "")
										error(_optlist[j].error(1, temp));
								}
								else if (_optlist[j]._valtype == ValType::Float){
									double value = strtod(temp.c_str(), &ending);
									if (std::string(ending) != "" || temp == "")
										error(_optlist[j].error(1, temp));
								}
							}
						}
					}
					found = true;
				}
			}
			// Boolean Args
			for (int j=0; j<flags.size(); j++){
				char sname = (flags[j]->_sname != "")? flags[j]->_sname[1] : '\0';
				std::string lname = flags[j]->_lname;
				
				if (contains(given, sname, true) || given == lname){
					if (flags[j]->_action == Action::Store){
						if (!flags[j]->_has_default || (flags[j]->_has_default && flags[j]->_def[0] == FALSE))
							flags[j]->set(TRUE, _has_digit_opt);
						else
							flags[j]->set(FALSE, _has_digit_opt);
					}
					else if (flags[j]->_action == Action::StoreConst){
						flags[j]->_val = flags[j]->_const;
						flags[j]->_found = true;
					}
					else if (flags[j]->_action == Action::Count && given == lname){
						flags[j]->_val[0] = std::to_string(strtol(flags[j]->_val[0].c_str(), nullptr, 10)+1);
						flags[j]->_found = true;
					}
					found = true;
				}
			}
			if (!found){
				if (_subparser) others.push_back(given);
				else unrecognized += " " + given;
			}
			if (found && given.size() > 1 && given[1] != '-'){
				for (int j=1; j<given.size(); j++){
					bool found_optarg = false;
					for (int k=0; k<_optlist.size(); k++){
						if (_optlist[k]._sname.size() > 1 && given[j] == _optlist[k]._sname[1]){
							found_optarg = true;
							if (_optlist[k]._action == Action::Count){
								_optlist[k]._val[0] = std::to_string(strtol(_optlist[k]._val[0].c_str(), nullptr, 10)+1);
								_optlist[k]._found = true;
							}
							break;
						}
					}
					if (!found_optarg){
						unrecognized += " " + _args[i];
						break;
					}
				}
			}
		}
	}
	if (help_index>=0){
		print_help();
		exit(0);
	}
	if (print_version >= 0){
		std::cout << _optlist[print_version]._version << std::endl;
		exit(0);
	}

	std::string reqlist = "";
	for (int i=0; i<_optlist.size(); i++){
		bool parser_def = args.find(_optlist[i]._dest) != args.end();
		if (_optlist[i]._action == Action::Version || _optlist[i]._action == Action::Help)
			continue;
		if (!_optlist[i]._found){
			if (_optlist[i]._required){
				if (reqlist == "")
					reqlist += _optlist[i].get_id();
				else
					reqlist += ", " + _optlist[i].get_id();
			}
			else if (_optlist[i]._has_default && !parser_def){
				_optlist[i]._val = _optlist[i]._def;
				_optlist[i]._found = true;
			}
		}
		else if (_optlist[i].size() == 0 && _optlist[i]._more_nargs == '?'){
			_optlist[i]._val = _optlist[i]._const;
			_optlist[i]._found = true;
		}
		
		if (_optlist[i]._found)
			args[_optlist[i]._dest] = _optlist[i]._val;
		else if (_none_str != SUPPRESS && !parser_def)
			args[_optlist[i]._dest] = std::vector<std::string>{_none_str};
	}
	// Positional Arguments
	int nonreq_fillable = posargs.size() - _req_posargs - _base_req_posargs;
	int req_filled = 0;
	int i=0;
	for (int j=0; j<posargs.size(); j++){
		if (i >= _arglist.size()){
			if (_subparser) others.push_back(posargs[j]);
			else{
				unrecognized += " " + posargs[j];
				i++;
			}
			continue;
		}
		if (_arglist[i]._required){
			if (_arglist[i]._more_nargs == '\0'){
				if (_arglist[i]._valtype == ValType::Int){
					char *ending;
					int value = strtol(posargs[j].c_str(), &ending, 10);
					if (std::string(ending) != "" || posargs[j] == "")
						error(_arglist[i].error(1, posargs[j]));
				}
				else if (_arglist[i]._valtype == ValType::Float){
					char *ending;
					double value = strtod(posargs[j].c_str(), &ending);
					if (std::string(ending) != "" || posargs[j] == "")
						error(_arglist[i].error(1, posargs[j]));
				}
				int setresult = _arglist[i].set(posargs[j], _has_digit_opt);
				if (setresult == 1)
					error(_arglist[i].error(3, posargs[j]));
				req_filled++;
				if (_arglist[i]._dest != "")
					args[_arglist[i]._dest] = _arglist[i]._val;
				
				if (_arglist[i]._subparser){
					for (auto& parser : _subparsers._subparsers){
						if (parser._subparser_cmd == _arglist[i]._val[0]){
							parser._base_req_posargs = _req_posargs - req_filled;
							auto leftovers = parser.parse_args(
								std::vector<std::string>(posargs.begin()+j+1, posargs.end())
							);
							for (const auto& it : leftovers)
								if (it.first != SUPPRESS)
									args[it.first] = it.second;
							j = -1;
							posargs.clear();
							for (const auto& str : leftovers[SUPPRESS]){
								if (str[0] == '-')
									unrecognized += " " + str;
								else posargs.push_back(str);
							}
							nonreq_fillable = posargs.size() - _req_posargs - _base_req_posargs - req_filled;
						}
					}
				}
				i++;
			}
			else{//_more_nargs == '+'
				if (_arglist[i].size() == 0){
					if (_arglist[i]._valtype == ValType::Int){
						char *ending;
						int value = strtol(posargs[j].c_str(), &ending, 10);
						if (std::string(ending) != "" || posargs[j] == "")
							error(_arglist[i].error(1, posargs[j]));
					}
					else if (_arglist[i]._valtype == ValType::Float){
						char *ending;
						double value = strtod(posargs[j].c_str(), &ending);
						if (std::string(ending) != "" || posargs[j] == "")
							error(_arglist[i].error(1, posargs[j]));
					}
					int setresult = _arglist[i].set(posargs[j], _has_digit_opt);
					if (setresult == 1)
						error(_arglist[i].error(3, posargs[j]));
					if (_arglist[i]._dest != "") args[_arglist[i]._dest] = _arglist[i]._val;
					req_filled++;
				}
				else if (nonreq_fillable > 0){
					if (_arglist[i]._valtype == ValType::Int){
						char *ending;
						int value = strtol(posargs[j].c_str(), &ending, 10);
						if (std::string(ending) != "" || posargs[j] == "")
							error(_arglist[i].error(1, posargs[j]));
					}
					else if (_arglist[i]._valtype == ValType::Float){
						char *ending;
						double value = strtod(posargs[j].c_str(), &ending);
						if (std::string(ending) != "" || posargs[j] == "")
							error(_arglist[i].error(1, posargs[j]));
					}
					int setresult = _arglist[i].set(posargs[j], _has_digit_opt);
					if (setresult == 1)
						error(_arglist[i].error(3, posargs[j]));
					if (_arglist[i]._dest != "") args[_arglist[i]._dest] = _arglist[i]._val;
					nonreq_fillable--;
				}
				else{
					i++;
					j--;
				}
			}
		}
		else{
			if (first_collection >= 0){
				if (i > first_collection){
					if (_none_str != SUPPRESS && _arglist[i]._dest != "")
						args[_arglist[i]._dest] = std::vector<std::string>{_none_str};
					i++;
					j--;
				}
				else if (i == first_collection){//_more_nargs == '*'
					if (nonreq_fillable > 0){
						if (_arglist[i]._valtype == ValType::Int){
							char *ending;
							int value = strtol(posargs[j].c_str(), &ending, 10);
							if (std::string(ending) != "" || posargs[j] == "")
								error(_arglist[i].error(1, posargs[j]));
						}
						else if (_arglist[i]._valtype == ValType::Float){
							char *ending;
							double value = strtod(posargs[j].c_str(), &ending);
							if (std::string(ending) != "" || posargs[j] == "")
								error(_arglist[i].error(1, posargs[j]));
						}
						int setresult = _arglist[i].set(posargs[j], _has_digit_opt);
						if (setresult == 1)
							error(_arglist[i].error(3, posargs[j]));
						if (_arglist[i]._dest != "") args[_arglist[i]._dest] = _arglist[i]._val;
						nonreq_fillable--;
					}
					else{
						if (_arglist[i].size() == 0 && _none_str != SUPPRESS)
							args[_arglist[i]._dest] = std::vector<std::string>{_none_str};
						i++;
						j--;
					}
				}
				else if (nonreq_fillable > 0){
					if (_arglist[i]._valtype == ValType::Int){
						char *ending;
						int value = strtol(posargs[j].c_str(), &ending, 10);
						if (std::string(ending) != "" || posargs[j] == "")
							error(_arglist[i].error(1, posargs[j]));
					}
					else if (_arglist[i]._valtype == ValType::Float){
						char *ending;
						double value = strtod(posargs[j].c_str(), &ending);
						if (std::string(ending) != "" || posargs[j] == "")
							error(_arglist[i].error(1, posargs[j]));
					}
					int setresult = _arglist[i].set(posargs[j], _has_digit_opt);
					if (setresult == 1)
						error(_arglist[i].error(3, posargs[j]));
					if (_arglist[i]._dest != "") 
						args[_arglist[i]._dest] = _arglist[i]._val;
					nonreq_fillable--;
					
					if (_arglist[i]._subparser){
						for (auto& parser : _subparsers._subparsers){
							if (parser._subparser_cmd == _arglist[i]._val[0]){
								parser._args = std::vector<std::string>(posargs.begin()+j+1, posargs.end());
								parser._nargs = parser._args.size();
								parser._base_req_posargs = _req_posargs - req_filled;
								auto leftovers = parser.parse_args();
								for (const auto& it : leftovers)
									if (it.first != SUPPRESS)
										args[it.first] = it.second;
								j = -1;
								posargs.clear();
								for (const auto& str : leftovers[SUPPRESS]){
									if (str[0] == '-')
										unrecognized += " " + str;
									else posargs.push_back(str);
								}
								nonreq_fillable = posargs.size() - _req_posargs - _base_req_posargs - req_filled;
							}
						}
					}
					i++;
				}
				else{
					if (_none_str != SUPPRESS && _arglist[i]._dest != "")
						args[_arglist[i]._dest] = std::vector<std::string>{_none_str};
					i++;
					j--;
				}
			}
			else{
				if (nonreq_fillable > 0){
					if (_arglist[i]._valtype == ValType::Int){
						char *ending;
						int value = strtol(posargs[j].c_str(), &ending, 10);
						if (std::string(ending) != "" || posargs[j] == "")
							error(_arglist[i].error(1, posargs[j]));
					}
					else if (_arglist[i]._valtype == ValType::Float){
						char *ending;
						double value = strtod(posargs[j].c_str(), &ending);
						if (std::string(ending) != "" || posargs[j] == "")
							error(_arglist[i].error(1, posargs[j]));
					}
					int setresult = _arglist[i].set(posargs[j], _has_digit_opt);
					if (setresult == 1)
						error(_arglist[i].error(3, posargs[j]));
					if (_arglist[i]._dest != "") args[_arglist[i]._dest] = _arglist[i]._val;
					nonreq_fillable--;
					
					if (_arglist[i]._subparser){
						for (auto& parser : _subparsers._subparsers){
							if (parser._subparser_cmd == _arglist[i]._val[0]){
								parser._args = std::vector<std::string>(posargs.begin()+j+1, posargs.end());
								parser._nargs = parser._args.size();
								parser._base_req_posargs = _req_posargs - req_filled;
								auto leftovers = parser.parse_args();
								for (const auto& it : leftovers)
									if (it.first != SUPPRESS)
										args[it.first] = it.second;
								j = -1;
								posargs.clear();
								for (const auto& str : leftovers[SUPPRESS]){
									if (str[0] == '-')
										unrecognized += " " + str;
									else posargs.push_back(str);
								}
								nonreq_fillable = posargs.size() - _req_posargs - _base_req_posargs - req_filled;
							}
						}
					}
					i++;
				}
				else{
					if (_none_str != SUPPRESS && _arglist[i]._dest != "")
						args[_arglist[i]._dest] = std::vector<std::string>{_none_str};
					i++;
					j--;
				}
			}
		}
	}
	while (i < _arglist.size()){
		if (!_arglist[i]._found && _arglist[i]._required)
			reqlist += (reqlist == ""? "" : ", ") + _arglist[i]._metavar[0];
		i++;
	}
	if (_subparser) args[SUPPRESS] = others;
	// Final Error Checking
	if (reqlist != "") error("the following arguments are required: " + reqlist);
	if (unrecognized != "")
		error("unrecognized arguments:" + unrecognized);

	return args;
}

// Private Helpers {{{2
void ArgumentParser::check_conflict(const Argument& arg, ArgType type){
	std::string sname = arg._sname, lname = arg._lname;
	if (type == ArgType::Optarg){
		if (sname == "" && lname == "")
			error("option string(s) must be provided");
		std::string nodash = "";
		if (sname[0] != '-'){
			nodash = sname;
			if (lname[0] != '-') nodash += ", ";
		}
		if (lname[0] != '-') nodash += lname;
		if (nodash != "")
			error("invalid option std::string '" + nodash + "': "
					+ "must start with a character '-'");
		if (sname != ""){
			if (sname.size() == 1 || (sname.size() == 2 && !isalnum(sname[1]))) 
				error("invalid option std::string '" + sname + "': "
						+ "must end with an alphanumeric character");
			if (sname.size() > 2)
				error("invalid option std::string '" + sname + "': "
						+ "must be a character '-' and "
						+ " one alphanumeric character");
			if (isdigit(sname[1])) _has_digit_opt = true;
		}
		if (lname != ""){
			if (lname.size() == 1)
				error("invalid option std::string '" + lname + "': "
						+ "must start with two prefix characters '-'");
			else if (lname.size() == 2)
				error("invalid option std::string '" + lname + "': "
						+ "must end with at least one alphanumeric character");
		}

		std::string conflicts = "";
		for (int i=0; i<_optlist.size(); i++){
			if (sname != "" && sname == _optlist[i]._sname){
				conflicts += (conflicts == ""? "" : ", ") + sname;
				if (_resolve) _optlist[i]._sname = "";
			}
			if (lname != "" && lname == _optlist[i]._lname){
				conflicts += (conflicts == ""? "" : ", ") + lname;
				if (_resolve) _optlist[i]._lname = "";
			}
			if (_optlist[i]._sname == "" && _optlist[i]._lname == ""){
				_optlist.erase(_optlist.begin()+i);
				i--;
			}
		}
		if (!_resolve && conflicts != ""){
			conflicts = "argument " + arg.get_id() 
				+ ": conflicting option strings: " + conflicts;
			error(conflicts);
		}
	}
}

inline bool ArgumentParser::valid_value(std::string val) const{
	return !(val[0] == '-' && val.size() > 1 && (!isdigit(val[1]) || (isdigit(val[1]) && _has_digit_opt)));
}

void ArgumentParser::load_helpstring(){
	_opthelp = "";
	_arghelp = "";
	_subhelp = "";
	_usage = "usage: " + _prog;
	_usage_line_len = _usage.size(); 
	_preusage_len = _usage_line_len;
	
	if (_add_help && !_added_help){
		Argument arg(ArgType::Optarg, ValType::Bool, "-h", "--help");
		arg.help("show this help message and exit");
		arg._action = Action::Help;
		check_conflict(arg, ArgType::Optarg);
		_helpargs.push_back(arg);
		_optlist.insert(_optlist.begin(), arg);
		_added_help = true;
	}

	for (const auto& arg : _optlist){
		int temp = arg.get_help_id(_format).size();
		if (temp+2 > _help_indent) _help_indent 
			= (temp+2>HELP_INDENT_MAX)? HELP_INDENT_MAX : temp+2;
	}
	for (const auto& arg : _arglist){
		int temp = arg.get_help_id(_format).size();
		if (temp+2 > _help_indent) _help_indent 
			= (temp+2>HELP_INDENT_MAX)? HELP_INDENT_MAX : temp+2;
		if (arg._subparser){
			for (const auto& parser : _subparsers._subparsers){
				temp = std::string("    " + parser._subparser_cmd).size();
				if (temp+2 > _help_indent) _help_indent
					= (temp+2>HELP_INDENT_MAX)? HELP_INDENT_MAX : temp+2;
			}
		}
	}
	
	if (_optlist.size() != 0) _opthelp += "options:";
	for (auto& arg : _optlist){
		if (arg._help == SUPPRESS) continue;
		_opthelp += "\n" + arg.get_help(_linecap, _help_indent, _format);
		if (_override_usage) continue;
		std::string usage = arg.get_usage(_format);
		if (_usage_line_len + usage.size() + 1 <= _linecap){
			_usage += " " + usage;
			_usage_line_len += usage.size() + 1;
		}
		else{
			_usage += '\n';
			_multiline_usage = true;
			for (int i=0; i<_preusage_len; i++) _usage += ' ';
			_usage += " " + usage;
			_usage_line_len = _preusage_len + usage.size() + 1;
		}
	}

	bool get_sub_prog = _subparsers.valid();
	std::string sub_prog = "";
	
	if (_arglist.size()-(_subparsers.valid() && _subparsers._title != ""? 1:0)!= 0)
		_arghelp += "positional arguments:";
	for (int i=0; i<_arglist.size(); i++){
		if (_arglist[i]._help == SUPPRESS) continue;

		if (!_arglist[i]._subparser || (_arglist[i]._subparser && _subparsers._title == "")) 
			_arghelp += "\n" + _arglist[i].get_help(_linecap, _help_indent, _format);
		if (_arglist[i]._subparser && _subparsers._title == ""){
			for (const auto& parser : _subparsers._subparsers){
				if (parser._subparser_help == "") continue;
				_arghelp += "\n" + parser.get_subparser_help(_linecap, _help_indent, _format);
			}
		}
		if (_override_usage) continue;
		std::string usage = _arglist[i].get_usage(_format);
		if (_multiline_usage){
			_multiline_usage = false;
			_usage += '\n';
			for (int i=0; i<_preusage_len; i++) _usage += ' ';
			_usage += " " + usage;
			_usage_line_len = _preusage_len + usage.size() + 1;
		}
		else if (_usage_line_len + usage.size() + 1 <= _linecap){
			_usage += " " + usage;
			_usage_line_len += usage.size() + 1;
		}
		else{
			_usage += '\n';
			for (int i=0; i<_preusage_len; i++) _usage += ' ';
			_usage += " " + usage;
			_usage_line_len = _preusage_len + usage.size() + 1;
		}
		
		if (_arglist[i]._subparser)
			get_sub_prog = false;
		else if (get_sub_prog){
			sub_prog += " " + usage;
		}
	}
	
	if (_subparsers.valid()){
		for (auto& parser : _subparsers._subparsers){
			if (_subparsers._prog != "")
				parser.prog(_subparsers._prog + " " + parser._subparser_cmd);
			else parser.prog(_prog + sub_prog + " " + parser._subparser_cmd);
		}
	}

	if (_subparsers.valid() && _subparsers._title != ""){
		_subhelp += _subparsers._title + ":";
		if (_subparsers._desc != "")
			_subhelp += "\n" + wrap(_subparsers._desc, 1) + "\n";
		_subhelp += "\n" + _arglist[_subparser_index].get_help(_linecap, _help_indent, _format);
		for (const auto& parser : _subparsers._subparsers){
			if (parser._subparser_help == "") continue;
			_subhelp += "\n" + parser.get_subparser_help(_linecap, _help_indent, _format);
		}
	}
}

std::vector<std::string> ArgumentParser::read_args_from_files(std::vector<std::string> args){
	std::vector<std::string> result;
	for (const auto& str : args){
		bool found = false;
		for (const auto& prefix : _fromfile_prefix){
			if (str[0] == prefix){
				found = true;
				break;
			}
		}
		if (!found) result.push_back(str);
		else{
			std::ifstream infile(str.substr(1));
			if (infile.is_open()){
				std::vector<std::string> temp;
				std::string line;
				while (std::getline(infile, line))
					temp.push_back(line);
				temp = read_args_from_files(temp);
				result.insert(result.end(), temp.begin(), temp.end());
			}
			else error("error opening file " + str.substr(1));
		}
	}
	return result;
}

bool ArgumentParser::find_arg(const Argument& arg, std::string given) const{
	bool equals = (given.find('=') != std::string::npos);
	bool cond1 = given.size() > 1 && given[1] != '-' 
		&& given[given.size()-1] == arg._sname[1];
	bool cond2 = arg._lname != "";
	bool cond3a = (given.find(arg._lname) == 0 && (given.find('=') == arg._lname.size()));
	bool cond3b = (given == arg._lname);

	// True if (1) does not begin with '--' and ends with sname[0] OR (2) begins with long form
	return cond1 || (cond2 && (cond3a || cond3b));
}

bool ArgumentParser::contains(std::string given, char sname, bool flag) const{
	bool found = (given.find(sname) != std::string::npos);
	bool cond1 = (given.size() > 1 && given[1] != '-');
	bool cond2 = given.find(sname) != given.size()-1 || flag;
	
	// True if (1) sname is within the given string, (2) the given string does not begin with '--',
	// and (3) sname is not the last character (unless it is a flag)
	return (sname != '\0' && found && cond1 && cond2);
}

std::string ArgumentParser::get_subparser_help(int linecap, int indent, HelpFormatter format) const{
	if (!_subparser) return "";
	std::string helpstr = "    " + _subparser_cmd;

	if (_subparser_help != ""){
		std::string help = _subparser_help;
	
		bool rawtext = (format == HelpFormatter::RawText);
		int line_len = helpstr.size();
		if (line_len < indent - 1){
			for (int i=0; i<indent-line_len-1; i++) helpstr += ' ';
			line_len = helpstr.size();
		}
		else{
			helpstr += '\n';
			for (int i=0; i<indent-1; i++) helpstr += ' ';
			line_len = indent-1;
		}
		
		if (!rawtext){
			std::string line = "", word = "";
			for (int i=0; i<help.size(); i++){
				if (!isspace(help[i])) word += help[i];
				else if (word != ""){
					if (line_len + word.size() + 1 <= linecap){
						helpstr += " " + word;
						line_len += word.size() + 1;
						word = "";
					}
					else{
						for (int i=0; i<indent-1; i++) line += ' ';
						line += " " + word;
						line_len = line.size();
						helpstr += "\n" + line;
						word = line = "";
					}
				}
			}
			if (word != ""){
				if (line_len + word.size() + 1 <= linecap){
					helpstr += " " + word;
					line_len += word.size() + 1;
					word = "";
				}
				else{
					for (int i=0; i<indent-1; i++) line += ' ';
					line += " " + word;
					line_len = line.size();
					helpstr += "\n" + line;
					word = line = "";
				}
			}
		}
		else{
			helpstr += " ";
			for (int i=0; i<help.size(); i++){
				helpstr += help[i];
				if (help[i] == '\n')
					for (int i=0; i<indent; i++) 
						helpstr += ' ';
			}
		}
	}

	return helpstr;
}

std::string ArgumentParser::trim(std::string str, char c) const{
	for (int i=0; i<str.size(); i++){
		if (str[i] != c){
			str = str.substr(i);
			break;
		}
	}
	for (int i=0; i<str.size(); i++){
		if (str[str.size()-i-1] != c){
			str = str.substr(0,str.size()-i);
			break;
		}
	}
	return str;
}

std::string ArgumentParser::wrap(std::string input, int indent_level) const{
	std::string str = "";
	if (input == "") return str;
	bool rawtext = (_format == HelpFormatter::RawDescription 
			|| _format == HelpFormatter::RawText);
	
	if (!rawtext){
		std::string line = "", word = "";
		for (int i=0; i<input.size(); i++){
			if (!isspace(input[i])) word += input[i];
			else if (word != ""){
				if (line.size() + word.size() + 1 <= _linecap){
					line += ((line == "")? "":" ") + word;
					word = "";
				}
				else{
					str += ((str == "")? "":"\n");
					for (int i=0; i<indent_level; i++) str += "  ";
					str += line;
					line = word;
					word = "";
				}
			}
		}
		if (word != ""){
			if (line.size() + word.size() + 1 <= _linecap){
				line += ((line == "")? "":" ") + word;
			}
			else{
				str += ((str == "")? "":"\n");
				for (int i=0; i<indent_level; i++) str += "  ";
				str += line;
				line = word;
			}
			str += ((str == "")? "":"\n");
			for (int i=0; i<indent_level; i++) str += "  ";
			str += line;
		}
	}
	else{
		for (int i=0; i<indent_level; i++) str += "  ";
		for (int i=0; i<input.size(); i++){
			if (input[i] == '\n' && str.size() > 2)
				if (str[str.size()-2] == '\n' && str[str.size()-1] == '\n') 
					continue;
			str += input[i];
		}
	}
	
	return trim(str);
}

// === ARGUMENTS === {{{1
//
// Constructors {{{2
ArgumentParser::Argument::Argument(){
	_type = ArgType::Optarg;
	_valtype = ValType::Bool;
	_action = Action::Store;
	_sname = _lname = _dest = "";
	_help = "";
	_nargs = 0;
	_more_nargs = '\0';
	_found = _required = _subparser = false;
	_has_default = _override_metavar = false; 
}

ArgumentParser::Argument::Argument(ArgType type, ValType valtype, 
		std::string sname, std::string lname) : _type(type), 
		_valtype(valtype), _sname(sname), _lname(lname), _nargs(1), 
		_required(false), _more_nargs('\0'), _has_default(false), 
		_override_metavar(false), _found(false), _subparser(false){
	std::string metavar = "";
	if (type == ArgType::Optarg){
		if (lname != ""){
			for (int i=2; i<lname.size(); i++){
				metavar += (lname[i] == '-')? '_' : toupper(lname[i]);
				_dest += (lname[i] == '-')? '_' : lname[i];
			}
		}
		else{
			metavar = toupper(sname[1]);
			_dest = sname[1];
		}
		if (valtype == ValType::Bool){
			_nargs = 0;
			def(FALSE);
		}
	}
	else{
		metavar = sname;
		_dest = sname;
		_required = true;
	}
	_metavar.push_back(metavar);
	_action = Action::Store;
}

// Error Handling {{{2
std::string ArgumentParser::Argument::error(std::string msg) const{
	std::string result = "argument " + get_id() + ": ";
	if (msg == "") result += "unrecognized error";
	else result += msg;
	return result;
}
std::string ArgumentParser::Argument::error(int code, std::string msg) const{
	std::string result = "argument " + get_id() + ": ";
	if (code == 1){//invalid type given
		std::string type = "";
		if (_valtype == ValType::Int) type = "int ";
		else if (_valtype == ValType::Float) type = "float ";
		result += "invalid " + type + "value";
		result += ": '" + msg + "'";
	}
	else if (code == 2){//expected (nargs) arguments
		result += "expected ";
		if (_nargs == 1) result += "one argument";
		else if (_more_nargs == '+')
			result += "at least one argument";
		else result += std::to_string(_nargs) + " arguments";
	}
	else if (code == 3){//invalid choice
		result += "invalid choice: '" + msg 
			+ "' (choose from '" + _choices[0];
		for (int i=1; i<_choices.size(); i++)
			result += "', '" + _choices[i];
		result += "')";
	}
	else result += "unrecognized error";
	return result;
}

// Chain Modifiers {{{2
ArgumentParser::Argument& ArgumentParser::Argument::nargs(int nargs){
	if (_valtype != ValType::Bool && nargs > 0)
		_nargs = nargs;
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::nargs(char nargs){
	if (nargs != '?' && nargs != '+' && nargs != '*')
		return *this;
	if (_valtype != ValType::Bool){
		_nargs = -1;
		_more_nargs = nargs;
	}
	if (nargs != '+' && _type == ArgType::Posarg)
		_required = false;
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::nargs(std::string nargs){
	if (nargs != "?" && nargs != "*" && nargs != "+" && nargs != "...")
		return *this;
	if (_valtype != ValType::Bool){
		_nargs = -1;
		_more_nargs = nargs[0];
	}
	if (nargs != "+" && _type == ArgType::Posarg)
		_required = false;
	
	//TODO Remove once work begins on REMAINDER parsing
	if (_more_nargs == '.') _more_nargs = '*';
	
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::choices(std::string choices, char delim){
	if (_valtype == ValType::Bool) return *this;
	std::vector<std::string> result;
	std::string temp="";
	std::string final_choices="";
	
	for (int i=0; i<choices.size(); i++){
		if (choices[i] != delim) temp += choices[i];
		else{
			if (_valtype == ValType::Int){
				char *ending;
				int value = strtol(temp.c_str(), &ending, 10);
				if (std::string(ending) != "") continue;
			}
			else if (_valtype == ValType::Float){
				char *ending;
				double value = strtod(temp.c_str(), &ending);
				if (std::string(ending) != "") continue;
			}
			result.push_back(temp);
			final_choices += (final_choices==""? "":",") + temp;
			temp = "";
		}
	}
	if (_valtype == ValType::Int){
		char *ending;
		int value = strtol(temp.c_str(), &ending, 10);
		if (std::string(ending) == ""){
			result.push_back(temp);
			final_choices += (final_choices==""? "":",") + temp;
		}
	}
	else if (_valtype == ValType::Float){
		char *ending;
		double value = strtod(temp.c_str(), &ending);
		if (std::string(ending) == ""){
			result.push_back(temp);
			final_choices += (final_choices==""? "":",") + temp;
		}
	}
	else{
		result.push_back(temp);
		final_choices += (final_choices==""? "":",") + temp;
	}
	
	_choices = result;
	
	if (!_override_metavar)
		_metavar[0] = "{" + final_choices + "}";

	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::required(bool required){
	if (_type == ArgType::Optarg){
		_required = required;
	}
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::dest(std::string dest){
	if (dest == "") return *this;
	if (!_subparser && _type == ArgType::Posarg) return *this;
	_dest = dest;
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::help(std::string help){
	_help = trim(help);
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::metavar(std::string metavar, char delim){
	if (_override_metavar) return *this;
	if (_more_nargs == '.') return *this;
	_metavar.clear();
	if (_nargs == 1 || _nargs == -1){
		_metavar.push_back(metavar);
	}
	else{
		std::vector<std::string> result;
		std::string temp;
		for (int i=0; i<metavar.size(); i++){
			if (metavar[i] != delim) temp += metavar[i];
			else{
				result.push_back(temp);
				temp = "";
			}
		}
		result.push_back(temp);
		if (result.size() == 1 || result.size() == _nargs)
			_metavar = result;
	}
	_override_metavar = true;
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::def(std::string def, char delim){
	if (!_required || _valtype == ValType::Bool){
		std::vector<std::string> result;	
		std::string temp;
		for (int i=0; i<def.size(); i++){
			if (def[i] != delim) temp += def[i];
			else{
				result.push_back(temp);
				temp = "";
			}
		}
		result.push_back(temp);
		
		if (_valtype == ValType::Bool && _action == Action::Store){
			if (result.size() == 1){
				result[0][0] = toupper(result[0][0]);
				for (int i=1; i<result[0].size(); i++) 
					result[0][i] = tolower(result[0][i]);
				if (result[0] == "1") result[0] = TRUE;
				else if (result[0] == "0") result[0] = FALSE;
				else if (result[0] != TRUE && result[0] != FALSE)
					result[0] = FALSE;
			}
			else return *this;
		}
		if (_valtype == ValType::Bool && _action == Action::Count){
			if (result.size() == 1){
				char *ending;
				int temp = strtol(result[0].c_str(), &ending, 10);
				if ((std::string)ending == ""){
					_val = result;
					_def = result;
					_has_default = true;
				}
			}
		}
		else{
			_def = result;
			_has_default = true;
		}
	}
	return *this;
}
//TODO figure out what cases this should/shouldn't run
ArgumentParser::Argument& ArgumentParser::Argument::constant(std::string con, char delim){
	if (_type == ArgType::Optarg){
		if (_valtype == ValType::Bool) _action = Action::StoreConst;
		std::vector<std::string> result;
		std::string temp;
		for (int i=0; i<con.size(); i++){
			if (con[i] != delim) temp += con[i];
			else{
				result.push_back(temp);
				temp = "";
			}
		}
		result.push_back(temp);
		_const = result;
	}
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::version(std::string version){
	if (version == "" || _valtype != ValType::Bool) return *this;
	_action = Action::Version;
	_version = version;
	if (_help == "") _help = "show program's version number and exit";
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::print_help(){
	if (_action != Action::Store || _valtype != ValType::Bool)
		return *this;
	_action = Action::Help;
	return *this;
}
ArgumentParser::Argument& ArgumentParser::Argument::count(){
	if (_valtype == ValType::Bool && _action == Action::Store){
		_action = Action::Count;
		_val.clear();
		_val.push_back("0");
		_def = _val;
		_has_default = true;
	}
	return *this;
}

// Modifiers {{{2
int ArgumentParser::Argument::set(std::string value, bool has_digit_opt, 
		bool careful){
	// Exit Codes:
	// 	0: Success
	// 	1: Not a valid choice
	// 	2: Encountered optarg (no values given)
	_found = true;
	bool valid_choice = true;
	if (_choices.size() != 0){
		valid_choice = false;
		for (int i=0; i<_choices.size(); i++){
			if (value == _choices[i]){
				valid_choice = true;
				break;
			}
		}
	}
	if (careful){
		if (value[0] == '-' && value.size() > 1){
			if (isdigit(value[1]) && !has_digit_opt){
				if (!valid_choice) return 1;
				if (_more_nargs == '\0' && _val.size() >= _nargs) _val.clear();
				_val.push_back(value);
				return 0;
			}
			else return 2;
		}
	}
	if (!valid_choice) return 1;
	if (_more_nargs == '\0' && _val.size() >= _nargs) _val.clear();
	_val.push_back(value);
	return 0;
}

void ArgumentParser::Argument::add_choice(std::string choice){
	_choices.push_back(choice);
	if (!_override_metavar){
		std::string metavar = "{" + _choices[0];
		for (int i=1; i<_choices.size(); i++)
			metavar += "," + _choices[i];
		metavar += "}";
		if (_metavar.size() == 0)
			_metavar.push_back(metavar);
		else _metavar[0] = metavar;
	}
}

// Accessors {{{2
const std::string& ArgumentParser::Argument::operator[](int index) const{
	return _val[index];
}
int ArgumentParser::Argument::size() const{return _val.size();}

std::string ArgumentParser::Argument::get_id() const{
	std::string id;
	if (_type == ArgType::Optarg)
		id = _sname + (_sname != "" && _lname != ""? "/" : "") + _lname;
	else
		id = _metavar[0];
	return id;
}

std::string ArgumentParser::Argument::get_help_id(HelpFormatter format) const{
	std::string helpstr = "  ";
	
	std::string metavar = "";
	bool metavartype = (format == HelpFormatter::MetavarType);
	if (metavartype){
		switch (_valtype){
			case ValType::Int:
				metavar = "int";
				break;
			case ValType::Float:
				metavar = "float";
				break;
			case ValType::String:
				metavar = "str";
				break;
			case ValType::Bool:
				metavar = "bool";
				break;
		}
	}
	else metavar = _metavar[0];

	if (_sname != ""){
		if (_type == ArgType::Optarg){
			helpstr += _sname;
			if (!metavartype && _metavar.size() == _nargs)
				for (int i=0; i<_nargs; i++) helpstr += " " + _metavar[i];
			else if (_nargs != -1)
				for (int i=0; i<_nargs; i++) helpstr += " " + metavar;
			else if (_more_nargs == '?')
				helpstr += " [" + metavar + "]";
			else if (_more_nargs == '*')
				helpstr += " [" + metavar + " ...]";
			else if (_more_nargs == '+')
				helpstr += " " + metavar 
					+ " [" + metavar + " ...]";
			else if (_more_nargs == '.')
				helpstr += "...";
		}
		else if (_more_nargs == '.') helpstr += "...";
		else helpstr += metavar;
		if (_lname != "") helpstr += ", ";
	}
	if (_lname != ""){
		helpstr += _lname;
		if (!metavartype && _metavar.size() == _nargs)
			for (int i=0; i<_nargs; i++) helpstr += " " + _metavar[i];
		else if (_nargs != -1)
			for (int i=0; i<_nargs; i++) helpstr += " " + metavar;
		else if (_more_nargs == '?')
			helpstr += " [" + metavar + "]";
		else if (_more_nargs == '*')
			helpstr += " [" + metavar + " ...]";
		else if (_more_nargs == '+')
			helpstr += " " + metavar 
				+ " [" + metavar + " ...]";
		else if (_more_nargs == '.')
			helpstr += "...";
	}
	return helpstr;
}

std::string ArgumentParser::Argument::get_usage(HelpFormatter format) const{
	std::string usagestr = "";

	std::string metavar = "";
	bool metavartype = (format == HelpFormatter::MetavarType);
	if (metavartype){
		switch (_valtype){
			case ValType::Int:
				metavar = "int";
				break;
			case ValType::Float:
				metavar = "float";
				break;
			case ValType::String:
				metavar = "str";
				break;
			case ValType::Bool:
				metavar = "bool";
				break;
		}
	}
	else metavar = _metavar[0];

	if (_type == ArgType::Optarg){
		if (_sname != "") usagestr += _sname;
		else usagestr += _lname;
		if (!metavartype && _metavar.size() == _nargs)
			for (int i=0; i<_nargs; i++) usagestr += " " + _metavar[i];
		else for (int i=0; i<_nargs; i++) usagestr += " " + metavar;
		if (_more_nargs == '?') usagestr += " [" + metavar + "]";
		else if (_more_nargs == '*') usagestr += " [" + metavar + " ...]"; 
		else if (_more_nargs == '+') 
			usagestr += " " + metavar + " [" + metavar + " ...]";
		else if (_more_nargs == '.') usagestr += "...";
	}
	else{
		usagestr += metavar;
		if (_more_nargs == '*' || _subparser) usagestr += " ...";
		else if (_more_nargs == '+') 
			usagestr += " [" + metavar + " ...]";
		else if (_more_nargs == '.') usagestr += "...";
	}
	
	if (!_required && !_subparser) usagestr = "[" + usagestr + "]";
	return usagestr;
}

std::string ArgumentParser::Argument::get_help(int linecap, int indent, 
		HelpFormatter format) const{
	std::string helpstr = get_help_id(format);

	if (_help != ""){
		std::string help = _help;
		if (help != "" && !_required && format == HelpFormatter::ArgumentDefaults){
			if (!_subparser && (_action == Action::Store || _action == Action::StoreConst || _action == Action::Count)){
				if (!isspace(help[help.size()-1])) help += " ";
				help += "(default: ";
				if (_has_default){
					if (_def.size() > 1) help += "[";
					help += _def[0];
					for (int i=1; i<_def.size(); i++){
						if (_valtype == ValType::String)
							help += ", '" + _def[i] + "'";
						else
							help += ", " + _def[i];
					}
					if (_def.size() > 1) help += "]";
					help += ")";
				}
				else if (!_required) help += "None)";
			}
		}
	
		bool rawtext = (format == HelpFormatter::RawText);
		int line_len = helpstr.size();
		if (line_len < indent - 1){
			for (int i=0; i<indent-line_len-1; i++) helpstr += ' ';
			line_len = helpstr.size();
		}
		else{
			helpstr += '\n';
			for (int i=0; i<indent-1; i++) helpstr += ' ';
			line_len = indent-1;
		}
		
		if (!rawtext){
			std::string line = "", word = "";
			for (int i=0; i<help.size(); i++){
				if (!isspace(help[i])) word += help[i];
				else if (word != ""){
					if (line_len + word.size() + 1 <= linecap){
						helpstr += " " + word;
						line_len += word.size() + 1;
						word = "";
					}
					else{
						for (int i=0; i<indent-1; i++) line += ' ';
						line += " " + word;
						line_len = line.size();
						helpstr += "\n" + line;
						word = line = "";
					}
				}
			}
			if (word != ""){
				if (line_len + word.size() + 1 <= linecap){
					helpstr += " " + word;
					line_len += word.size() + 1;
					word = "";
				}
				else{
					for (int i=0; i<indent-1; i++) line += ' ';
					line += " " + word;
					line_len = line.size();
					helpstr += "\n" + line;
					word = line = "";
				}
			}
		}
		else{
			helpstr += " ";
			for (int i=0; i<help.size(); i++){
				helpstr += help[i];
				if (help[i] == '\n')
					for (int i=0; i<indent; i++) 
						helpstr += ' ';
			}
		}
	}

	return helpstr;
}

// Helpers {{{2
std::string ArgumentParser::Argument::trim(std::string str) const{
	for (int i=0; i<str.size(); i++){
		if (!isspace(str[i])){
			str = str.substr(i);
			break;
		}
	}
	for (int i=0; i<str.size(); i++){
		if (!isspace(str[str.size()-i-1])){
			str = str.substr(0,str.size()-i);
			break;
		}
	}

	return str;
}

// === SUBPARSERS === {{{1
//
// Constructor {{{2
ArgumentParser::SubparserList::SubparserList(){
	_arg = -1;
	_parser = nullptr;
	_override_title = false;
}
// Chain Modifiers {{{2
ArgumentParser::SubparserList& ArgumentParser::SubparserList::title(std::string title){
	if (title == "") return *this;
	if (!_override_title) _title = title;
	_override_title = true;
	return *this;
}
ArgumentParser::SubparserList& ArgumentParser::SubparserList::description(std::string desc){
	if (desc == "" || _desc != "") return *this;
	if (_title == "") _title = "subcommands";
	_desc = desc;
	return *this;
}
ArgumentParser::SubparserList& ArgumentParser::SubparserList::prog(std::string prog){
	if (prog == "" || _prog != "") return *this;
	_prog = prog;
	return *this;
}
ArgumentParser::SubparserList& ArgumentParser::SubparserList::dest(std::string dest){
	if (_parser != nullptr) _parser->_arglist[_arg].dest(dest);
	return *this;
}
ArgumentParser::SubparserList& ArgumentParser::SubparserList::help(std::string help){
	if (_parser != nullptr) _parser->_arglist[_arg].help(help);
	return *this;
}
ArgumentParser::SubparserList& ArgumentParser::SubparserList::metavar(std::string metavar){
	if (_parser != nullptr) _parser->_arglist[_arg].metavar(metavar);
	return *this;
}
ArgumentParser::SubparserList& ArgumentParser::SubparserList::required(bool required){
	if (_parser != nullptr) _parser->_arglist[_arg].required(required);
	return *this;
}

// Modifiers {{{2
ArgumentParser& ArgumentParser::SubparserList::add_parser(std::string cmd){
	std::vector<char*> temp;
	temp.push_back(const_cast<char*>(_parser->_prog.c_str()));
	for (int i=0; i<_parser->_nargs; i++)
		temp.push_back(const_cast<char*>(_parser->_args[i].c_str()));
	_parser->_arglist[_arg].add_choice(cmd);
	ArgumentParser parser(_parser->_nargs, temp.data());
	parser._subparser = true;
	parser._subparser_cmd = cmd;
	_subparsers.push_back(parser);
	return _subparsers.back();
}

// Private Modifiers {{{2
void ArgumentParser::SubparserList::add(ArgumentParser* parser){
	_parser = parser;
	_parser->add_argument("subcommands").required(false);
	_arg = _parser->_arglist.size()-1;
	_parser->_subparser_index = _arg;
	_parser->_arglist[_arg]._subparser = true;
	_parser->_arglist[_arg]._dest = "";
	_parser->_arglist[_arg]._metavar[0] = "{}";
}

// Accessors {{{2
const ArgumentParser& ArgumentParser::SubparserList::operator[](int index) const{
	return _subparsers[index];
}
int ArgumentParser::SubparserList::size() const{return _subparsers.size();}
bool ArgumentParser::SubparserList::valid() const{return _parser != nullptr;}
