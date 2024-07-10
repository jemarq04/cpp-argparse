#ifndef ARGPARSE_H
#define ARGPARSE_H
#include <iostream>
#include <fstream>
#include <sys/ioctl.h>
#include <string>
#include <vector>
#include <map>

namespace argparse{
	// Constants and Enums 
	const char OPTIONAL			= '?';
	const char ZERO_OR_MORE		= '*';
	const char ONE_OR_MORE		= '+';
	const std::string REMAINDER	= "...";
	const std::string SUPPRESS 	= "===SUPPRESS===";
	const std::string NONE		= "===NONE===";
	const std::string TRUE		= "True";
	const std::string FALSE		= "False";
	
	enum class HelpFormatter{
		Default,
		ArgumentDefaults,
		RawDescription,
		RawText,
		MetavarType
	};

	class ArgumentValue : public std::string {
	public:
		template <typename ... Args>
		ArgumentValue(Args ... args) : std::string(args ...){}
		
		operator int(){return strtol(c_str(), nullptr, 10);}
		operator float(){return strtof(c_str(), nullptr);}
		operator double(){return strtod(c_str(), nullptr);}
		operator bool(){return *this == TRUE;}

		bool is_none(){return *this == NONE;}
	};
	class ArgumentValueList : public std::vector<ArgumentValue>{
	public:
		ArgumentValueList(std::initializer_list<ArgumentValue> vals={}){
			for (auto& val : vals) push_back(val);
		}

		ArgumentValueList& operator=(const std::vector<std::string>& other){
			clear();
			for (auto& val : other) push_back(val);
			return *this;
		}
		operator int(){return strtol(at(0).c_str(), nullptr, 10);}
		operator float(){return strtof(at(0).c_str(), nullptr);}
		operator double(){return strtod(at(0).c_str(), nullptr);}
		operator bool(){return at(0) == TRUE;}
		operator std::string(){return at(0);}
		friend std::ostream& operator<<(std::ostream& os, const ArgumentValueList& arglist);

		std::string str() const{return at(0);}
		void str(const std::string &val){at(0) = val;}
		const char* c_str() const{return at(0).c_str();}
		bool is_none(){return at(0) == NONE;}
	};
	typedef std::map<std::string, ArgumentValueList> ArgumentMap;
	std::string format_args(ArgumentMap args);
	void print_args(ArgumentMap args, std::ostream& out=std::cout);
		
	
	class ArgumentParser {
		friend class SubparserList;
	private:
		// Private Enums 
		const int HELP_INDENT_MAX = 24;
		enum class ArgType {
			Optarg,
			Posarg
		};
		enum class ValType {
			Int,
			Float,
			String,
			Bool
		};
		enum class Action {
			Store,
			StoreConst,
			Count,
			Version,
			Help
		};
		// Argument Class 
		class Argument {
			friend class ArgumentParser;
			friend class SubparserList;
		public:
			Argument();
			Argument(ArgType type, ValType valtype, 
					std::string sname, std::string lname="");
			
			// Chain Modifiers
			Argument& nargs(int nargs);
			Argument& nargs(char nargs);
			Argument& nargs(std::string nargs);
			Argument& choices(std::string choices, char delim=',');
			Argument& required(bool required=true);
			Argument& dest(std::string dest);
			Argument& help(std::string="");
			Argument& metavar(std::string metavar, char delim=',');
			Argument& def(std::string def, char delim=',');
			Argument& constant(std::string con, char delim=',');
			Argument& version(std::string version);
			Argument& print_help();
			Argument& count();
			
		private:
			// Error Handling
			std::string error(std::string msg="") const;
			std::string error(int code, std::string msg="") const;
			
			// Modifiers
			int set(std::string value, bool has_digit_opt,	bool careful=true);
			void add_choice(std::string choice);
			
			// Accessors
			const std::string& operator[](int index) const;
			int size() const;
			std::string get_id() const;
			std::string get_help_id(HelpFormatter format) const;
			std::string get_usage(HelpFormatter format) const;
			std::string get_help(int linecap, int indent, 
					HelpFormatter format) const;
			
			// Helpers
			std::string trim(std::string str) const;
			
			// _type				= Enum storing the type of argument (optarg or 
			// 							posarg)
			// _valtype				= Enum storing the basic datatype of the arg
			// _action				= Enum storing the action the argument does
			ArgType _type;
			ValType _valtype;
			Action _action;
			
