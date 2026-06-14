#pragma once

#include "ast.hpp"

#include <iostream>
#include <string>

class ASTPrinter : public Visitor{
public:
    explicit ASTPrinter(std::ostream& out = std::cout);

    void print(Program& program);

    // expressions
    void visit(AddrOfExpr&) override;
    void visit(DerefExpr&) override;
    void visit(LiteralExpr&) override;
    void visit(VariableExpr&) override;
    void visit(SelfExpr&) override;
    void visit(BinaryExpr&) override;
    void visit(UnaryExpr&) override;
    void visit(AssignmentExpr&) override;
    void visit(MethodCallExpr&) override;
    void visit(CallExpr&) override;
    void visit(MemberAccessExpr&) override;
    void visit(IndexExpr&) override;
    void visit(TernaryExpr&) override;
    void visit(CastExpr&) override;
    void visit(ArrayLiteralExpr&) override;
    void visit(EmptyExpr&) override;

    // statements
    void visit(BlockStmt&) override;
    void visit(ExprStmt&) override;
    void visit(IfStmt&) override;
    void visit(WhenStmt&) override;
    void visit(WhileStmt&) override;
    void visit(ForStmt&) override;
    void visit(ReturnStmt&) override;
    void visit(BreakStmt&) override;
    void visit(ContinueStmt&) override;

    // declarations
    void visit(VarDecl&) override;
    void visit(FunctionDecl&) override;
    void visit(StructDecl&) override;
    void visit(ImportDecl&) override;

    void visit(Module&) override;
    void visit(Program&) override;

private:
    std::ostream& out;

    size_t current_space = 0;
    size_t current_value = 0;

private:
    void print_prefix();

    void print_node(const std::string& text);

    void push(size_t value);

    std::string binary_to_string(binary_oper op);
    std::string unary_to_string(unary_oper op);
    std::string assign_to_string(assign_type op);
    std::string type_to_string(const Type& type);
};