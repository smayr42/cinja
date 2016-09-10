/* clang-format off */

#define DEFINE_TOKEN(name, regexp)                                                               \
    namespace tk_types { extern const tk_type *const name; }                                     \
    static const tk_type name##_(#name, regexp);                                                 \
    const tk_type_ptr tk_types::name = &name##_;

/* clang-format on */

#include "tokens.h"

const tk_type_vec BLOCK_TOKENS{tk_types::CODE_BLOCK, tk_types::VAR_BLOCK, tk_types::CONTENT};

const tk_type_vec CODE_TOKENS{
    tk_types::FOR,        tk_types::ENDFOR, tk_types::IN,     tk_types::IF,
    tk_types::FILTER,     tk_types::ENDIF,  tk_types::ELIF,   tk_types::ELSE,
    tk_types::NOT,        tk_types::TRUE,   tk_types::FALSE,  tk_types::COMMA,
    tk_types::OPENP,      tk_types::CLOSEP, tk_types::OPENB,  tk_types::CLOSEB,
    tk_types::BIN_OP,     tk_types::NUMBER, tk_types::STRING, tk_types::IDENTIFIER,
    tk_types::ASSIGNMENT, tk_types::SET,    tk_types::MACRO,  tk_types::ENDMACRO};

const tk_type_vec CODE_DELIMITER{tk_types::CODE_END, tk_types::WS};

const tk_type_vec VAR_TOKENS{tk_types::VAR_START, tk_types::VAR_END,    tk_types::NOT,
                             tk_types::TRUE,      tk_types::FALSE,      tk_types::OPENP,
                             tk_types::CLOSEP,    tk_types::BIN_OP,     tk_types::NUMBER,
                             tk_types::STRING,    tk_types::IDENTIFIER, tk_types::COMMA};

const tk_type_vec VAR_DELIMITER{tk_types::WS};
