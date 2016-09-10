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
    tk token_;
    tk_type_vec expected_;
    mutable std::string what_;

  public:
    ParseException(const tk &token, const tk_type_vec &expected)
        : token_(token), expected_(expected){};

    const tk_type_vec &expected() const { return expected_; }
    const tk &token() const { return token_; }
    const char *what() const noexcept;
};

nptr<> parse_template(tk_iterator &it);
