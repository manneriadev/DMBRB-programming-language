//module_loader.cpp

#include "module_loader.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
namespace fs = std::filesystem;

ModuleLoader::ModuleLoader(const std::string& base_dir) : base_dir(base_dir) {}

std::string ModuleLoader::resolve_path(const std::string& p)
{
    fs::path full = fs::path(base_dir) / p;
    if(fs::exists(full)) return full.string();
    full = fs::path(base_dir) / (p + ".dmb");
    if(fs::exists(full)) return full.string();
    throw std::runtime_error("cannot find module: " + p);
}

std::unique_ptr<Module> ModuleLoader::load_file(const std::string& path, const std::string& alias)
{
    std::string canonical = fs::canonical(path).string(); // delete all . near /
    if(loaded.count(canonical)) return nullptr; // try to find in set
    loaded.insert(canonical);

    std::ifstream file(path);
    if(!file.is_open()) throw std::runtime_error("cannot open: " + path);
    std::stringstream buf; // make string stream on ram as std::string and write all data from file buffer inside
    buf << file.rdbuf();

    Lexer lexer(buf.str());
    auto tokens = lexer.Tokenize();
    Parser parser(tokens);
    auto prog = parser.parse();
    if(!prog) throw std::runtime_error("parse failed: " + path);

    auto mod = std::make_unique<Module>();
    mod->name = alias.empty() ? fs::path(path).stem().string() : alias; // transform string path to object pass and after this delete all .exe .dmd or same and transform back to string

    for(auto& item : prog->items)
    {
        if(auto* imp = dynamic_cast<ImportDecl*>(item.get()))
        {
            std::string sub_path = resolve_path(imp->path);
            std::string sub_alias = imp->alias.empty() ? fs::path(sub_path).stem().string() : imp->alias;
            auto sub = load_file(sub_path, sub_alias);
            if(sub) mod->submodules.push_back(std::move(sub));
        }
        else if(auto* decl = dynamic_cast<Decl*>(item.get()))
        {
            item.release();
            mod->decls.push_back(std::unique_ptr<Decl>(decl));
        }
    }
    return mod;
}

std::vector<std::unique_ptr<Module>> ModuleLoader::resolve(Program& program)
{
    std::vector<std::unique_ptr<Module>> result;
    std::vector<std::unique_ptr<Node>> remaining;

    for(auto& item : program.items)
    {
        if(auto* imp = dynamic_cast<ImportDecl*>(item.get()))
        {
            std::string p = resolve_path(imp->path);
            std::string a = imp->alias.empty() ? fs::path(p).stem().string() : imp->alias;
            auto mod = load_file(p, a);
            if(mod) result.push_back(std::move(mod));
        }
        else remaining.push_back(std::move(item));
    }
    program.items = std::move(remaining);
    return result;
}