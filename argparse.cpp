#include "argparse.h"
using namespace argparse;

std::ostream& operator<<(std::ostream& out, const ValType& val){
  if (val == ValType::Bool) out << "bool";
  else if (val == ValType::Int) out << "int";
  else if (val == ValType::Float) out << "float";
  else if (val == ValType::Double) out << "double";
  else if (val == ValType::String) out << "string";
  return out;
}

bool helpers::isidentifier(std::string str){
  if (isdigit(str[0])) return false;
  for (char c : str)
    if (!isalnum(c) || c == '_') return false;
  return true;
}
std::string helpers::join(std::string connector, std::vector<std::string> items){
  std::string result = "";
  if (items.size() != 0){
    result += items[0];
    for (size_t i=1; i<items.size(); i++)
      result += connector + items[i];
  }
  return result;
}

AttributeHolder::AttributeHolder(){}

Action::Action(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  _option_strings(option_strings),
  _dest(dest),
  _nargs(DEFAULT),
  _const(std::nullopt),
  _default(std::nullopt),
  _type(ValType::String),
  _required(false)
{
  //TODO: commented out to alow for ChoicesPseudoAction constructor
  //if (_option_strings.size() == 0)
  //  throw std::invalid_argument("option_strings argument must be non-empty");
  if (_dest.size() == 0)
    throw std::invalid_argument("dest argument must be non-empty");

  for (auto& [key, val] : kwargs){
    if (key == "nargs"){
      if (std::holds_alternative<int>(val)) _nargs = std::get<int>(val);
      else if (std::holds_alternative<char>(val)) _nargs = std::get<char>(val);
      else throw std::invalid_argument("invalid argument: " + key);
    }
    else if (key == "const")
      helpers::copy_variant(val, _const);
    else if (key == "default")
      helpers::copy_variant(val, _default);
    else if (key == "type"){
      if (std::holds_alternative<ValType>(val)) _type = std::get<ValType>(val);
      else throw std::invalid_argument("invalid argument: " + key);
    }
    else if (key == "choices"){
      if (std::holds_alternative<std::vector<int>>(val)) _choices = std::get<std::vector<int>>(val);
      else if (std::holds_alternative<std::vector<float>>(val)) _choices = std::get<std::vector<float>>(val);
      else if (std::holds_alternative<std::vector<double>>(val)) _choices = std::get<std::vector<double>>(val);
      else if (std::holds_alternative<std::vector<std::string>>(val)) _choices = std::get<std::vector<std::string>>(val);
      else throw std::invalid_argument("invalid argument: " + key);
    }
    else if (key == "required"){
      if (std::holds_alternative<bool>(val)) _required = std::get<bool>(val);
      else throw std::invalid_argument("invalid argument: " + key);
    }
    else if (key == "help"){
      if (std::holds_alternative<std::string>(val)) _help = std::get<std::string>(val);
      else throw std::invalid_argument("invalid argument: " + key);
    }
    else if (key == "metavar"){
      if (std::holds_alternative<std::string>(val)) _metavar = std::get<std::string>(val);
      else throw std::invalid_argument("invalid argument: " + key);
    }
    else throw std::invalid_argument("unrecognized argument: " + key);
  }

  // Type checking
  if (_type == ValType::Bool){
    bool valid_const   = !_const.has_value() || std::holds_alternative<bool>(*_const);
    bool valid_default = !_default.has_value() || std::holds_alternative<bool>(*_default);
    bool valid_choices = std::visit([](auto && arg)->bool{return arg.size() == 0;}, _choices);
    if ( !(valid_const && valid_default && valid_choices) )
      throw std::invalid_argument("bool specified, provided non-bool types and/or non-zero size for choices");
  }
  else if (_type == ValType::Int){
    bool valid_const   = !_const.has_value() || std::holds_alternative<int>(*_const);
    bool valid_default = !_default.has_value() || std::holds_alternative<int>(*_default);
    bool valid_choices = std::holds_alternative<std::vector<int>>(_choices);
    if ( !(valid_const && valid_default && valid_choices) )
      throw std::invalid_argument("int specified, provided non-int types");
  }
  else if (_type == ValType::Float){
    bool valid_const   = !_const.has_value() || std::holds_alternative<float>(*_const);
    bool valid_default = !_default.has_value() || std::holds_alternative<float>(*_default);
    bool valid_choices = std::holds_alternative<std::vector<float>>(_choices);
    if ( !(valid_const && valid_default && valid_choices) )
      throw std::invalid_argument("float specified, provided non-float types");
  }
  else if (_type == ValType::Double){
    bool valid_const   = !_const.has_value() || std::holds_alternative<double>(*_const);
    bool valid_default = !_default.has_value() || std::holds_alternative<double>(*_default);
    bool valid_choices = std::holds_alternative<std::vector<double>>(_choices);
    if ( !(valid_const && valid_default && valid_choices) )
      throw std::invalid_argument("double specified, provided non-double types");
  }
  else if (_type == ValType::String){
    bool valid_const   = !_const.has_value() || std::holds_alternative<std::string>(*_const);
    bool valid_default = !_default.has_value() || std::holds_alternative<std::string>(*_default);
    bool valid_choices = std::holds_alternative<std::vector<std::string>>(_choices);
    if ( !(valid_const && valid_default && valid_choices) )
      throw std::invalid_argument("string specified, provided non-string types");
  }
}

std::string Action::format_usage() const{return _option_strings[0];}

//TODO: debug
void Action::printme() const{
  std::cout << "_option_strings: ";
  if (_option_strings.size()){
    std::cout << _option_strings[0];
    for (size_t i=1; i<_option_strings.size(); i++)
      std::cout << ", " << _option_strings[i];
  }
  std::cout << std::endl;
  std::cout << "_dest: " << _dest << std::endl;
  std::cout << "_nargs: ";
  if (std::holds_alternative<int>(_nargs)) std::cout << std::get<int>(_nargs) << std::endl;
  else std::cout << std::get<char>(_nargs) << std::endl;
  std::cout << "_const: ";
  if (_const.has_value()) std::visit([this](auto&& arg){this->print(arg);}, *_const);
  else std::cout << "None" << std::endl;
  std::cout << "_default: ";
  if (_default.has_value()) std::visit([this](auto&& arg){this->print(arg);}, *_default);
  else std::cout << "None" << std::endl;
  std::cout << "_type: " << _type << std::endl;
  std::cout << "_choices: ";
  std::visit([this](auto&& arg){this->print(arg);}, _choices);
  //if (_choices.size()){
  //  std::cout << _choices[0];
  //  for (size_t i=1; i<_choices.size(); i++)
  //    std::cout << ", " << _choices[i];
  //}
  std::cout << std::endl;
  std::cout << "_required: " << _required << std::endl;
  std::cout << "_help: " << _help << std::endl;
  std::cout << "_metavar: " << _metavar << std::endl;
}

BooleanOptionalAction::BooleanOptionalAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  std::vector<std::string> new_option_strings;
  for (std::string str : _option_strings){
    new_option_strings.push_back(str);
    if (str.rfind("--", 0) == 0)
      new_option_strings.push_back("--no-" + str.substr(2));
  }

  for (std::string key : {"choices", "metavar"})
    if (kwargs.find(key) != kwargs.end())
      throw std::invalid_argument("invalid argument for BooleanOptionalAction: " + key);
}

