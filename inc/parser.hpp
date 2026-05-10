//parser.hpp
#pragma once

#include "ast.hpp"
#include "token.hpp"

#include <cstddef>
#include <memory>
#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token>& tokens);
    std::unique_ptr<Program> parse();

private:
    std::vector<Token>* tokens;
    std::size_t pos;
    bool failed;


    Token* cur() const;
    void next();
    bool at_end() const;
    bool match(TokenType type) const;
    bool eat(TokenType type);
    void fail(const char* msg);
    bool is_stop(TokenType type, const std::vector<TokenType>& stops) const;
    bool is_assignment(TokenType type) const;
    int priority_of(TokenType type) const;

    bool is_decl_start(TokenType type) const;
    bool is_stmt_start(TokenType type) const;
    bool is_expr_start(TokenType type) const;
    bool is_type_start(TokenType type) const;

    std::unique_ptr<Module> parse_module();
    
    std::unique_ptr<Decl> parse_decl();
    std::unique_ptr<VarDecl> parse_var_decl();
    std::unique_ptr<FunctionDecl> parse_function_decl();
    std::unique_ptr<StructDecl> parse_struct_decl();

    pair_arg parse_pair_arg();
    arg_type parse_arg_type();
    Type parse_type();
    bool try_parse_type(Type& out);

    std::unique_ptr<BlockStmt> parse_block(TokenType stop1, TokenType stop2 = TokenType::ERROR, TokenType stop3 = TokenType::ERROR);
    std::unique_ptr<Stmt> parse_stmt();

    std::unique_ptr<Stmt> parse_if_stmt();
    std::unique_ptr<Stmt> parse_when_stmt();
    std::unique_ptr<Stmt> parse_while_stmt();
    std::unique_ptr<Stmt> parse_for_stmt();
    std::unique_ptr<Stmt> parse_return_stmt();
    std::unique_ptr<Stmt> parse_expr_stmt();

    std::unique_ptr<Expr> parse_expression();
    std::unique_ptr<Expr> parse_binary(int min_priority);
    std::unique_ptr<Expr> parse_unary();
    std::unique_ptr<Expr> parse_postfix();
    std::unique_ptr<Expr> parse_primary();
    std::unique_ptr<Expr> parse_group();

    bool right_associative(TokenType type) const;

    binary_oper map_binary(TokenType type) const;
    unary_oper map_unary(TokenType type) const;
    assign_type map_assign(TokenType type) const;
};

std::unique_ptr<Program> parse_program(std::vector<Token>& tokens);