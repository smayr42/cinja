#pragma once
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <typeindex>
#include <unordered_set>
#include <vector>

enum class BinOp {
    DOT,
    ARROW,

    ADD,
    SUB,
    MUL,
    DIV,

    EQ,
    NEQ,
    GT,
    GE,
    LT,
    LE,

    AND,
    OR
};

enum class UnOp { NEG, NOT };

/* base class for all ast nodes */
class Node
{
  public:
    typedef std::unordered_multiset<std::string> mset;
    typedef std::set<std::string> set;
    typedef std::ostream ostr;

    ostr &print(ostr &o, unsigned lvl = 0) const
    {
        set fsym;
        mset bsym;
        return print(o, fsym, bsym, lvl);
    }

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const = 0;
    virtual void validate() const = 0;
    virtual ~Node() {}
};

template <typename N = Node> using nptr = std::unique_ptr<N>;

/* base class for all statements */
class StmtNode : public Node
{
  public:
};

/* base class for all expressions */
class ExprNode : public Node
{
  public:
    virtual long begin_line() const = 0;
    virtual long end_line() const = 0;
    virtual std::type_index type() const = 0;
    virtual void validate() const = 0;
};

class StmtListNode : public Node
{
  public:
    std::vector<nptr<StmtNode>> stmts;

    StmtListNode() = default;
    StmtListNode(std::vector<nptr<StmtNode>> &&stmts) : stmts(std::move(stmts)) {}

    virtual void validate() const override;
    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
};

class FieldNode : public ExprNode
{
  private:
    long line_no;

  public:
    std::string name;

    FieldNode(const std::string &name, long line_no) : line_no(line_no), name(name) {}

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual std::type_index type() const override;
    virtual long begin_line() const override { return line_no; }
    virtual long end_line() const override { return line_no; }
    virtual void validate() const override {}
};

/* a node that represents an identifier */
class IdNode : public ExprNode
{
  private:
    long line_no;

  public:
    std::string name;
    std::string nspace;
    bool binding;

    IdNode(const std::string &name, long line_no) : line_no(line_no), name(name) {}

    IdNode(const std::string &prefix, const std::string &name, long line_no, bool binding = false)
        : line_no(line_no), name(prefix + "_" + name), binding(binding)
    {
    }

    IdNode(const std::string &name, long line_no, const std::string &nspace, bool binding = false)
        : line_no(line_no), name(name), nspace(nspace), binding(binding)
    {
    }

    std::string full_name() const
    {
        if (nspace.empty())
            return name;

        return nspace + "::" + name;
    }

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual std::type_index type() const override;
    virtual long begin_line() const override { return line_no; }
    virtual long end_line() const override { return line_no; }
    virtual void validate() const override {}
};

class ForNode : public StmtNode
{
  public:
    nptr<IdNode> var;
    nptr<ExprNode> collection;
    nptr<ExprNode> filter;
    nptr<StmtListNode> body;

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual void validate() const override;
};

class SetNode : public StmtNode
{
  public:
    nptr<IdNode> var;
    nptr<ExprNode> value;
    nptr<StmtListNode> body;

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual void validate() const override;
};

class ContentNode : public StmtNode
{
  public:
    std::string content;

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual void validate() const override {}
};

class ListNode : public ExprNode
{
  private:
    long line_no;

  public:
    std::vector<nptr<ExprNode>> values;

    ListNode(std::vector<nptr<ExprNode>> &&values) : values(std::move(values)) {}

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual void validate() const override;
    virtual long begin_line() const override { return line_no; }
    virtual long end_line() const override { return line_no; }
    virtual std::type_index type() const override;
};

class IfNode : public StmtNode
{
  public:
    nptr<ExprNode> condition;
    nptr<StmtListNode> body;
    nptr<StmtListNode> elze;

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual void validate() const override;
};

class VarNode : public StmtNode
{
  public:
    nptr<ExprNode> expr;

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual void validate() const override;
};

template <typename T> class LiteralNode : public ExprNode
{
  private:
    long line_no;

  public:
    LiteralNode(const T &value, long line_no) : line_no(line_no), value(value) {}
    T value;

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override
    {
        return o << value;
    }

    virtual long begin_line() const override { return line_no; }
    virtual long end_line() const override { return line_no; }
    virtual std::type_index type() const override { return typeid(T); }
    virtual void validate() const override {}
};

class UnOpNode : public ExprNode
{
  public:
    UnOp op;
    nptr<ExprNode> arg;

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual long begin_line() const override { return arg->begin_line(); }
    virtual long end_line() const override { return arg->end_line(); }
    virtual void validate() const override;
    virtual std::type_index type() const override;

  private:
    void match_type(std::type_index const &arg, std::type_index const &type) const;
};

class BinOpNode : public ExprNode
{
  public:
    BinOp op;
    nptr<ExprNode> lhs;
    nptr<ExprNode> rhs;

    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
    virtual long begin_line() const override { return lhs->begin_line(); }
    virtual long end_line() const override { return rhs->end_line(); }
    virtual void validate() const override;
    virtual std::type_index type() const override;

  private:
    void match_types(std::type_index const &lhs, std::type_index const &rhs,
                     std::type_index const &type) const;
};

class CallNode : public StmtNode
{
  public:
    nptr<IdNode> id;
    std::vector<nptr<ExprNode>> args;

    virtual void validate() const override;
    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const override;
};

class ArgumentNode : public Node
{
  public:
    nptr<IdNode> id;
    nptr<ExprNode> dflt;

    virtual void validate() const override;
    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
};

class MacroNode : public Node
{
  public:
    nptr<IdNode> id;
    std::vector<nptr<ArgumentNode>> args;
    nptr<StmtListNode> body;

    virtual void validate() const override;
    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
};

/* root node of the parsed template */
class TemplateNode : public Node
{
  public:
    nptr<StmtListNode> body;
    std::vector<nptr<MacroNode>> macros;

    TemplateNode() = default;
    virtual void validate() const override;
    virtual ostr &print(ostr &o, set &fsym, mset &bsym, unsigned lvl = 0) const override;
};
