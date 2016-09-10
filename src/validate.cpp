#include "validate.h"
#include <cxxabi.h>
#include <sstream>

static std::string demangle(const char *name)
{
    int status = -1;
    std::unique_ptr<char, void (*)(void *)> res{abi::__cxa_demangle(name, NULL, NULL, &status),
                                                std::free};

    return (status == 0) ? res.get() : name;
}

/* Definition of dummy classes that are used for typechecking */
class IdentifierT
{
    IdentifierT() = delete;
};
static const std::type_index IdentifierType = typeid(IdentifierT);

class FieldT
{
    FieldT() = delete;
};
static const std::type_index FieldType = typeid(FieldT);

class List
{
    List() = delete;
};
static const std::type_index ListType = typeid(List);

const char *InvalidTypeException::what() const noexcept
{
    if (!what_.empty())
        return what_.c_str();

    std::stringstream s;
    s << "expression '";
    node_->print(s) << "' on";

    if (node_->begin_line() != node_->end_line())
        s << " lines " << node_->begin_line() + 1 << "-" << node_->end_line() + 1;
    else
        s << " line " << node_->begin_line() + 1;

    s << " has type '" << demangle(node_->type().name()) << "', but expected type '"
      << demangle(expected_type_.name()) << "'";

    what_ = s.str();
    return what_.c_str();
}

void UnOpNode::validate() const { type(); }

void UnOpNode::match_type(const std::type_index &arg, const std::type_index &type) const
{
    if (arg != IdentifierType && arg != type)
        throw InvalidTypeException(this->arg.get(), type);
}

std::type_index UnOpNode::type() const
{
    auto arg = this->arg->type();

    if (arg == IdentifierType)
        return IdentifierType;

    switch (op) {
    case UnOp::NEG:
        match_type(arg, typeid(double));
        return typeid(double);

    case UnOp::NOT:
        match_type(arg, typeid(bool));
        return typeid(bool);
    }

    throw InvalidTypeException(this, IdentifierType);
}

void BinOpNode::validate() const { type(); }

void BinOpNode::match_types(const std::type_index &lhs, const std::type_index &rhs,
                            const std::type_index &type) const
{
    if (lhs != IdentifierType && lhs != type)
        throw InvalidTypeException(this->lhs.get(), type);

    if (rhs != IdentifierType && rhs != type)
        throw InvalidTypeException(this->rhs.get(), type);
}

std::type_index BinOpNode::type() const
{
    auto lhs = this->lhs->type();
    auto rhs = this->rhs->type();

    switch (op) {
    case BinOp::AND:
    case BinOp::OR:
        match_types(lhs, rhs, typeid(bool));
        return typeid(bool);

    case BinOp::EQ:
    case BinOp::NEQ:
        if (lhs == typeid(bool) || rhs == typeid(bool))
            match_types(lhs, rhs, typeid(bool));
        else if (lhs == typeid(std::string) || rhs == typeid(std::string))
            match_types(lhs, rhs, typeid(std::string));
        else
            match_types(lhs, rhs, typeid(double));
        return typeid(bool);

    case BinOp::GT:
    case BinOp::GE:
    case BinOp::LT:
    case BinOp::LE:
        if (lhs == typeid(std::string) || rhs == typeid(std::string))
            match_types(lhs, rhs, typeid(std::string));
        else
            match_types(lhs, rhs, typeid(double));
        return typeid(bool);

    case BinOp::ADD:
    case BinOp::SUB:
    case BinOp::MUL:
    case BinOp::DIV:
        match_types(lhs, rhs, typeid(double));
        return typeid(double);

    case BinOp::DOT:
    case BinOp::ARROW:
        if (lhs != IdentifierType)
            throw InvalidTypeException(this, IdentifierType);
        if (rhs != FieldType)
            throw InvalidTypeException(this, FieldType);
        return IdentifierType;
    }

    throw InvalidTypeException(this, IdentifierType);
}

void StmtListNode::validate() const
{
    for (const auto &stmt : stmts)
        stmt->validate();
}

std::type_index FieldNode::type() const { return FieldType; }
std::type_index IdNode::type() const { return IdentifierType; }

void ForNode::validate() const
{
    if (filter->type() != typeid(bool))
        throw InvalidTypeException(filter.get(), typeid(bool));

    auto ctype = collection->type();
    if (ctype != IdentifierType && ctype != ListType)
        throw InvalidTypeException(collection.get(), ListType);

    body->validate();
}

void SetNode::validate() const
{
    value->validate();
    body->validate();
}

void ListNode::validate() const { type(); }

std::type_index ListNode::type() const
{
    if (values.size() < 2)
        return ListType;

    auto type = values.front()->type();
    for (const auto &value : values) {
        if (value->type() != type)
            throw InvalidTypeException(value.get(), type);
    }

    return ListType;
}

void IfNode::validate() const
{
    auto type = condition->type();

    if (type != typeid(bool) && type != IdentifierType)
        throw InvalidTypeException(condition.get(), typeid(bool));

    body->validate();
    elze->validate();
}

void VarNode::validate() const { expr->validate(); }

void ArgumentNode::validate() const
{
    if (dflt)
        dflt->validate();
}

void MacroNode::validate() const
{
    for (const auto &arg : args)
        arg->validate();

    body->validate();
}

void CallNode::validate() const
{
    for (const auto &value : args)
        value->validate();
}

void TemplateNode::validate() const
{
    for (const auto &macro : macros)
        macro->validate();

    body->validate();
}
