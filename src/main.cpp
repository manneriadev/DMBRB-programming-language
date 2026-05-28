#include "lexer.hpp"
#include "parser.hpp"
#include "ASTPrinter.hpp"
#include "analyzer.hpp"
#include "codegen.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

static std::string read_file(const std::string& path)
{
    std::ifstream file(path);
    if(!file.is_open())
    {
        throw std::runtime_error("failed to open file");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

static void dump_tokens(const std::vector<Token>& tokens)
{
    std::cout << "TOKENS:\n\n";
    for(const auto& token : tokens)
        std::cout << "[" << static_cast<int>(token.type) << "] "
                  << "'" << token.lexeme << "' "
                  << "(" << token.line << ":" << token.column << ")" << "\n";
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cerr << "usage: ./my_lang <file.txt>\n";
        return 1;
    }

    std::string filepath = argv[1];
    if(filepath.size() < 4 || filepath.substr(filepath.size() - 4) != ".txt")
    {
        std::cerr << "error: file must have .txt extension\n";
        return 1;
    }

    try
    {
        std::string source = read_file(filepath);

        Lexer lexer(source);
        auto tokens = lexer.Tokenize();
 
        Parser parser(tokens);
        auto program = parser.parse();
        if(!program)
        {
            std::cerr << "parser failed\n";
            return 1;
        }

        analyzer sem;
        sem.analyze(*program);
        if(sem.get_status() == true) return 1;

        std::ofstream out("out.c");

        if(!out.is_open())
        {
            std::cerr << "failed to create out.c\n";
            return 1;
        }

        Codegen cg(out);
        cg.generate(*program);

        std::cout << "\nC code generated -> out.c\n";
    }
    catch(const std::exception& ex)
    {
        std::cerr << "fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}