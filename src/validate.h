#include "ast.h"

class InvalidTypeException : std::exception
{
  private:
    const ExprNode *node_;
    std::type_index expected_type_;
    mutable std::string what_;

  public:
    InvalidTypeException(const ExprNode *node, const std::type_index &type)
        : node_(node), expected_type_(type)
    {
    }

    const Node *node() const { return node_; }
    const std::type_index &expected_type() const { return expected_type_; }
    const char *what() const noexcept override;
};
