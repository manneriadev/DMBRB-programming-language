//codegen.hpp

#pragma once
#include "ast.hpp"
#include <ostream>
#include <string>

class Codegen : public Visitor
{
public:
    explicit Codegen(std::ostream& out);
    void generate(Program& program);

private:
    std::ostream& out;
    int indent = 0;

    void write(const std::string& s);
    void writeln(const std::string& s);
    void begin_indent();
    void end_indent();
    std::string decl_to_c(const Type& t, const std::string& name);
    std::string type_to_c(const Type& t);
    std::string mangle(const std::string& module, const std::string& name);

    // expressions
    void visit(AddrOfExpr&) override;
    void visit(DerefExpr&) override;
    void visit(LiteralExpr&) override;
    void visit(VariableExpr&) override;
    void visit(SelfExpr&) override;
    void visit(BinaryExpr&) override;
    void visit(UnaryExpr&) override;
    void visit(AssignmentExpr&) override;
    void visit(CallExpr&) override;
    void visit(MemberAccessExpr&) override;
    void visit(MethodCallExpr&) override;
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
    void visit(Module&) override;
    void visit(Program&) override;

    std::string current_module;
};