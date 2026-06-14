// analyzer.hpp
#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "ast.hpp" 

struct AnyVariable{
    std::string name;
    Type type;
    bool is_const = false;
    bool is_initialized = false;
    bool is_exported = false;
};

struct AnyFunction{
    std::string name;
    std::string mangled_name;
    std::vector<Type> args;
    Type return_type;
    bool is_exported = false;
};

struct StructField{
    std::string name;
    Type type;
};

struct AnyStruct{
    std::string name;
    std::vector<StructField> fields;
    bool is_exported = false;
};

struct AnyModule{
    std::string name;
    std::unordered_map<std::string, AnyVariable> variables;
    std::unordered_map<std::string, std::vector<AnyFunction>> functions;
    std::unordered_map<std::string, AnyStruct> structs;
    std::unordered_map<std::string, AnyModule> modules;
};

struct VisionPart{
    std::unordered_map<std::string, AnyVariable> variables;
    std::unordered_map<std::string, std::vector<AnyFunction>> functions;
    std::unordered_map<std::string, AnyStruct> structs;
};

class analyzer : public Visitor{

public:
    void analyze(Program& program);
    bool get_status() { return failed; }

private:

    std::vector<VisionPart> visions;
    std::unordered_map<std::string, AnyModule> global_modules;

    Type current_expr_type;
    Type current_self_type;
    FunctionDecl* current_function = nullptr;
    std::string current_module_prefix; // for mem access
    
    bool inside_loop = false;
    bool inside_method = false;
    bool has_return = false;
    int loop_depth = 0;
    
    Type expected_return_type;
    bool failed = false;

    void push_new_vision();
    void pop_last_vision();

    bool declare_variable(VarDecl& node);
    bool declare_function(FunctionDecl& node);
    bool declare_struct(StructDecl& node);
    AnyVariable* resolve_variable(const std::string& name);
    std::vector<AnyFunction>* resolve_function_set(const std::string& name);
    AnyFunction* resolve_overload(std::vector<AnyFunction>* set, const std::vector<Type>& arg_types, const Node* node, const std::string& fname);
    int conversion_cost(const Type& from, const Type& to) const;
    std::string build_module_chain_name(Expr* expr);
    AnyStruct* resolve_struct(const std::string& name);
    AnyModule* resolve_module(const std::string& name);
    AnyModule* resolve_module_chain(Expr* expr);
    bool is_module_name(const std::string& name);

    bool is_lvalue(Expr& expr);
    bool is_none_type(const Type& t);
    bool is_integer(const Type& type) const; 
    bool is_float(const Type& type) const;
    bool is_numeric(const Type& type) const; 
    bool is_bool(const Type& type) const;
    bool is_string(const Type& type) const;
    
    Type visit_expr(Expr& node);
    void error(const std::string& message, const Node* node);

    bool type_equal(const Type& a, const Type& b) const;
    bool can_convert(const Type& from, const Type& to) const;
    bool is_comparable(const Type& a, const Type& b) const;

    //expression

    void visit(AddrOfExpr& node) override;
    void visit(DerefExpr& node) override;
    void visit(LiteralExpr& node) override; 
    void visit(VariableExpr& node) override; 
    void visit(SelfExpr& node) override; 
    void visit(BinaryExpr& node) override; 
    void visit(UnaryExpr& node) override; 
    void visit(AssignmentExpr& node) override; 
    void visit(CallExpr& node) override; 
    void visit(MemberAccessExpr& node) override; 
    void visit(MethodCallExpr& node) override;
    void visit(IndexExpr& node) override; 
    void visit(TernaryExpr& node) override; 
    void visit(CastExpr& node) override; 
    void visit(ArrayLiteralExpr& node) override; 
    void visit(EmptyExpr& node) override;

    // statements 
    
    void visit(BlockStmt& node) override; 
    void visit(ExprStmt& node) override; 
    void visit(IfStmt& node) override; 
    void visit(WhenStmt& node) override; 
    void visit(WhileStmt& node) override; 
    void visit(ForStmt& node) override; 
    void visit(ReturnStmt& node) override; 
    void visit(BreakStmt& node) override; 
    void visit(ContinueStmt& node) override; 
    
    // declarations 
    
    void visit(VarDecl& node) override; 
    void visit(FunctionDecl& node) override; 
    void visit(StructDecl& node) override;
    void visit(ImportDecl&) override;

    void visit(Module& node) override;
    void visit(Program& node) override;
};