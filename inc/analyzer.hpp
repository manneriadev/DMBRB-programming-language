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
};

struct AnyFunction{
    std::string name;
    std::vector<Type> args;
    Type return_type;
};

struct StructField{
    std::string name;
    Type type;
};

struct AnyStruct{
    std::string name;
    std::vector<StructField> fields;
};

class analyzer : public Visitor{

public:
    void analyze(Program& program);

private:

    std::vector<std::unordered_map<std::string, AnyVariable>> variable_vision;
    std::unordered_map<std::string , AnyFunction> functions;
    std::unordered_map<std::string , AnyStruct> structs;

    FunctionDecl* current_func = nullptr;
    bool inside_loop = false;

    bool has_return = false;
    Type expected_return_type;


    void push_new_vision();
    void pop_last_vision();

    bool declare_variable(const AnyVariable variable);
    bool declare_function(const AnyFunction variable);
    bool declare_struct(const AnyStruct variable);
    AnyVariable* resolve_variable(const std::string& name);
    AnyFunction* resolve_function(const std::string& name);
    AnyStruct* resolve_struct(const std::string& name);

    bool type_equal(const Type& a, const Type& b) const; 
    bool is_integer(const Type& type) const; 
    bool is_float(const Type& type) const; 
    bool is_numeric(const Type& type) const; 
    bool is_bool(const Type& type) const;
    bool is_string(const Type& type) const;
    
    Type visit_expr(Expr& node);
    void error(const std::string& message);

    //expression

    void visit(LiteralExpr& node) override; 
    void visit(VariableExpr& node) override; 
    void visit(BinaryExpr& node) override; 
    void visit(UnaryExpr& node) override; 
    void visit(AssignmentExpr& node) override; 
    void visit(CallExpr& node) override; 
    void visit(MemberAccessExpr& node) override; 
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
};