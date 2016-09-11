#include "ast.h"

class InvalidTypeException : public std::exception
{
  private:
    std::string what_;

  public:
    InvalidTypeException(const ExprNode &node, const std::type_index &type);
    const char *what() const noexcept override { return what_.c_str(); }
};
