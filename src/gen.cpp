#include "ast.h"
#include <iostream>
#include <map>
#include <set>
#include <sstream>

template <typename T> static void insert_sym(T &syms, std::string sym) { syms.insert(sym); }

template <typename T> static void erase_sym(T &syms, std::string sym)
{
    const auto it(syms.find(sym));

    if (it != syms.end())
        syms.erase(it);
}

template <typename T> static bool contains_sym(T &syms, std::string sym)
{
    return (syms.find(sym) != syms.end());
}

template <typename T, typename F>
Node::ostr &join(Node::ostr &o, const T &collection, const std::string &sep, F print)
{
    auto it(collection.begin());
    const auto end(collection.end());

    if (it == end)
        return o;

    print(o, *it, 0);

    size_t i = 0;
    while (++it != end) {
        o << sep;
        print(o, *it, ++i);
    }

    return o;
}

static inline std::string indent(unsigned lvl) { return std::string(lvl, '\t'); }

Node::ostr &ForNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    o << indent(lvl) << "for (const auto& ";

    var->print(o, fsym, bsym) << " : ";
    collection->print(o, fsym, bsym) << ") {\n";

    insert_sym(bsym, var->name);
    o << indent(lvl + 1) << "if (";
    filter->print(o, fsym, bsym) << ") {\n";
    body->print(o, fsym, bsym, lvl + 2);
    o << indent(lvl + 1) << "}\n";
    o << indent(lvl) << "}\n";
    erase_sym(bsym, var->name);

    return o;
}

Node::ostr &IfNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    o << indent(lvl) << "if (";
    condition->print(o, fsym, bsym, lvl) << ") {\n";
    body->print(o, fsym, bsym, lvl + 1);
    o << indent(lvl) << "} else {\n";
    elze->print(o, fsym, bsym, lvl + 1);
    return o << indent(lvl) << "}\n";
}

template <> Node::ostr &LiteralNode<bool>::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    if (value)
        return o << "true";
    else
        return o << "false";
}

template <>
Node::ostr &LiteralNode<std::string>::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    return o << "std::string(\"" << value.c_str() << "\")";
}

Node::ostr &FieldNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    return o << name;
}

Node::ostr &IdNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    if (binding)
        return o << name;

    std::string fname(full_name());

    if (!contains_sym(bsym, fname))
        insert_sym(fsym, fname);

    return o << fname;
}

Node::ostr &UnOpNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    static std::map<UnOp, std::string> str_map{{UnOp::NEG, "-"}, {UnOp::NOT, "!"}};

    o << str_map[op] << "(";
    return arg->print(o, fsym, bsym) << ")";
}

Node::ostr &BinOpNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    static std::map<BinOp, std::string> str_map{
        {BinOp::ADD, "+"}, {BinOp::SUB, "-"},   {BinOp::MUL, "*"},  {BinOp::DIV, "/"},
        {BinOp::EQ, "=="}, {BinOp::NEQ, "!="},  {BinOp::GT, ">"},   {BinOp::GE, ">="},
        {BinOp::LT, "<"},  {BinOp::LE, "<="},   {BinOp::AND, "&&"}, {BinOp::OR, "||"},
        {BinOp::DOT, "."}, {BinOp::ARROW, "->"}};

    std::string pad(" ");

    if (op == BinOp::DOT || op == BinOp::ARROW)
        pad = "";

    o << "(";
    lhs->print(o, fsym, bsym) << pad << str_map[op] << pad;
    return rhs->print(o, fsym, bsym) << ")";
}

Node::ostr &print_macro_proto(Node::ostr &o, const nptr<MacroNode> &m, unsigned lvl)
{
    o << indent(lvl) << "template<typename O, ";
    join(o, m->args, ", ", [](auto &o, auto &v, auto i) { o << "typename T" << i; });
    o << ">\n";

    o << indent(lvl) << "void " << m->id->name << "(O &o, ";
    join(o, m->args, ", ", [](auto &o, auto &v, auto i) { o << "T" << i << " " << v->id->name; });
    o << ")";

    return o;
}

Node::ostr &TemplateNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    o << "#include <iostream>\n"
      << "#include <string>\n\n"
      << "namespace macros {\n";

    join(o, this->macros, "", [&](auto &o, auto &v, auto i) {
        print_macro_proto(o, v, lvl + 1) << ";\n\n";
        insert_sym(bsym, v->id->full_name());
    });

    join(o, this->macros, "\n", [&](auto &o, auto &v, auto i) {
        set fsym;

        print_macro_proto(o, v, 1) << " {\n";

        for (auto &arg : v->args)
            insert_sym(bsym, arg->id->name);

        v->body->print(o, fsym, bsym, lvl + 2);

        for (auto &arg : v->args)
            erase_sym(bsym, arg->id->name);

        o << indent(lvl + 1) << "}\n";

        for (const auto &sym : fsym)
            std::cerr << "Unknown symbol '" << sym << "' in macro '" << v->id->name << "'\n";
    });

    o << "}\n\n";

    std::ostringstream body;
    this->body->print(body, fsym, bsym, lvl + 1);

    o << "template<typename OS, ";
    join(o, fsym, ", ", [](auto &o, auto &v, auto i) { o << "typename N" << i; });
    o << ">\n";

    o << "void render_template(OS &o, ";
    join(o, fsym, ", ", [](auto &o, auto &v, auto i) { o << "N" << i << " " << v; });
    o << ") {\n";

    o << body.str();
    o << "}\n";
    return o;
}

Node::ostr &SetNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    o << indent(lvl) << "{\n" << indent(lvl + 1);

    if (!contains_sym(bsym, var->name))
        o << "auto ";

    var->print(o, fsym, bsym, lvl) << " = ";
    value->print(o, fsym, bsym, lvl) << ";\n";

    insert_sym(bsym, var->name);
    body->print(o, fsym, bsym, lvl + 1);
    erase_sym(bsym, var->name);

    return o << indent(lvl) << "}\n";
}

Node::ostr &ListNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    o << "{";

    for (const auto &node : this->values) {
        node->print(o, fsym, bsym, lvl);
        if (node != this->values.back())
            o << ", ";
    }

    o << "}";
    return o;
}

Node::ostr &StmtListNode::print(ostr &o, StmtListNode::set &fsym, StmtListNode::mset &bsym,
                                unsigned lvl) const
{
    for (const auto &stmt : stmts)
        stmt->print(o, fsym, bsym, lvl);

    return o;
}

Node::ostr &ContentNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    return o << indent(lvl) << "o << u8R\"content\"\"\"(" << content << ")content\"\"\"\";\n";
}

Node::ostr &VarNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    o << indent(lvl) << "o << ";
    return expr->print(o, fsym, bsym) << ";\n";
}

Node::ostr &ArgumentNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    this->id->print(o, fsym, bsym, lvl);

    if (this->dflt) {
        o << " = ";
        this->dflt->print(o, fsym, bsym, lvl);
    }

    return o;
}

Node::ostr &MacroNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const { return o; }

Node::ostr &CallNode::print(ostr &o, set &fsym, mset &bsym, unsigned lvl) const
{
    o << indent(lvl);
    this->id->print(o, fsym, bsym, lvl) << "(o, ";
    join(o, this->args, ", ", [&](auto &o, auto &v, auto i) { v->print(o, fsym, bsym, 0); });
    o << ");\n";
    return o;
}
