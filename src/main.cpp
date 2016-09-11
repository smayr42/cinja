#include "lexer.h"
#include "parser.h"
#include "validate.h"
#include <fstream>
#include <unistd.h>

static void print_help(std::ostream &ostream, const std::string &prog)
{
    ostream << "Usage:\n\t" << prog << " FLAGS FILE\n";
    ostream << "FLAGS:\n";
    ostream << "\t-o out  Write output to the given file\n";
    ostream << "\t-h      Display this message\n";
}

int main(int argc, char *argv[])
{
    using namespace std;

    ostream *out = &cout;
    istream *in = &cin;
    ifstream in_file;
    ofstream out_file;
    in_file.exceptions(ios::failbit | ios::badbit);
    out_file.exceptions(ios::failbit | ios::badbit);

    try {
        int param;
        while ((param = getopt(argc, argv, "ho:")) != -1) {
            switch (param) {
            case '?':
                print_help(cerr, argv[0]);
                return EXIT_FAILURE;
            case 'h':
                print_help(cout, argv[0]);
                return EXIT_SUCCESS;
            case 'o':
                out_file.open(optarg, fstream::out);
                out = &out_file;
                break;
            }
        }

        if (optind < argc) {
            in_file.open(argv[optind]);
            in = &in_file;
        }

        string content((istreambuf_iterator<char>(*in)), istreambuf_iterator<char>());

        tk_vec tokens = tokenize_template(content);
        tk_iterator it(tokens.begin(), tokens.end());

        nptr<> root = parse_template(it);
        root->validate();
        root->print(*out) << endl;

    } catch (const system_error &e) {
        cerr << "Error: " << e.code().message() << endl;
        return EXIT_FAILURE;

    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
