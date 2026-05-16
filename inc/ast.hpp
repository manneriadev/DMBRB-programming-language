//ast.hpp
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <cstdint>

// FORWARD DECL

struct Expr;
struct Stmt;
struct Decl;
struct Visitor;

// TYPES & OPERATORS


enum class InnerType{
    INT8, INT16, INT32, INT64,
    UINT8, UINT16, UINT32, UINT64,
    FLOAT32, FLOAT64,
    BOOL, CHAR,
    VOID,
    STRUCT
};

struct ArrayDim{
    bool is_dynamic = false;
    std::unique_ptr<Expr> size_expr;

    ArrayDim() = default;

    ArrayDim(ArrayDim&&) = default;
    ArrayDim& operator=(ArrayDim&&) = default;

    ArrayDim(const ArrayDim& other)
    {
        is_dynamic = other.is_dynamic;
        if (other.size_expr)size_expr = nullptr;
    }

    ArrayDim& operator=(const ArrayDim& other){
        if (this == &other) return *this;

        is_dynamic = other.is_dynamic;
        size_expr = nullptr;

        return *this;
    }
};

struct Type{
    InnerType base = InnerType::VOID;
    std::string struct_name;
    std::vector<ArrayDim> arrays;
    bool is_optional = false;
    bool is_array = false; // true when [elem_type; size]
};

struct arg_type{
    Type type;
    bool is_const = false;
};

struct pair_arg{
    std::string name;
    arg_type type;
};

enum class binary_oper{
    PLUS, MINUS, STAR, STARSTAR,
    SLASH, SLASHSLASH, PERCENT,
    LESSLESS, GREATERGREATER,
    XOR, PIPE, AMP,
    EQEQ, NOTEQ,
    GREATER, LESS, GREATEREQ, LESSEQ,
    AMPERAMPER, PIPEPIPE,
};

enum class unary_oper{
    TILDA,
    NEGATIVE,
    POSITIVE,
    NOT
};

enum class assign_type{
    EQ,
    PLUSEQ, MINUSEQ,
    STAREQ, SLASHEQ, PERCENTEQ,
    AMPEQ, PIPEEQ
};

// BASE NODE

struct Node{
    virtual ~Node() = default;
    virtual void accept(Visitor& v) = 0;

    size_t line = 0;
    size_t column = 0;
};

struct Expr : Node{};
struct Stmt : Node{};
struct Decl : Node{};

// EXPRESSIONS

struct LiteralExpr : Expr{
    std::variant<int64_t, double, bool, std::string, char> value;
    void accept(Visitor& v) override;
};

struct VariableExpr : Expr{
    std::string name;
    void accept(Visitor& v) override;
};

struct BinaryExpr : Expr{
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    binary_oper op;
    void accept(Visitor& v) override;
};

struct UnaryExpr : Expr{
    std::unique_ptr<Expr> expr;
    unary_oper op;
    void accept(Visitor& v) override;
};

struct AssignmentExpr : Expr{
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> value;
    assign_type type;
    void accept(Visitor& v) override;
};

struct CallExpr : Expr{
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;
    void accept(Visitor& v) override;
};

struct MemberAccessExpr : Expr{
    std::unique_ptr<Expr> object;
    std::string field;
    void accept(Visitor& v) override;
};

struct MethodCallExpr : Expr{
    std::unique_ptr<Expr> object;
    std::string method;
    std::vector<std::unique_ptr<Expr>> args;
    void accept(Visitor& v) override;
};

struct IndexExpr : Expr{
    std::unique_ptr<Expr> object;
    std::unique_ptr<Expr> index;
    void accept(Visitor& v) override;
};

struct TernaryExpr : Expr{
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> thenExpr;
    std::unique_ptr<Expr> elseExpr;
    void accept(Visitor& v) override;
};

struct CastExpr : Expr{
    std::unique_ptr<Expr> expr;
    Type target_type;
    void accept(Visitor& v) override;
};

struct ArrayLiteralExpr : Expr{
    std::vector<std::unique_ptr<Expr>> elements;
    void accept(Visitor& v) override;
};

struct EmptyExpr : Expr{
    void accept(Visitor& v) override;
};

// STATEMENTS

struct BlockStmt : Stmt{
    std::vector<std::unique_ptr<Node>> items;
    void accept(Visitor& v) override;
};

struct ExprStmt : Stmt{
    std::unique_ptr<Expr> expr;
    void accept(Visitor& v) override;
};

struct IfStmt : Stmt{
    std::unique_ptr<Expr> cond;
    std::unique_ptr<BlockStmt> then_block;
    std::unique_ptr<BlockStmt> else_block;
    void accept(Visitor& v) override;
};

struct WhenStmt : Stmt{
    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<BlockStmt>>> cases;
    std::unique_ptr<BlockStmt> else_block;
    void accept(Visitor& v) override;
};

struct WhileStmt : Stmt{
    std::unique_ptr<Expr> cond;
    std::unique_ptr<BlockStmt> body;
    void accept(Visitor& v) override;
};

