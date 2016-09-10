#include "parser.h"
#include "tokens.h"
#include <cassert>
#include <vector>

static std::string symbol_prefix("vsym");
static std::string macro_namespace("macros");

const char *ParseException::what() const noexcept
{
    if (!what_.empty())
        return what_.c_str();

    std::stringstream s;
    s << "unexpected token '" << token_.value() << "' (" << token_.type()->name() << ") on line "
      << (token_.start_line() + 1);
    if (expected_.size() == 1) {
        s << ", expected " << expected_[0]->name();
    } else if (expected_.size() > 1) {
        s << ", expected one of " << expected_[0]->name();
        for (auto it = expected_.begin() + 1, end = expected_.end(); it != end; ++it)
            s << ", " << (*it)->name();
    }

    what_ = s.str();
    return what_.c_str();
}

enum class Associativity { LEFT, RIGHT };

static Associativity binop_associativity(const BinOp op)
{
    switch (op) {
    default:
        return Associativity::LEFT;
    }
}

static unsigned unop_precedence(const UnOp op)
{
    switch (op) {
    default:
        return 4;
    }
}

static unsigned binop_precedence(const tk &token, BinOp &op)
{
    assert(token.type() == tk_types::BIN_OP);

    static std::map<std::string, BinOp> binop{
        {"-", BinOp::SUB}, {"+", BinOp::ADD},   {"*", BinOp::MUL},   {"/", BinOp::DIV},
        {"<", BinOp::LT},  {">", BinOp::GT},    {"<=", BinOp::LE},   {">=", BinOp::GE},
        {"==", BinOp::EQ}, {"!=", BinOp::NEQ},  {"and", BinOp::AND}, {"or", BinOp::OR},
        {".", BinOp::DOT}, {"->", BinOp::ARROW}};

    op = binop[token.value()];

    switch (op) {
    case BinOp::AND:
    case BinOp::OR:
        return 0;

    case BinOp::EQ:
    case BinOp::NEQ:
    case BinOp::GT:
    case BinOp::GE:
    case BinOp::LT:
    case BinOp::LE:
        return 1;

    case BinOp::ADD:
    case BinOp::SUB:
        return 2;

    case BinOp::MUL:
    case BinOp::DIV:
        return 3;

    case BinOp::DOT:
    case BinOp::ARROW:
        return 5;
    }

    return 0;
}

