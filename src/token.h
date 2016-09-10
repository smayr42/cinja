#pragma once
#include <regex>

class tk_type
{
  private:
    std::string name_;
    std::regex regexp_;

  public:
    explicit tk_type() : name_("UNKNOWN"){};

    explicit tk_type(const std::string &name) : name_(name){};

    explicit tk_type(const std::string &name, const std::string &regexp)
        : name_(name),
          regexp_(regexp, std::regex_constants::nosubs | std::regex_constants::optimize){};

    tk_type(const tk_type &) = delete;
    tk_type &operator=(const tk_type &) = delete;

    const std::string &name() const { return name_; }
    const std::regex &regexp() const { return regexp_; }
};

typedef const tk_type *tk_type_ptr;

namespace tk_types
{
extern const tk_type_ptr EOI;
extern const tk_type_ptr UNKNOWN;
}

class tk
{
  private:
    tk_type_ptr type_;
    std::string value_;
    long start_line_;
    long num_lines_;

  public:
    tk() : type_(tk_types::UNKNOWN), start_line_(0), num_lines_(0){};

    tk(tk_type_ptr type, const std::string &value, const long start_line)
        : type_(type), value_(value), start_line_(start_line),
          num_lines_(std::count(value.begin(), value.end(), '\n')){};

    tk_type_ptr type() const { return type_; }
    const long start_line() const { return start_line_; }
    const long num_lines() const { return num_lines_; }
    const std::string &value() const { return value_; }
};

std::ostream &operator<<(std::ostream &o, const tk &b);

typedef std::vector<tk_type_ptr> tk_type_vec;
typedef std::vector<tk> tk_vec;
