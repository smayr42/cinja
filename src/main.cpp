#include "lexer.h"
#include "parser.h"
#include "validate.h"
#include <fstream>
#include <unistd.h>

void print_help(std::ostream &ostream, char *prog_name)
{
    ostream << "Usage:\n\t" << prog_name << " FLAGS FILE\n";
    ostream << "FLAGS:\n";
    ostream << "\t-o out  Write output to the given file\n";
    ostream << "\t-h      Display this message\n";
}

int main(int argc, char *argv[])
{
    std::ostream *out = &std::cout;
    std::istream *in = &std::cin;
    std::ifstream in_file;
    std::ofstream out_file;

    int param;
    while ((param = getopt(argc, argv, "ho:")) != -1) {
        switch (param) {
        case '?':
            print_help(std::cerr, argv[0]);
            return EXIT_FAILURE;
        case 'h':
            print_help(std::cout, argv[0]);
            return EXIT_SUCCESS;
        case 'o':
            out_file.open(optarg, std::fstream::out);
            out = &out_file;
            break;
        }
    }

    if (optind < argc) {
        in_file.open(argv[optind]);
        in = &in_file;
    }

    std::string content((std::istreambuf_iterator<char>(*in)), std::istreambuf_iterator<char>());

    tk_vec tokens;
    tokenize_template(content, tokens);

    tk_iterator it(tokens.begin(), tokens.end());
    nptr<Node> root;
    try {
        root = parse_template(it);
    } catch (ParseException &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    try {
        root->validate();
    } catch (InvalidTypeException &e) {
        Node::set fsym;
        Node::mset bsym;
        root->print(std::cerr, fsym, bsym, 0) << std::endl;
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    root->print(*out) << std::endl;
    return EXIT_SUCCESS;
}