std::string BooleanOptionalAction::format_usage() const{
  return helpers::join(" | ", _option_strings);  
}
      
StoreAction::StoreAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  if (std::holds_alternative<int>(_nargs) && std::get<int>(_nargs) == 0)
    throw std::invalid_argument("nargs for store actions must be != 0");
  if (_const.has_value() && !(std::holds_alternative<char>(_nargs) && std::get<char>(_nargs) == OPTIONAL))
    throw std::invalid_argument((std::string)"nargs must be " + OPTIONAL + " to supply const");
}

StoreConstAction::StoreConstAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  _nargs = 0;
  for (std::string key : {"nargs", "choices", "metavar"})
    if (kwargs.find(key) != kwargs.end())
      throw std::invalid_argument("invalid argument for StoreConstAction: " + key);
}

StoreTrueAction::StoreTrueAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  StoreConstAction(option_strings, dest, kwargs)
{
  _default = false;
  _const   = true;
  for (std::string key : {"const", "default"})
    if (kwargs.find(key) != kwargs.end())
      throw std::invalid_argument("invalid argument for StoreTrueAction: " + key);
}

StoreFalseAction::StoreFalseAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  StoreConstAction(option_strings, dest, kwargs)
{
  _default = true;
  _const   = false;
  for (std::string key : {"const", "default"})
    if (kwargs.find(key) != kwargs.end())
      throw std::invalid_argument("invalid argument for StoreFalseAction: " + key);
}

AppendAction::AppendAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  if (std::holds_alternative<int>(_nargs) && std::get<int>(_nargs) == 0)
    throw std::invalid_argument("nargs for store actions must be != 0");
  if (_const.has_value() && !(std::holds_alternative<char>(_nargs) && std::get<char>(_nargs) == OPTIONAL))
    throw std::invalid_argument((std::string)"nargs must be " + OPTIONAL + " to supply const");
}