template <typename T, typename... Args> static std::unique_ptr<T> make_node(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

static void match(tk_iterator &it, tk_type_ptr tk_type, bool consume = true)
{
    if (it->type() != tk_type)
        throw ParseException(*it, {tk_type});

    if (consume)
        ++it;
}

template <typename T>
static std::vector<nptr<T>>
parse_list(tk_iterator &it, std::function<nptr<T>(tk_iterator &)> parse_entry, tk_type_ptr begin,
           tk_type_ptr end, tk_type_ptr sep = tk_types::COMMA)
{
    std::vector<nptr<T>> list;

    match(it, begin);
    while (it->type() != end) {
        list.push_back(parse_entry(it));

        if (it->type() != sep)
            break;

        match(it, sep);
    }
    match(it, end);

    return list;
}

static nptr<IdNode> parse_var_id(tk_iterator &it)
{
    long line = it->start_line();
    match(it, tk_types::IDENTIFIER, false);
    return make_node<IdNode>(symbol_prefix, (it++)->value(), line);
}

static nptr<IdNode> parse_bvar_id(tk_iterator &it)
{
    long line = it->start_line();
    match(it, tk_types::IDENTIFIER, false);
    return make_node<IdNode>(symbol_prefix, (it++)->value(), line, true);
}

static nptr<IdNode> parse_macro_id(tk_iterator &it)
{
    long line = it->start_line();
    match(it, tk_types::IDENTIFIER, false);
    return make_node<IdNode>((it++)->value(), line, macro_namespace);
}

static nptr<IdNode> parse_bmacro_id(tk_iterator &it)
{
    long line = it->start_line();
    match(it, tk_types::IDENTIFIER, false);
    return make_node<IdNode>((it++)->value(), line, macro_namespace, true);
}

static nptr<ExprNode> parse_expr(tk_iterator &it, unsigned min_precedence);
static nptr<ExprNode> parse_rexpr(tk_iterator &it) { return parse_expr(it, 0); }

static nptr<ExprNode> parse_atom(tk_iterator &it)
{
    if (it->type() == tk_types::OPENP) {
        auto expr = parse_rexpr(++it);
        match(it, tk_types::CLOSEP);
        return expr;

    } else if (it->type() == tk_types::NOT) {
        auto unop = make_node<UnOpNode>();
        unop->op = UnOp::NOT;
        unop->arg = parse_expr(++it, unop_precedence(UnOp::NOT));
        return std::move(unop);

    } else if (it->type() == tk_types::BIN_OP && it->value() == "-") {
        auto unop = make_node<UnOpNode>();
        unop->op = UnOp::NEG;
        unop->arg = parse_expr(++it, unop_precedence(UnOp::NEG));
        return std::move(unop);

    } else if (it->type() == tk_types::NUMBER) {
        double val = atof(it->value().c_str());
        auto num = make_node<LiteralNode<double>>(val, (it++)->start_line());
        return std::move(num);

    } else if (it->type() == tk_types::TRUE) {
        long line = (it++)->start_line();
        return make_node<LiteralNode<bool>>(true, line);

    } else if (it->type() == tk_types::FALSE) {
        long line = (it++)->start_line();
        return make_node<LiteralNode<bool>>(false, line);

    } else if (it->type() == tk_types::STRING) {
        long line = it->start_line();
        const auto &str = (it++)->value();
        return make_node<LiteralNode<std::string>>(str.substr(1, str.length() - 2), line);

    } else if (it->type() == tk_types::IDENTIFIER) {
        return parse_var_id(it);

    } else if (it->type() == tk_types::OPENB) {
        return make_node<ListNode>(
            parse_list<ExprNode>(it, parse_rexpr, tk_types::OPENB, tk_types::CLOSEB));
    }

    throw ParseException(*it, {tk_types::OPENP, tk_types::NOT, tk_types::BIN_OP, tk_types::NUMBER,
                               tk_types::TRUE, tk_types::FALSE});
}

static nptr<FieldNode> parse_field(tk_iterator &it)
{
    match(it, tk_types::IDENTIFIER, false);
    long line = it->start_line();
    return make_node<FieldNode>((it++)->value(), line);
}

/* parse expressions with the precedence climbing technique */
static nptr<ExprNode> parse_expr(tk_iterator &it, unsigned min_precedence)
{
    auto result = parse_atom(it);

    BinOp op;
    unsigned precedence;

    while (it->type() == tk_types::BIN_OP &&
           (precedence = binop_precedence(*it, op)) >= min_precedence) {

        Associativity assoc = binop_associativity(op);
        nptr<ExprNode> rhs;

        if (op == BinOp::DOT || op == BinOp::ARROW)
            rhs = parse_field(++it);
        else if (assoc == Associativity::LEFT)
            rhs = parse_expr(++it, precedence + 1);
        else
            rhs = parse_expr(++it, precedence);

        auto tmp = make_node<BinOpNode>();
        tmp->op = op;
        tmp->lhs = std::move(result);
        tmp->rhs = std::move(rhs);
        result = std::move(tmp);
    }

    return result;
}

static nptr<StmtNode> parse_statement(tk_iterator &it);
static nptr<StmtListNode> parse_statement_list(tk_iterator &it)
{
    auto node = make_node<StmtListNode>();

    nptr<StmtNode> stmt;
    while ((stmt = parse_statement(it)))
        node->stmts.push_back(std::move(stmt));

    return node;
}

static nptr<IfNode> parse_if(tk_iterator &it)
{
    auto n = make_node<IfNode>();

    match(it, tk_types::IF);
    n->condition = parse_rexpr(it);
    n->body = parse_statement_list(it);

    auto tail = n.get();
    while (it->type() == tk_types::ELIF) {
        auto elif = make_node<IfNode>();
        auto elif_ptr = elif.get();

        elif->condition = parse_rexpr(++it);
        elif->body = parse_statement_list(it);

        tail->elze = make_node<StmtListNode>();
        tail->elze->stmts.push_back(std::move(elif));
        tail = elif_ptr;
    }

    if (it->type() == tk_types::ELSE)
        tail->elze = parse_statement_list(++it);
    else
        tail->elze = make_node<StmtListNode>();

    match(it, tk_types::ENDIF);
    return n;
}

static nptr<StmtNode> parse_interpolation(tk_iterator &it)
{
    match(it, tk_types::VAR_START);

    if (it->type() == tk_types::IDENTIFIER && std::next(it)->type() == tk_types::OPENP) {
        auto call = make_node<CallNode>();

        call->id = parse_macro_id(it);
        call->args = parse_list<ExprNode>(it, parse_rexpr, tk_types::OPENP, tk_types::CLOSEP);

        match(it, tk_types::VAR_END);
        return std::move(call);
    } else {
        auto var = make_node<VarNode>();
        var->expr = parse_rexpr(it);
        match(it, tk_types::VAR_END);
        return std::move(var);
    }
}

static nptr<ArgumentNode> parse_arg(tk_iterator &it)
{
    auto node = make_node<ArgumentNode>();
    node->id = parse_bvar_id(it);

    if (it->type() == tk_types::ASSIGNMENT) {
        match(it, tk_types::ASSIGNMENT);
        node->dflt = parse_rexpr(it);
    }

    return node;
}

static nptr<StmtNode> parse_statement(tk_iterator &it)
{
    if (it->type() == tk_types::CONTENT) {
        auto n = make_node<ContentNode>();
        n->content = (it++)->value();
        return std::move(n);

    } else if (it->type() == tk_types::IF) {
        return parse_if(it);

    } else if (it->type() == tk_types::FOR) {
        auto n = make_node<ForNode>();

        n->var = parse_bvar_id(++it);
        match(it, tk_types::IN);
        n->collection = parse_rexpr(it);

        if (it->type() == tk_types::FILTER)
            n->filter = parse_rexpr(++it);
        else
            n->filter = make_node<LiteralNode<bool>>(true, it->start_line());

        n->body = parse_statement_list(it);
        match(it, tk_types::ENDFOR);
        return std::move(n);

    } else if (it->type() == tk_types::VAR_START) {
        return parse_interpolation(it);

    } else if (it->type() == tk_types::SET) {
        auto n = make_node<SetNode>();

        n->var = parse_bvar_id(++it);
        match(it, tk_types::ASSIGNMENT);
        n->value = parse_rexpr(it);
        n->body = parse_statement_list(it);

        return std::move(n);
    }

    return nullptr;
}

static nptr<StmtListNode> parse_rstatement_list(tk_iterator &it,
                                                std::vector<nptr<MacroNode>> &macros)
{
    auto stmts = make_node<StmtListNode>();

    for (;;) {
        if (it->type() == tk_types::MACRO) {
            auto m = make_node<MacroNode>();

            m->id = parse_bmacro_id(++it);
            m->args = parse_list<ArgumentNode>(it, parse_arg, tk_types::OPENP, tk_types::CLOSEP);
            m->body = parse_statement_list(it);

            match(it, tk_types::ENDMACRO);
            macros.push_back(std::move(m));
        } else {
            nptr<StmtNode> stmt = parse_statement(it);

            if (stmt)
                stmts->stmts.push_back(std::move(stmt));
            else
                break;
        }
    }

    return stmts;
}

nptr<> parse_template(tk_iterator &it)
{
    auto root = make_node<TemplateNode>();
    root->body = parse_rstatement_list(it, root->macros);
    match(it, tk_types::EOI);
    return std::move(root);
}
