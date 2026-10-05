#include "Lexer.hpp"
#include "parser.hpp"
#include "module_loader.hpp"
#include "ASTPrinter.hpp"
#include "analyzer.hpp"
#include "codegen.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const char* VERSION = "0.1.0";

#ifdef _WIN32
static const char* NULL_DEVICE = "NUL";
#else
static const char* NULL_DEVICE = "/dev/null";
#endif

static std::string read_file(const std::string& path)
{
    std::ifstream file(path);
    if(!file.is_open())
    {
        throw std::runtime_error("cannot open file '" + path + "'");
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

static void print_usage(std::ostream& os)
{
    os << "DMBRB compiler " << VERSION << "\n\n"
       << "usage: dmbrb <file.dmb> [options]\n\n"
       << "options:\n"
       << "  -o <name>       output executable name (default: input name without .dmb)\n"
       << "  --emit-c        keep the generated C file next to the executable\n"
       << "  --dump-tokens   print lexer tokens and exit\n"
       << "  -h, --help      show this help\n"
       << "  -v, --version   show version\n\n"
       << "The generated C code is compiled with gcc (override with the CC environment variable).\n";
}

static std::string compiler_command()
{
    const char* cc = std::getenv("CC");
    return (cc && *cc) ? std::string(cc) : std::string("gcc");
}

static bool compiler_available(const std::string& cc)
{
    std::string cmd = cc + " --version > " + NULL_DEVICE + " 2>&1";
    return std::system(cmd.c_str()) == 0;
}

// Removes the temporary C file when it goes out of scope (unless --emit-c)
struct TempFile
{
    fs::path path;
    bool keep;
    ~TempFile()
    {
        if(!keep)
        {
            std::error_code ec;
            fs::remove(path, ec);
        }
    }
};

int main(int argc, char** argv)
{
    std::string input;
    std::string output;
    bool emit_c = false;
    bool dump = false;

    for(int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if(arg == "-h" || arg == "--help")
        {
            print_usage(std::cout);
            return 0;
        }
        else if(arg == "-v" || arg == "--version")
        {
            std::cout << "dmbrb " << VERSION << "\n";
            return 0;
        }
        else if(arg == "--emit-c") emit_c = true;
        else if(arg == "--dump-tokens") dump = true;
        else if(arg == "-o")
        {
            if(i + 1 >= argc)
            {
                std::cerr << "error: -o requires a name\n";
                return 1;
            }
            output = argv[++i];
        }
        else if(!arg.empty() && arg[0] == '-')
        {
            std::cerr << "error: unknown option '" << arg << "' (see --help)\n";
            return 1;
        }
        else if(input.empty()) input = arg;
        else
        {
            std::cerr << "error: only one input file is supported\n";
            return 1;
        }
    }

    if(input.empty())
    {
        print_usage(std::cerr);
        return 1;
    }

    if(fs::path(input).extension() != ".dmb")
    {
        std::cerr << "error: file must have .dmb extension\n";
        return 1;
    }

    try
    {
        std::string source = read_file(input);

        Lexer lexer(source);
        auto tokens = lexer.Tokenize();

        if(dump)
        {
            dump_tokens(tokens);
            return 0;
        }

        Parser parser(tokens);
        auto program = parser.parse();
        if(!program)
        {
            std::cerr << "parser failed\n";
            return 1;
        }

        std::string base_dir = fs::path(input).parent_path().string();
        if(base_dir.empty()) base_dir = ".";

        ModuleLoader loader(base_dir);
        auto imported = loader.resolve(*program);
        for(int i = (int)imported.size() - 1; i >= 0; --i) program->items.insert(program->items.begin(), std::move(imported[i]));

        analyzer sem;
        sem.analyze(*program);
        if(sem.get_status() == true) return 1;

        std::string cc = compiler_command();
        if(!compiler_available(cc))
        {
            std::cerr << "error: C compiler '" << cc << "' not found.\n"
                      << "Install gcc (MSYS2 / MinGW-w64 on Windows, build-essential on Linux)\n"
                      << "and make sure it is in your PATH.\n";
            return 1;
        }

        fs::path out_path = output.empty() ? fs::path(input).replace_extension() : fs::path(output);
#ifdef _WIN32
        if(!out_path.has_extension()) out_path += ".exe";
#endif

        fs::path c_path;
        if(emit_c)
        {
            c_path = out_path;
            c_path.replace_extension(".c");
        }
        else
        {
            auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
            c_path = fs::temp_directory_path() / ("dmbrb_" + std::to_string(stamp) + ".c");
        }
        TempFile tmp{c_path, emit_c};

        {
            std::ofstream out(c_path);
            if(!out.is_open())
            {
                std::cerr << "error: cannot create '" << c_path.string() << "'\n";
                return 1;
            }
            Codegen cg(out);
            cg.generate(*program);
        }

        std::string cmd = cc + " \"" + c_path.string() + "\" -o \"" + out_path.string() + "\" -lm";
        int ret = std::system(cmd.c_str());
        if(ret != 0)
        {
            std::cerr << "error: C compilation of generated code failed\n";
            return 1;
        }

        std::cout << "Built " << out_path.string() << "\n";
    }
    catch(const std::exception& ex)
    {
        std::cerr << "fatal error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