AppendConstAction::AppendConstAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  _nargs = 0;
  for (std::string key : {"nargs", "choices"})
    if (kwargs.find(key) != kwargs.end())
      throw std::invalid_argument("invalid argument for AppendConstAction: " + key);
}

CountAction::CountAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  _nargs = 0;
  for (std::string key : {"nargs", "const", "choices", "metavar"})
    if (kwargs.find(key) != kwargs.end())
      throw std::invalid_argument("invalid argument for CountAction: " + key);
}

HelpAction::HelpAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  _nargs = 0;
  for (std::string key : {"nargs", "const", "choices", "metavar"})
    if (kwargs.find(key) != kwargs.end())
      throw std::invalid_argument("invalid argument for HelpAction: " + key);
}

VersionAction::VersionAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  _nargs = 0;
  for (std::string key : {"nargs", "const", "choices", "metavar"})
    if (kwargs.find(key) != kwargs.end())
      throw std::invalid_argument("invalid argument for VersionAction: " + key);
}

SubParsersAction::ChoicesPseudoAction::ChoicesPseudoAction(std::string name, std::vector<std::string> aliases, std::string help) :
  Action({}, name, {{"help", help}})
{
  _metavar = name;
  if (aliases.size())
    _metavar += " (" + helpers::join(", ", aliases) + ")";
}

SubParsersAction::SubParsersAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  Action(option_strings, dest, kwargs)
{
  //TODO
}

ExtendAction::ExtendAction(std::vector<std::string> option_strings, std::string dest, Kwargs kwargs) :
  AppendAction(option_strings, dest, kwargs){}

Namespace::Namespace(Kwargs kwargs){
  _kwargs = kwargs;
}

Kwargs::iterator Namespace::begin(){
  return _kwargs.begin();
}

Kwargs::iterator Namespace::end(){
  return _kwargs.end();
}

Kwargs::const_iterator Namespace::cbegin() const{
  return _kwargs.cbegin();
}

Kwargs::const_iterator Namespace::cend() const{
  return _kwargs.cend();
}

Kwargs::reverse_iterator Namespace::rbegin(){
  return _kwargs.rbegin();
}

Kwargs::reverse_iterator Namespace::rend(){
  return _kwargs.rend();
}

Kwargs::const_reverse_iterator Namespace::crbegin() const{
  return _kwargs.crbegin();
}

Kwargs::const_reverse_iterator Namespace::crend() const{
  return _kwargs.crend();
}

bool Namespace::contains(std::string key) const{
  for (auto it : _kwargs)
    if (it.first == key)
      return true;
  return false;
}

bool Namespace::operator==(const Namespace& other){
  for (auto it : _kwargs)
    if (!other.contains(it.first))
      return false;
  return _kwargs.size() == other._kwargs.size();
}

ActionsContainer::ActionsContainer(std::string description, std::string prefix_chars, std::optional<ArgumentType> argument_default) :
  _description(description), _prefix_chars(prefix_chars), _argument_default(argument_default)
{
  _register("action", argparse::NONE, "StoreAction");
  _register("action", "store",        "StoreAction");
  _register("action", "store_const",  "StoreConstAction");
  _register("action", "store_true",   "StoreTrueAction");
  _register("action", "store_false",  "StoreFalseAction");
  _register("action", "append",       "AppendAction");
  _register("action", "append_const", "AppendConstAction");
  _register("action", "count",        "CountAction");
  _register("action", "help",         "HelpAction");
  _register("action", "version",      "VersionAction");
  _register("action", "parsers",      "SubParsersAction");
  _register("action", "extend",       "ExtendAction");
}

void ActionsContainer::set_defaults(Kwargs kwargs){
  for (auto it : kwargs)
    _defaults[it.first] = it.second;

  for (Action* action : _actions)
    for (auto it : kwargs)
      if (it.first == action->_dest)
        action->_default = it.second;
}

std::optional<ArgumentType> ActionsContainer::get_default(std::string dest) const{
  for (Action* action : _actions)
    if (action->_dest == dest && action->_default.has_value())
      return action->_default;
  for (auto it : _defaults)
    if (it.first == dest)
      return it.second;
  return std::nullopt;
}

void ActionsContainer::_register(std::string registry_name, std::string value, ArgumentType object){
  bool found = false;
  for (auto& it : _registries){
    if (it.first == registry_name){
      found = true;
      it.second[value] = object;
    }
  }
  if (!found){
    _registries[registry_name] = {};
    _registries[registry_name][value] = object;
  }
}

std::optional<ArgumentType> ActionsContainer::registry_get(std::string registry_name, std::string value, std::optional<ArgumentType> def){
  for (auto it : _registries[registry_name])
    if (it.first == value)
      return it.second;
  return def;
}
