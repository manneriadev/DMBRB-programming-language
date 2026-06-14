#include "lexer.hpp"
#include "parser.hpp"
#include "module_loader.hpp"
#include "ASTPrinter.hpp"
#include "analyzer.hpp"
#include "codegen.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

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
        std::cout << "[" << static_cast<int>(token.type) << "] " << "'" << token.lexeme << "' " << "(" << token.line << ":" << token.column << ")" << "\n";
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cerr << "usage: ./my_lang <file.dmb>\n";
        return 1;
    }

    std::string filepath = argv[1];
    if(filepath.size() < 4 || filepath.substr(filepath.size() - 4) != ".dmb")
    {
        std::cerr << "error: file must have .dmb extension\n";
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

        std::string base_dir = fs::path(filepath).parent_path().string();
        if(base_dir.empty()) base_dir = ".";

        ModuleLoader loader(base_dir);
        auto imported = loader.resolve(*program);
        for(int i = (int)imported.size() - 1; i >= 0; --i) program->items.insert(program->items.begin(), std::move(imported[i]));

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

        out.close();

        std::string binary = filepath.substr(0, filepath.size() - 4);
        std::string cmd = "gcc out.c -o " + binary + ".exe" + " -lm";
        int ret = std::system(cmd.c_str());
        if(ret != 0)
        {
            std::cerr << "compilation failed\n";
            return 1;
        }
    }
    catch(const std::exception& ex)
    {
        std::cerr << "fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}