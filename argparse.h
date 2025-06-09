#ifndef ARGPARSE_H
#define ARGPARSE_H
#define ARGPARSE_VERSION 3.0.0
#include <cctype>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/ioctl.h>
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <cassert>

// Forward Declarations
namespace argparse{
  enum class ValType;
  class Action;
  class ActionsContainer;
  class ArgumentGroup;
  class MutuallyExclusiveGroup;
}

std::ostream& operator<<(std::ostream&, const argparse::ValType&);

namespace argparse{
  // Constants and Enums 
  const char OPTIONAL          = '?';
  const char ZERO_OR_MORE      = '*';
  const char ONE_OR_MORE       = '+';
  const char DEFAULT           = '=';
  const std::string REMAINDER  = "...";
  const std::string SUPPRESS   = "===SUPPRESS===";
  const std::string NONE       = "===NONE===";
  const std::string TRUE       = "True";
  const std::string FALSE      = "False";
  const std::string _UNRECOGNIZED_ARGS_ATTR = "_unrecognized_args";

  // Type definitions
  enum class HelpFormatter{
    Default,
    ArgumentDefaults,
    RawDescription,
    RawText,
    MetavarType
  };
  enum class ValType {
    Bool,
    Int,
    Float,
    Double,
    String
  };

  typedef std::variant<
    bool, int, float, double, std::string> ArgumentTypeBase;
  typedef std::variant<
    std::vector<int>, std::vector<float>,
    std::vector<double>, std::vector<std::string> > ArgumentTypeVec;
  typedef std::variant<
    char, ValType, bool, int, float, double, std::string,
    std::vector<int>, std::vector<float>,
    std::vector<double>, std::vector<std::string> > ArgumentType;
  typedef std::optional<ArgumentType> Argument;
  typedef std::map<std::string, ArgumentType> Kwargs;

  // Helper Namespace
  namespace helpers {

    template <typename From, typename To>
    void copy_variant(const From& from, To& to){
      to = std::visit([](auto&& arg) -> To {return arg;}, from);
    }

    bool isidentifier(std::string str);
    std::string join(std::string connector, std::vector<std::string> items);

    std::optional<std::string> _get_action_name(Action *action);
  };

  //TODO
  class AttributeHolder {
    protected:
      AttributeHolder();

      std::string _type_name;
      Kwargs _kwargs;
      std::vector<std::string> _arg_strings;
      std::map<std::string, ArgumentType> _star_args;
  };

  class Action {
    friend class ActionsContainer;
    public:
      Action(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Accessors
      std::string format_usage() const;

      //Debugging
      void printme() const;

      template <typename T>
      void print(const T& val) const{std::cout << val << std::endl;}

      template <>
      void print(const ValType& val) const{std::cout << val << std::endl;}

      template <typename T>
      void print(const std::vector<T>& vec) const{
        std::cout << "{ ";
        for (T e : vec) std::cout << e << ' ';
        std::cout << '}';
      }

      //Operators
      //virtual void operator(ArgumentParser& parser, );//TODO

    protected:
      std::vector<std::string> _option_strings;
      std::string _dest, _metavar;
      std::variant<int, char> _nargs;
      std::optional<ArgumentType> _const, _default;
      ValType _type;
      ArgumentTypeVec _choices;
      bool _required;
      std::string _help;
  };

  class BooleanOptionalAction : public Action {
    public:
      BooleanOptionalAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Accessors
      std::string format_usage() const;

      //Operators
      //void operator()(...); //TODO
  };

  class StoreAction : public Action {
    public:
      StoreAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
  };

  class StoreConstAction : public Action {
    public:
      StoreConstAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
  };

  class StoreTrueAction : public StoreConstAction {
    public:
      StoreTrueAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
  };

  class StoreFalseAction : public StoreConstAction {
    public:
      StoreFalseAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
  };

  class AppendAction : public Action {
    public:
      AppendAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs);

      //Operators
      //void operator()(...); //TODO
  };

  class AppendConstAction : public Action {
    public:
      AppendConstAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
  };

  class CountAction : public Action {
    public:
      CountAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
  };  

  class HelpAction : public Action {
    public:
      HelpAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
  };  

  class VersionAction : public Action {
    public:
      VersionAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
    private:
      std::string _version;
  };

  class SubParsersAction : public Action {
    class ChoicesPseudoAction : public Action {
      public:
        ChoicesPseudoAction(std::string name, std::vector<std::string> aliases, std::string help);
    };
    public:
      SubParsersAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

    private:
      std::string _prog_prefix;
      //TODO others...
  };

  class ExtendAction : public AppendAction {
    public:
      ExtendAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs={});

      //Operators
      //void operator()(...); //TODO
  };

  class Namespace : public AttributeHolder {
    public:
      Namespace(Kwargs kwargs={});

      //Accessors
      Kwargs::iterator begin();
      Kwargs::iterator end();
      Kwargs::const_iterator cbegin() const;
      Kwargs::const_iterator cend() const;
      Kwargs::reverse_iterator rbegin();
      Kwargs::reverse_iterator rend();
      Kwargs::const_reverse_iterator crbegin() const;
      Kwargs::const_reverse_iterator crend() const;
      
      bool contains(std::string key) const;

      //Operators
      bool operator==(const Namespace& other);
  };

  class ActionsContainer {
    public:
      ActionsContainer(std::string description, std::string prefix_chars, std::optional<ArgumentType> argument_default);

      void set_defaults(Kwargs kwargs);
      std::optional<ArgumentType> get_default(std::string dest) const;

      void add_argument(std::vector<ArgumentType> args, Kwargs kwargs);

      void register_(std::string registry_name, std::string value, ArgumentType object);

    protected:
      std::optional<ArgumentType> registry_get(std::string registry_name, std::string value, std::optional<ArgumentType> def=std::nullopt);

      Action* add_action(Action *action);
      void remove_action(Action *action);
      //void add_container_actions(ActionsContainer* container);

      //return-type get_positional_kwargs(std::string dest, Kwargs kwargs={});
      //return-type get_optional_kwargs(std::string dest, Kwargs kwargs={});
      
      //return-type pop_action_class(Kwargs& kwargs, std::optional<ArgumentType> def=std::nullopt);

      //return-type check_conflict(Action* action);

      std::string _description, _prefix_chars;
      std::optional<ArgumentType> _argument_default;

      std::map<std::string, Kwargs> _registries;
      
      Kwargs _defaults;

      std::vector<Action*> _actions;
      std::map<std::string, Action*> _option_string_actions;

      std::vector<ArgumentGroup*> _action_groups;
      std::vector<ArgumentGroup*> _mutually_exclusive_groups;

      // TODO
      // negative_number_matcher
      // has_negative_number_optionals
  };
};

#endif//ARGPARSE_H