			// _nargs				= Number of arguments the arg takes
			// _more_nargs			= Special cases ('*', '+', '?') for _nargs
			// _sname				= "Short" name for optarg (e.g. -a, -b)
			// 							OR name of posarg
			// _lname				= "Long" name for optarg (e.g. --num, 
			// 							--str) (this is empty for positional 
			// 							arguments)
			// _dest				= Identifier for the argument. This is the key
			// 							in the map returned from 
			// 							ArgumentParser::parse_args()
			// _help				= Non-formatted help string for the arg
			// _version				= Version string. Only used for args that print
			// 							the given version when provided
			int _nargs;
			char _more_nargs;
			std::string _sname, _lname, _dest;
			std::string _help, _version;
			
			// _found				= Bool storing whether the arg was found in 
			// 							ArgumentParser::parse_args()
			// _required			= Bool storing whether the arg is required
			// _subparser			= Bool storing whether the arg chooses the
			// 							subparser for the program
			// _has_default			= Bool storing whether the arg has a default
			// _override_metavar	= Bool storing whether the arg has a custom 
			// 							metavar
			bool _found, _required, _subparser;
			bool _has_default, _override_metavar;
			
			// _val					= Vector of values stored by the arg
			// _metavar				= Vector of the "Metavariable" of the arg
			// 							This is what is shown in usage/help messages
			// _def					= Vector of default values given if the arg is 
			// 							not given in the command line
			// _choices				= Vector of valid choices for values stored by 
			// 							the arg
			// _const				= Vector of constants stored in specific cases
			// 							(e.g. boolean flag that stores the const
			// 							value when given or the default when 
			// 							missing)
			std::vector<std::string> _val, _metavar, _def, _choices, _const;
		};
	public:
		class SubparserList{
			friend class ArgumentParser;
		public:
			// Chain Modifiers
			SubparserList& title(std::string title);
			SubparserList& description(std::string desc);
			SubparserList& prog(std::string prog);
			SubparserList& dest(std::string dest);
			SubparserList& help(std::string help);
			SubparserList& metavar(std::string metavar);
			SubparserList& required(bool required=true);
			
			// Modifiers
			ArgumentParser& add_parser(std::string cmd);
		private:
			SubparserList();
			
			// Private Modifiers
			void add(ArgumentParser* parser);
			
			// Accessors
			const ArgumentParser& operator[](int index) const;
			int size() const;
			bool valid() const;
			
			// _parser			= Pointer to the parser one step above this
			// 						collection of subparsers
			// _subparsers		= Vector of the subparsers
			// _arg				= Index of the subparser argument in the base
			// 						parser's positional argument list
			ArgumentParser* _parser;
			std::vector<ArgumentParser> _subparsers;
			int _arg;
			
			// _title			= Title of the subparser arg group
			// 						(if empty, the help is added to posargs)
			// _desc			= Description of the subparser arg group
			// 						(if added, _title="subcommands"
			// _help			= Help message for the subparser argument
			// _dest			= Destination of the subparser argument
			// 						(by default, it is suppressed)
			// _metavar			= Metavariable for the subparser argument
			// _prog			= Name of the base program (by default, it is
			// 						the program name plus any following positional
			// 						arguments up to the subparser argument)
			std::string _title, _desc, _help;
			std::string _dest, _metavar, _prog;
			
			// _override_title	= Bool storing whether the title has already been
			// 						overridden
			bool _override_title;
		};
	public:
		ArgumentParser(int nargs, char **args);
		
		// Error Handling
		void error(std::string msg="");
		
		// Modifiers
		template <typename T>
		Argument& add_argument(std::string sname, std::string lname);
		template <typename T>
		Argument& add_argument(std::string name);
		Argument& add_argument(std::string sname, std::string lname);
		Argument& add_argument(std::string name);
		
		SubparserList& add_subparsers();
		
		// Chain Modifiers
		ArgumentParser& prog(std::string prog);
		ArgumentParser& usage(std::string usage);
		ArgumentParser& description(std::string desc);
		ArgumentParser& epilog(std::string epilog);
		ArgumentParser& add_help(bool add_help);
		ArgumentParser& argument_default(std::string none);
		ArgumentParser& parent(const ArgumentParser& parser);
		ArgumentParser& conflict_handler(std::string name);
		ArgumentParser& formatter_class(HelpFormatter format);
		ArgumentParser& fromfile_prefix_chars(std::string prefix);
		