struct ForStmt : Stmt{
    std::string var_name;
    std::unique_ptr<Expr> start;
    std::unique_ptr<Expr> end;
    std::unique_ptr<Expr> step;
    std::unique_ptr<BlockStmt> body;
    void accept(Visitor& v) override;
};

struct ReturnStmt : Stmt{
    std::unique_ptr<Expr> value;
    void accept(Visitor& v) override;
};

struct BreakStmt : Stmt{
    void accept(Visitor& v) override;
};

struct ContinueStmt : Stmt{
    void accept(Visitor& v) override;
};

// DECLARATIONS

struct VarDecl : Decl{
    std::string name;
    arg_type type;
    bool has_type = false;
    std::unique_ptr<Expr> init;
    bool has_init = false;
    void accept(Visitor& v) override;
};

struct FunctionDecl : Decl{
    std::string name;
    std::vector<pair_arg> args;
    Type return_type;
    std::unique_ptr<BlockStmt> body;
    void accept(Visitor& v) override;
};

struct StructDecl : Decl{
    std::string name;
    std::vector<pair_arg> fields;
    void accept(Visitor& v) override;
};

// MODULE / PROGRAM

struct Module : Node{
    std::string name;
    std::vector<std::unique_ptr<Decl>> decls;
    std::vector<std::unique_ptr<Module>> submodules;
    void accept(Visitor& v) override;
};

struct Program : Node {
    std::vector<std::unique_ptr<Node>> items;
    void accept(Visitor& v) override;
};

// VISITOR

struct Visitor{
    // expressions
    virtual void visit(LiteralExpr&) = 0;
    virtual void visit(VariableExpr&) = 0;
    virtual void visit(BinaryExpr&) = 0;
    virtual void visit(UnaryExpr&) = 0;
    virtual void visit(AssignmentExpr&) = 0;
    virtual void visit(CallExpr&) = 0;
    virtual void visit(MemberAccessExpr&) = 0;
    virtual void visit(MethodCallExpr&) = 0;
    virtual void visit(IndexExpr&) = 0;
    virtual void visit(TernaryExpr&) = 0;
    virtual void visit(CastExpr&) = 0;
    virtual void visit(ArrayLiteralExpr&) = 0;
    virtual void visit(EmptyExpr&) = 0;

    // statements
    virtual void visit(BlockStmt&) = 0;
    virtual void visit(ExprStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(WhenStmt&) = 0;
    virtual void visit(WhileStmt&) = 0;
    virtual void visit(ForStmt&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(BreakStmt&) = 0;
    virtual void visit(ContinueStmt&) = 0;

    // declarations
    virtual void visit(VarDecl&) = 0;
    virtual void visit(FunctionDecl&) = 0;
    virtual void visit(StructDecl&) = 0;

    // module/program

    virtual void visit(Program&) = 0;
    virtual void visit(Module&) = 0;

    virtual ~Visitor() = default;
};

inline void LiteralExpr::accept(Visitor& v) { v.visit(*this); }
inline void VariableExpr::accept(Visitor& v) { v.visit(*this); }
inline void BinaryExpr::accept(Visitor& v) { v.visit(*this); }
inline void UnaryExpr::accept(Visitor& v) { v.visit(*this); }
inline void AssignmentExpr::accept(Visitor& v) { v.visit(*this); }
inline void CallExpr::accept(Visitor& v) { v.visit(*this); }
inline void MemberAccessExpr::accept(Visitor& v) { v.visit(*this); }
inline void MethodCallExpr::accept(Visitor& v) { v.visit(*this); }
inline void IndexExpr::accept(Visitor& v) { v.visit(*this); }
inline void TernaryExpr::accept(Visitor& v) { v.visit(*this); }
inline void CastExpr::accept(Visitor& v) { v.visit(*this); }
inline void ArrayLiteralExpr::accept(Visitor& v) { v.visit(*this); }
inline void EmptyExpr::accept(Visitor& v) { v.visit(*this); }
inline void BlockStmt::accept(Visitor& v) { v.visit(*this); }
inline void ExprStmt::accept(Visitor& v) { v.visit(*this); }
inline void IfStmt::accept(Visitor& v) { v.visit(*this); }
inline void WhenStmt::accept(Visitor& v) { v.visit(*this); }
inline void WhileStmt::accept(Visitor& v) { v.visit(*this); }
inline void ForStmt::accept(Visitor& v) { v.visit(*this); }
inline void ReturnStmt::accept(Visitor& v) { v.visit(*this); }
inline void BreakStmt::accept(Visitor& v) { v.visit(*this); }
inline void ContinueStmt::accept(Visitor& v) { v.visit(*this); }
inline void VarDecl::accept(Visitor& v) { v.visit(*this); }
inline void FunctionDecl::accept(Visitor& v) { v.visit(*this); }
inline void StructDecl::accept(Visitor& v) { v.visit(*this); }
inline void Module::accept(Visitor& v) { v.visit(*this); }
inline void Program::accept(Visitor& v) { for (auto& item : items) item->accept(v); }