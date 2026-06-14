//module_loader.hpp

#pragma once
#include "ast.hpp"
#include <string>
#include <unordered_set>
#include <vector>
#include <memory>

class ModuleLoader
{
public:
    explicit ModuleLoader(const std::string& base_dir);
    std::vector<std::unique_ptr<Module>> resolve(Program& program);

private:
    std::string base_dir;
    std::unordered_set<std::string> loaded; // protect from double import

    std::unique_ptr<Module> load_file(const std::string& path, const std::string& alias);
    std::string resolve_path(const std::string& import_path);
};