		ArgumentParser& set_defaults(std::map<std::string, std::string> defaults);
		ArgumentParser& help(std::string help);

		ArgumentParser& set_linecap(int linecap);
		
		// Accessors
		std::string get_prog() const;
		std::vector<std::string> get_default(std::string name) const;
		std::string format_usage();
		std::string format_help();
		void print_usage(std::ostream& out=std::cout);
		void print_help(std::ostream& out=std::cout);
		
		// Parse
		ArgumentMap parse_args(std::vector<std::string> argv = {});
		
	private:
		// Private Helpers
		void check_conflict(const Argument& arg, ArgType type);
		inline bool valid_value(std::string val) const;
		
		void load_helpstring();
		std::vector<std::string> read_args_from_files(std::vector<std::string> args);
		bool find_arg(const Argument& arg, std::string given) const;
		bool contains(std::string given, char sname, bool flag=false) const;
		
		std::string get_subparser_help(int linecap, int indent, 
				HelpFormatter format) const;
		
		std::string trim(std::string str, char c='\n') const;
		std::string wrap(std::string input, int indent_level=0) const;
		
		// _nargs			= Number of arguments from command line
		// _args			= Vector of arguments from command line
		int _nargs;
		std::vector<std::string> _args;
		
		// _optlist			= Vector of optional arguments (optargs)
		// _arglist			= Vector of positional arguments (posargs)
		// _helpargs		= Vector of flags that print help
		// _opthelp			= Formatted help message for optargs
		// _arghelp			= Formatted help message for posargs
		// _none_str		= String value that args are assigned
		// 						if they are not filled
		// 						(default: NONE)
		// _defaults		= Map of default values for argument parsing
		// _req_posargs		= Number of required posargs
		std::vector<Argument> _optlist, _arglist, _helpargs;
		std::string _opthelp, _arghelp, _none_str;
		std::map<std::string, std::string> _defaults;
		int _req_posargs;
		
		// _format			= Enum storing how the help screen (and usage) 
		// 						is formatted
		// _resolve			= Bool storing whether the conflict_handler is 
		// 						set to resolve
		// _fromfile_prefix = String containing the prefix characters that denote
		// 						desired input from a file
		HelpFormatter _format;
		bool _resolve;
		std::string _fromfile_prefix;
		
		// _prog			= Name of the program
		// _desc			= Description of the program
		// _epilog			= Post-help description of the program
		// _usage			= Usage string
		// _help_indent		= Length of the indent before help strings of args
		// _linecap			= Width of the terminal screen (-2)
		// _preusage_len	= Length of the initial string before the program usage
		// _usage_line_len	= Length of the current line in the usage string
		std::string _prog, _desc, _epilog, _usage;
		int _help_indent, _linecap;
		int _preusage_len, _usage_line_len;
		
		// _parsed			= Bool storing whether arguments have been parsed
		// 						(stored for error-printing purposes)
		// _has_digit_opt	= Bool storing whether there is an optarg that
		// 						looks like a negative number
		// _multiline_usage	= Bool storing whether the usage string is multiple 
		// 						lines (stored for posarg usage formatting)
		// _override_usage	= Bool storing whether usage has been given a
		// 						custom value
		// _override_prog	= Bool storing whether the program name has been given
		// 						a custom value
		// _add_help		= Bool storing whether the -h/--help arg
		// 						should be added (default: true)
		bool _parsed, _has_digit_opt, _multiline_usage;
		bool _override_usage, _override_prog, _add_help, _added_help;
		
		// _subparser		= Bool storing whether the parser is a subparser
		// _subparser_index = Index for the subparser argument in the positional
		// 						argument list
		// _base_req_posargs= Number of required positional arguments from the base
		// 						parser (set to 0 if the parser is not a subparser)
		// _subparser_cmd	= Name of the subparser command (empty if the parser
		// 						is not a subparser)
		// _subparser_help	= Help string for the subparser (empty if the parser
		// 						is not a subparser)
		// _subhelp			= Formatted help message for the subparser arg group
		// _subparsers		= SubparserList object that allows user to format
		// 						the subparser group help message and add parsers
		bool _subparser;
		int _subparser_index, _base_req_posargs;
		std::string _subparser_cmd, _subparser_help, _subhelp;
		SubparserList _subparsers;
	};
};

#endif//ARGPARSE_H
