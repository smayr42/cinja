#include "lexer.h"
#include "tokens.h"
#include <iostream>
#include <regex>

const tk tk_iterator::end_of_input(tk_types::EOI, "", 0);

static void tokenize(const std::string &string, tk_vec &tokens, const tk_type_vec &accept,
                     const tk_type_vec &ignore = tk_type_vec(), long line_no = 0)
{
    using namespace std;
    using namespace std::regex_constants;

    const auto flags = match_not_null | match_continuous;
    auto it = string.begin();
    auto end = string.end();
    bool found;
    smatch match;

    while (it != end) {
        found = false;

        for (const auto &type : ignore) {
            if (std::regex_search(it, end, match, type->regexp(), flags)) {
                const auto &str = match.str();
                line_no += std::count(str.begin(), str.end(), '\n');
                it += match.length();
                found = true;
                break;
            }
        }

        if (found)
            continue;

        tk_type_ptr token_type;
        std::string token_value;

        for (const auto &type : accept) {
            if (!std::regex_search(it, end, match, type->regexp(), flags))
                continue;

            if (token_value.length() < match.str().length()) {
                token_value = match.str();
                token_type = type;
                found = true;
            }
        }

        if (found) {
            tokens.emplace_back(token_type, token_value, line_no);
            it += token_value.length();
        } else {
            tokens.emplace_back(tk_types::UNKNOWN, std::string(1, *it), line_no);
            ++it;
        }

        line_no += tokens.back().num_lines();
    }
}

bool tokenize_template(const std::string &content, tk_vec &tokens)
{
    std::vector<tk> blocks;
    tokenize(content, blocks, BLOCK_TOKENS);

    for (const auto &block : blocks) {
        if (block.type() == tk_types::CONTENT)
            tokens.push_back(block);
        else if (block.type() == tk_types::CODE_BLOCK)
            tokenize(block.value(), tokens, CODE_TOKENS, CODE_DELIMITER, block.start_line());
        else if (block.type() == tk_types::VAR_BLOCK)
            tokenize(block.value(), tokens, VAR_TOKENS, VAR_DELIMITER, block.start_line());
        else
            return false;
    }

    return true;
}
