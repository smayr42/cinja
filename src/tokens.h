#pragma once
#include "token.h"

/* clang-format off */

#ifndef DEFINE_TOKEN
#define DEFINE_TOKEN(name, regexp)                                                                 \
    namespace tk_types { extern const tk_type *const name; }
#endif

DEFINE_TOKEN(CONTENT, "(" "[^{]" "|" "\\{[^%{]" ")*");
DEFINE_TOKEN(CODE_BLOCK, "\\{%" "(" "[^%]" "|" "%[^}]" ")*" "%\\}");
DEFINE_TOKEN(VAR_BLOCK, "\\{\\{" "(" "[^}]" "|" "\\}[^}]" ")*" "\\}\\}");

DEFINE_TOKEN(CODE_START, "\\{%");
DEFINE_TOKEN(CODE_END, "%\\}");

DEFINE_TOKEN(VAR_START, "\\{\\{");
DEFINE_TOKEN(VAR_END, "\\}\\}");

DEFINE_TOKEN(WS, "\\s");

DEFINE_TOKEN(FOR, "\\{%" "\\s*" "for");
DEFINE_TOKEN(ENDFOR, "\\{%" "\\s*" "endfor");
DEFINE_TOKEN(IN, "in");
DEFINE_TOKEN(IF, "\\{%" "\\s*" "if");
DEFINE_TOKEN(FILTER, "if");
DEFINE_TOKEN(ENDIF, "\\{%" "\\s*" "endif");
DEFINE_TOKEN(ELIF, "\\{%" "\\s*" "elif");
DEFINE_TOKEN(ELSE, "\\{%" "\\s*" "else");
DEFINE_TOKEN(NOT, "not");
DEFINE_TOKEN(TRUE, "true");
DEFINE_TOKEN(FALSE, "false");
DEFINE_TOKEN(COMMA, ",");
DEFINE_TOKEN(OPENP, "\\(");
DEFINE_TOKEN(CLOSEP, "\\)");
DEFINE_TOKEN(OPENB, "\\[");
DEFINE_TOKEN(CLOSEB, "\\]");
DEFINE_TOKEN(BIN_OP, "\\+|-|\\*|/|==|!=|>|<|>=|<=|and|or|\\.|->");
DEFINE_TOKEN(NUMBER, "[[:digit:]]+(\\.[[:digit:]]+)?");
DEFINE_TOKEN(STRING, "\"[^\"]*\"");
DEFINE_TOKEN(IDENTIFIER, "_*[[:alpha:]]([[:alnum:]]|_)*");
DEFINE_TOKEN(SET, "\\{%" "\\s*" "set");
DEFINE_TOKEN(ASSIGNMENT, "=");
DEFINE_TOKEN(MACRO, "\\{%" "\\s*" "macro");
DEFINE_TOKEN(ENDMACRO, "\\{%" "\\s*" "endmacro");

/* clang-format on */

extern const tk_type_vec BLOCK_TOKENS;
extern const tk_type_vec CODE_TOKENS;
extern const tk_type_vec VAR_TOKENS;
extern const tk_type_vec CODE_DELIMITER;
extern const tk_type_vec VAR_DELIMITER;
