#pragma once
#include "ast.h"
#include "lexer.h"
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class ParseException : public std::exception
{
  private:
    std::string what_;

  public:
    ParseException(const tk &token, const tk_type_vec &expected);
    const char *what() const noexcept override { return what_.c_str(); }
};

nptr<> parse_template(tk_iterator &it);
