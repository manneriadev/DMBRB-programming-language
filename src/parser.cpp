//parser.cpp
#include "parser.hpp"

#include <cstdio>
#include <cstdlib>
#include <utility>

void Parser::get_pos(Node* node)
{
    if(cur())
    {
        node->line = cur()->line;
        node->column = cur()->column;
    }
}

Parser::Parser(std::vector<Token>& tokens) : tokens(&tokens), pos(0), failed(false) {}

std::unique_ptr<Program> Parser::parse() 
{
    auto program = std::make_unique<Program>();
    get_pos(program.get());
    
    while(!at_end()) 
    {
        if(eat(TokenType::MODULE)) 
        {
            auto module = parse_module();
            if(failed) return nullptr;

            program->items.push_back(std::move(module));
            continue;
        }

        if(is_decl_start(cur()->type)) 
        {
            auto decl = parse_decl();
            if(failed) return nullptr;

            program->items.push_back(std::move(decl));
            continue;
        }

        fail("expected module or declaration");
        return nullptr;
    }

    if(failed) return nullptr;
    return program; 
}

std::unique_ptr<Program> parse_program(std::vector<Token>& tokens) 
{
    Parser p(tokens);
    return p.parse();
}

// helper functions

Token* Parser::cur() const 
{
    if(!tokens || pos >= tokens->size()) return nullptr;
    return &(*tokens)[pos];
}

void Parser::next() 
{
    if(tokens && !at_end()) ++pos;
}

bool Parser::at_end() const 
{
    return !cur() || cur()->type == TokenType::END_TOKEN;
}

bool Parser::match(TokenType type) const 
{
    return cur() && cur()->type == type;
}

bool Parser::eat(TokenType type) 
{
    if(match(type)) 
    {
        next();
        return true;
    }
    return false;
}

void Parser::fail(const char* msg) 
{
    if(failed) return;

    failed = true;

    if(cur()) std::fprintf(stderr, "parser error at %zu:%zu: %s\n", cur()->line, cur()->column, msg);
    else if(tokens && !tokens->empty())
    {
        const Token& last = tokens->back();
        std::fprintf(stderr, "parser error near end at %zu:%zu: %s\n", last.line, last.column, msg);
    } 
    else std::fprintf(stderr, "parser error: %s\n", msg);
}

bool Parser::is_stop(TokenType type, const std::vector<TokenType>& stops) const
{
    for(auto s : stops)
    {
        if(type == s) return true;
    }
    return false;
}

bool Parser::is_decl_start(TokenType type) const 
{
    return type == TokenType::VAR
        || type == TokenType::FUNCTION
        || type == TokenType::STRUCT;
}

bool Parser::is_stmt_start(TokenType type) const 
{
    return type == TokenType::IF
        || type == TokenType::WHEN
        || type == TokenType::WHILE
        || type == TokenType::FOR
        || type == TokenType::RETURN
        || type == TokenType::BREAK
        || type == TokenType::CONTINUE;
}

bool Parser::is_expr_start(TokenType type) const 
{
    return type == TokenType::IDENTIFIER
        || type == TokenType::INT_LITERAL
        || type == TokenType::FLOAT_LITERAL
        || type == TokenType::STRING_LITERAL
        || type == TokenType::CHAR_LITERAL
        || type == TokenType::TRUE
        || type == TokenType::FALSE
        || type == TokenType::NONE
        || type == TokenType::LPAREN
        || type == TokenType::LSQUAREPAREN
        || type == TokenType::PLUS
        || type == TokenType::MINUS
        || type == TokenType::NOT
        || type == TokenType::TILDA
        || type == TokenType::COLONCOLON;
}

bool Parser::is_type_start(TokenType type) const 
{
    return type == TokenType::INT8
        || type == TokenType::INT16
        || type == TokenType::INT32
        || type == TokenType::INT64
        || type == TokenType::UINT8
        || type == TokenType::UINT16
        || type == TokenType::UINT32
        || type == TokenType::UINT64
        || type == TokenType::FLOAT32
        || type == TokenType::FLOAT64
        || type == TokenType::BOOL
        || type == TokenType::CHAR
        || type == TokenType::VOID
        || type == TokenType::STRUCT
        || type == TokenType::LSQUAREPAREN;
}

bool Parser::is_assignment(TokenType type) const
{
    return
        type == TokenType::EQ ||
        type == TokenType::PLUSEQ ||
        type == TokenType::MINUSEQ ||
        type == TokenType::SLASHEQ ||
        type == TokenType::PERCENTEQ ||
        type == TokenType::AMPEQ ||
        type == TokenType::PIPEEQ ||
        type == TokenType::STAREQ;
}

int Parser::priority_of(TokenType type) const
{
    // assignment
    if(is_assignment(type)) return 1;

    // ternary
    if(type == TokenType::QUESTION) return 2;

    if(type == TokenType::PIPEPIPE) return 3;
    if(type == TokenType::AMPERAMPER) return 4;

    if(type == TokenType::PIPE) return 5;
    if(type == TokenType::XOR) return 6;
    if(type == TokenType::AMP) return 7;

    if(type == TokenType::EQEQ ||
        type == TokenType::NOTEQ) return 8;

    if(type == TokenType::GREATER ||
        type == TokenType::LESS ||
        type == TokenType::GREATEREQ ||
        type == TokenType::LESSEQ) return 9;

    if(type == TokenType::LESSLESS ||
        type == TokenType::GREATERGREATER) return 10;

    if(type == TokenType::PLUS ||
        type == TokenType::MINUS) return 11;

    if(type == TokenType::STAR ||
        type == TokenType::SLASH ||
        type == TokenType::SLASHSLASH ||
        type == TokenType::PERCENT) return 12;

    if(type == TokenType::STARSTAR) return 13;

    return 0;
}

bool Parser::right_associative(TokenType type) const
{
    return
        type == TokenType::STARSTAR ||
        type == TokenType::QUESTION ||
        is_assignment(type);
}

binary_oper Parser::map_binary(TokenType type) const
{
    switch(type)
    {
        case TokenType::PLUS: return binary_oper::PLUS;
        case TokenType::MINUS: return binary_oper::MINUS;
        case TokenType::STAR: return binary_oper::STAR;
        case TokenType::STARSTAR: return binary_oper::STARSTAR;
        case TokenType::SLASH: return binary_oper::SLASH;
        case TokenType::SLASHSLASH: return binary_oper::SLASHSLASH;
        case TokenType::PERCENT: return binary_oper::PERCENT;
        case TokenType::LESSLESS: return binary_oper::LESSLESS;
        case TokenType::GREATERGREATER: return binary_oper::GREATERGREATER;
        case TokenType::XOR: return binary_oper::XOR;
        case TokenType::PIPE: return binary_oper::PIPE;
        case TokenType::AMP: return binary_oper::AMP;
        case TokenType::EQEQ: return binary_oper::EQEQ;
        case TokenType::NOTEQ: return binary_oper::NOTEQ;
        case TokenType::GREATER: return binary_oper::GREATER;
        case TokenType::LESS: return binary_oper::LESS;
        case TokenType::GREATEREQ: return binary_oper::GREATEREQ;
        case TokenType::LESSEQ: return binary_oper::LESSEQ;
        case TokenType::AMPERAMPER: return binary_oper::AMPERAMPER;
        case TokenType::PIPEPIPE: return binary_oper::PIPEPIPE;
        default:
            std::fprintf(stderr, "internal error: unknown binary operator\n");
            return binary_oper::PLUS;
    }
}

unary_oper Parser::map_unary(TokenType type) const
{
    switch(type)
    {
        case TokenType::MINUS: return unary_oper::NEGATIVE;
        case TokenType::PLUS: return unary_oper::POSITIVE;
        case TokenType::NOT: return unary_oper::NOT;
        case TokenType::TILDA: return unary_oper::TILDA;
        default:
            std::fprintf(stderr, "internal error: unknown unary operator\n");
            return unary_oper::NEGATIVE;
    }
}

assign_type Parser::map_assign(TokenType type) const
{
    switch(type)
    {
        case TokenType::EQ: return assign_type::EQ;
        case TokenType::PLUSEQ: return assign_type::PLUSEQ;
        case TokenType::MINUSEQ: return assign_type::MINUSEQ;
        case TokenType::STAREQ: return assign_type::STAREQ;
        case TokenType::SLASHEQ: return assign_type::SLASHEQ;
        case TokenType::PERCENTEQ: return assign_type::PERCENTEQ;
        case TokenType::AMPEQ: return assign_type::AMPEQ;
        case TokenType::PIPEEQ: return assign_type::PIPEEQ;
        default:
            std::fprintf(stderr, "internal error: unknown assign operator\n");
            return assign_type::EQ;
    }
}

// program / module

std::unique_ptr<Module> Parser::parse_module() 
{
    auto module = std::make_unique<Module>();
    get_pos(module.get());

    if(!match(TokenType::IDENTIFIER)) 
    {
        fail("expected module name after 'module'");
        return nullptr;
    }

    module->name = cur()->lexeme;
    next();

    if(!eat(TokenType::BEGIN))
    {
        fail("expected 'begin' after module name");
        return nullptr;
    }

    while(!at_end() && !match(TokenType::END))
    {
        if(eat(TokenType::MODULE))
        {
            auto submodule = parse_module();
            if(failed) return nullptr;
            module->submodules.push_back(std::move(submodule));
            continue;
        }

        if(!is_decl_start(cur()->type)) 
        {
            fail("only declarations are allowed inside module body");
            return nullptr;
        }

        auto decl = parse_decl();
        if(failed) return nullptr;

        module->decls.push_back(std::move(decl));
    }

    if(!eat(TokenType::END)) 
    {
        fail("expected 'end' to close module");
        return nullptr;
    }

    return module;
}

// declarations

std::unique_ptr<Decl> Parser::parse_decl() 
{
    if(match(TokenType::VAR)) return parse_var_decl();
    if(match(TokenType::FUNCTION)) return parse_function_decl();
    if(match(TokenType::STRUCT)) return parse_struct_decl();

    fail("expected declaration");
    return nullptr;
}

std::unique_ptr<VarDecl> Parser::parse_var_decl() 
{
    auto decl = std::make_unique<VarDecl>();
    get_pos(decl.get());

    if(!eat(TokenType::VAR)) 
    {
        fail("expected 'var'");
        return nullptr;
    }

    if(eat(TokenType::CONST)) decl->type.is_const = true;

    if(!match(TokenType::IDENTIFIER))
    {
        fail("expected variable name");
        return nullptr;
    }

    decl->name = cur()->lexeme;
    next();

    if(eat(TokenType::COLON)) 
    {
        if(!try_parse_type(decl->type.type))
        {
            fail("expected type");
            return nullptr;
        }
        decl->has_type = true;
    }

    if(eat(TokenType::EQ))
    {
        decl->init = parse_expression();
        if(failed) return nullptr;
        decl->has_init = true;
    }

    if(!decl->has_type && !decl->has_init)
    {
        fail("variable declaration requires a type or an initializer");
        return nullptr;
    }

    return decl;
}

std::unique_ptr<FunctionDecl> Parser::parse_function_decl() 
{
    auto decl = std::make_unique<FunctionDecl>();
    get_pos(decl.get());

    if(!eat(TokenType::FUNCTION))
    {
        fail("expected 'function'");
        return nullptr;
    }

    if(!match(TokenType::IDENTIFIER)) 
    {
        fail("expected function name");
        return nullptr;
    }

    decl->name = cur()->lexeme;
    next();

    if(!eat(TokenType::LPAREN)) 
    {
        fail("expected '(' after function name");
        return nullptr;
    }

    if(!match(TokenType::RPAREN)) 
    {
        while(true) 
        {
            decl->args.push_back(parse_pair_arg());
            if(failed) return nullptr;
            if(eat(TokenType::COMMA)) continue;
            if(match(TokenType::RPAREN)) break;

            fail("expected ',' or ')'");
            return nullptr;
        }
    }

    if(!eat(TokenType::RPAREN)) 
    {
        fail("expected ')' after function arguments");
        return nullptr;
    }

    if(eat(TokenType::COLONCOLON))
    {
        decl->return_type = parse_type();
        if (failed) return nullptr;
    } 
    else 
    {
        decl->return_type.base = InnerType::VOID;
        decl->return_type.struct_name.clear();
        decl->return_type.arrays.clear();
    }

    if(!eat(TokenType::BEGIN))
    {
        fail("expected 'begin' after function header");
        return nullptr;
    }

    decl->body = parse_block(TokenType::END);
    if(failed) return nullptr;
    
    if(!eat(TokenType::END))
    {
        fail("expected 'end' to close function");
        return nullptr;
    }

    return decl;
}

std::unique_ptr<StructDecl> Parser::parse_struct_decl()
{
    auto decl = std::make_unique<StructDecl>();
    get_pos(decl.get());

    if(!eat(TokenType::STRUCT)) 
    {
        fail("expected 'struct'");
        return nullptr;
    }

    if(!match(TokenType::IDENTIFIER)) {
        fail("expected struct name");
        return nullptr;
    }

    decl->name = cur()->lexeme;
    next();

    if(!eat(TokenType::BEGIN))
    {
        fail("expected 'begin' after struct name");
        return nullptr;
    }

    while(!at_end() && !match(TokenType::END))
    {
        decl->fields.push_back(parse_pair_arg());
        if(failed) return nullptr;
    }

    if(!eat(TokenType::END)) {
        fail("expected 'end' to close struct");
        return nullptr;
    }

    return decl;
}

pair_arg Parser::parse_pair_arg() 
{
    pair_arg arg;

    if(!match(TokenType::IDENTIFIER))
    {
        fail("expected name");
        return arg;
    }

    arg.name = cur()->lexeme;
    next();

    if(!eat(TokenType::COLON))
    {
        fail("expected ':' after name");
        return arg;
    }

    arg.type = parse_arg_type();
    return arg;
}

arg_type Parser::parse_arg_type() 
{
    arg_type out;

    if(eat(TokenType::CONST)) out.is_const = true;

    out.type = parse_type();
    return out;
}

bool Parser::try_parse_type(Type& out)
{
    const std::size_t save = pos;

    if(eat(TokenType::LSQUAREPAREN))
    {
        Type elem_type;
        if(!try_parse_type(elem_type))
        {
            pos = save;
            return false;
        }

        if(!eat(TokenType::SEMICOLON))
        {
            fail("expected ';' in array type (use [T; N] or [T;])");
            return false;
        }

        ArrayDim dim{};
        if(match(TokenType::RSQUAREPAREN))
        {
            dim.is_dynamic = true;
        }
        else
        {
            dim.size_expr = parse_expression();
            if(failed) return false;
        }

        if(!eat(TokenType::RSQUAREPAREN))
        {
            fail("expected ']' to close array type");
            return false;
        }

        out = elem_type;
        out.arrays.push_back(std::move(dim));

        if(eat(TokenType::QUESTION)) out.is_optional = true;
        return true;
    }

    Type type;
    type.base = InnerType::VOID;
    type.struct_name.clear();

    switch(cur()->type)
    {
        case TokenType::INT8: type.base = InnerType::INT8; break;
        case TokenType::INT16: type.base = InnerType::INT16; break;
        case TokenType::INT32: type.base = InnerType::INT32; break;
        case TokenType::INT64: type.base = InnerType::INT64; break;
        case TokenType::UINT8: type.base = InnerType::UINT8; break;
        case TokenType::UINT16: type.base = InnerType::UINT16; break;
        case TokenType::UINT32: type.base = InnerType::UINT32; break;
        case TokenType::UINT64: type.base = InnerType::UINT64; break;
        case TokenType::FLOAT32: type.base = InnerType::FLOAT32; break;
        case TokenType::FLOAT64: type.base = InnerType::FLOAT64; break;
        case TokenType::BOOL: type.base = InnerType::BOOL; break;
        case TokenType::CHAR: type.base = InnerType::CHAR; break;
        case TokenType::VOID: type.base = InnerType::VOID; break;

        case TokenType::STRUCT:
        {
            next();
            if(!match(TokenType::IDENTIFIER))
            {
                pos = save;
                return false;
            }
            type.base = InnerType::STRUCT;
            type.struct_name = cur()->lexeme;
            next();

            if(eat(TokenType::QUESTION)) type.is_optional = true;
            out = type;
            return true;
        }

        default:
            pos = save;
            return false;
    }

    next();

    if(eat(TokenType::QUESTION)) type.is_optional = true;

    out = type;
    return true;
}

Type Parser::parse_type() 
{
    Type out;
    if(!try_parse_type(out)) fail("expected type");
    return out;
}

// blocks / statements ------------

std::unique_ptr<BlockStmt> Parser::parse_block(TokenType stop1, TokenType stop2, TokenType stop3) 
{
    auto block = std::make_unique<BlockStmt>();
    get_pos(block.get());

    while(!at_end() && !is_stop(cur()->type, {stop1, stop2, stop3})) 
    {
        if(is_decl_start(cur()->type)) 
        {
            auto decl = parse_decl();
            if(failed) return nullptr;
            block->items.push_back(std::move(decl));
            continue;
        }

        if(is_stmt_start(cur()->type) || is_expr_start(cur()->type)) 
        {
            auto stmt = parse_stmt();
            if(failed) return nullptr;

            block->items.push_back(std::move(stmt));
            continue;
        }

        fail("unexpected token inside block");
        return nullptr;
    }

    return block;
}

std::unique_ptr<Stmt> Parser::parse_stmt() 
{
    if(match(TokenType::IF)) return parse_if_stmt();
    if(match(TokenType::WHEN)) return parse_when_stmt();
    if(match(TokenType::WHILE)) return parse_while_stmt();
    if(match(TokenType::FOR)) return parse_for_stmt();
    if(match(TokenType::RETURN)) return parse_return_stmt();
    if(eat(TokenType::BREAK))
    {
        auto br = std::make_unique<BreakStmt>();
        get_pos(br.get());
        return br;
    }
    if(eat(TokenType::CONTINUE))
    {
        auto cont = std::make_unique<ContinueStmt>();
        get_pos(cont.get());
        return cont;
    }
    if(is_expr_start(cur() ? cur()->type : TokenType::ERROR)) return parse_expr_stmt();

    fail("expected statement");
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_if_stmt() 
{
    auto stmt = std::make_unique<IfStmt>();
    get_pos(stmt.get());

    if(!eat(TokenType::IF))
    {
        fail("expected 'if'");
        return nullptr;
    }

    stmt->cond = parse_expression();
    if(failed) return nullptr;

    if(!eat(TokenType::BEGIN))
    {
        fail("expected 'begin' after if condition");
        return nullptr;
    }

    stmt->then_block = parse_block(TokenType::ELSE, TokenType::END);
    if(failed) return nullptr;

    if(eat(TokenType::ELSE)) 
    {
        if(!eat(TokenType::BEGIN))
        {
            fail("expected 'begin' after else");
            return nullptr;
        }

        stmt->else_block = parse_block(TokenType::END);
        if(failed) return nullptr;
    }

    if(!eat(TokenType::END)) 
    {
        fail("expected 'end' to close if");
        return nullptr;
    }

    return stmt;
}

std::unique_ptr<Stmt> Parser::parse_when_stmt() 
{
    auto stmt = std::make_unique<WhenStmt>();
    get_pos(stmt.get());

    if(!eat(TokenType::WHEN)) 
    {
        fail("expected 'when'");
        return nullptr;
    }

    while(!at_end() && !match(TokenType::END)) 
    {
        if(eat(TokenType::ELSE)) 
        {
            if(!eat(TokenType::ARROW)) 
            {
                fail("expected '->' after else");
                return nullptr;
            }

            if(!eat(TokenType::BEGIN))
            {
                fail("expected 'begin' after else ->");
                return nullptr;
            }

            auto body = parse_block(TokenType::END);
            if(failed) return nullptr;

            if(!eat(TokenType::END)) 
            {
                fail("expected 'end' after else block");
                return nullptr;
            }

            stmt->else_block = std::move(body);
            break;
        }

        auto cond = parse_expression();
        if(failed) return nullptr;

        if(!eat(TokenType::ARROW)) 
        {
            fail("expected '->' after condition");
            return nullptr;
        }

        if(!eat(TokenType::BEGIN))
        {
            fail("expected 'begin' after ->");
            return nullptr;
        }

        auto body = parse_block(TokenType::END);
        if(failed) return nullptr;

        if(!eat(TokenType::END)) 
        {
            fail("expected 'end' after case block");
            return nullptr;
        }

        stmt->cases.push_back(std::make_pair(std::move(cond), std::move(body)));
    }

    if(!eat(TokenType::END)) 
    {
        fail("expected 'end' to close when");
        return nullptr;
    }

    return stmt;
}

std::unique_ptr<Stmt> Parser::parse_while_stmt() 
{
    auto stmt = std::make_unique<WhileStmt>();
    get_pos(stmt.get());

    if(!eat(TokenType::WHILE))
    {
        fail("expected 'while'");
        return nullptr;
    }

    stmt->cond = parse_expression();
    if(failed) return nullptr;

    if(!eat(TokenType::BEGIN))
    {
        fail("expected 'begin' after while condition");
        return nullptr;
    }

    stmt->body = parse_block(TokenType::END);
    if(failed) return nullptr;
    

    if(!eat(TokenType::END)) 
    {
        fail("expected 'end' to close while");
        return nullptr;
    }

    return stmt;
}

std::unique_ptr<Stmt> Parser::parse_for_stmt()
{
    if(!eat(TokenType::FOR)) 
    {
        fail("expected 'for'");
        return nullptr;
    }

    if(!match(TokenType::IDENTIFIER)) 
    {
        fail("expected loop variable name");
        return nullptr;
    }

    std::string var_name = cur()->lexeme;
    next();

    if(!eat(TokenType::EQ)) 
    {
        fail("expected '=' after loop variable name");
        return nullptr;
    }

    auto start_expr = parse_expression();
    if(failed) return nullptr;

    if(!eat(TokenType::COLON))
    {
        fail("expected ':' after start expression");
        return nullptr;
    }

    auto end_expr = parse_expression();
    if(failed) return nullptr;

    std::unique_ptr<Expr> step_expr = nullptr;

    if(eat(TokenType::COLON)) 
    {
        step_expr = parse_expression();
        if (failed) return nullptr;
    }

    auto stmt = std::make_unique<ForStmt>();
    get_pos(stmt.get());
    stmt->var_name = var_name;
    stmt->start = std::move(start_expr);
    stmt->end = std::move(end_expr);
    stmt->step = std::move(step_expr);

    if(!eat(TokenType::BEGIN))
    {
        fail("expected 'begin' after for codition");
        return nullptr;
    }

    stmt->body = parse_block(TokenType::END);
    if(failed) return nullptr;

    if(!eat(TokenType::END)) 
    {
        fail("expected 'end' to close for");
        return nullptr;
    }

    return stmt;
}

std::unique_ptr<Stmt> Parser::parse_return_stmt()
{
    auto stmt = std::make_unique<ReturnStmt>();
    get_pos(stmt.get());

    if(!eat(TokenType::RETURN))
    {
        fail("expected 'return'");
        return nullptr;
    }

    if(cur() && is_expr_start(cur()->type))
    {
        stmt->value = parse_expression();
        if(failed) return nullptr;
    }

    return stmt;
}

std::unique_ptr<Stmt> Parser::parse_expr_stmt()
{
    auto stmt = std::make_unique<ExprStmt>();
    get_pos(stmt.get());
    stmt->expr = parse_expression();
    if(failed) return nullptr;
    return stmt;
}

// expressions ----------------------

std::unique_ptr<Expr> Parser::parse_expression()
{
    return parse_binary(1);
}

std::unique_ptr<Expr> Parser::parse_binary(int min_priority)
{
    auto left = parse_unary();
    if(failed) return nullptr;
    

    while(cur())
    {
        TokenType op = cur()->type;

        int prio = priority_of(op);
        if(prio == 0 || prio < min_priority) break;
        

        next();

        // ternary
        if(op == TokenType::QUESTION)
        {
            auto then_expr = parse_expression();
            if(failed) return nullptr;

            if(!eat(TokenType::COLON))
            {
                fail("expected ':' in ternary expression");
                return nullptr;
            }

            auto else_expr = parse_expression();
            if(failed) return nullptr;

            auto node = std::make_unique<TernaryExpr>();
            get_pos(node.get());
            node->cond = std::move(left);
            node->thenExpr = std::move(then_expr);
            node->elseExpr = std::move(else_expr);

            left = std::move(node);
            continue;
        }

        int next_priority = prio + (right_associative(op) ? 0 : 1);

        auto right = parse_binary(next_priority);
        if(failed) return nullptr;

        if(is_assignment(op))
        {
            auto node = std::make_unique<AssignmentExpr>();
            get_pos(node.get());
            node->target = std::move(left);
            node->value = std::move(right);
            node->type = map_assign(op);

            left = std::move(node);
            continue;
        }

        auto node = std::make_unique<BinaryExpr>();
        get_pos(node.get());
        node->left = std::move(left);
        node->right = std::move(right);
        node->op = map_binary(op);

        left = std::move(node);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parse_unary()
{
    if(match(TokenType::PLUS) || match(TokenType::MINUS) || match(TokenType::NOT) || match(TokenType::TILDA))
    {

        TokenType op = cur()->type;
        next();

        auto node = std::make_unique<UnaryExpr>();
        get_pos(node.get());
        node->op = map_unary(op);
        node->expr = parse_unary();

        if(failed) return nullptr;

        return node;
    }

    if(match(TokenType::LPAREN)) return parse_group();
    return parse_postfix();
}

std::unique_ptr<Expr> Parser::parse_postfix()
{
    auto expr = parse_primary();
    if(failed) return nullptr;

    while(cur())
    {
        if(eat(TokenType::DOT))
        {
            if(!match(TokenType::IDENTIFIER))
            {
                fail("expected field name after '.'");
                return nullptr;
            }

            auto node = std::make_unique<MemberAccessExpr>();
            get_pos(node.get());
            node->object = std::move(expr);
            node->field = cur()->lexeme;

            next();

            expr = std::move(node);
            continue;
        }

        if(eat(TokenType::LSQUAREPAREN))
        {
            auto node = std::make_unique<IndexExpr>();
            get_pos(node.get());
            node->object = std::move(expr);

            node->index = parse_expression();
            if(failed) return nullptr;
            

            if(!eat(TokenType::RSQUAREPAREN))
            {
                fail("expected ']'");
                return nullptr;
            }

            expr = std::move(node);
            continue;
        }

        if(eat(TokenType::LPAREN))
        {
            if(auto* mem = dynamic_cast<MemberAccessExpr*>(expr.get()))
            {
                auto call = std::make_unique<MethodCallExpr>();
                get_pos(call.get());
                call->method = mem->field;
                call->object = std::move(mem->object);

                if(!match(TokenType::RPAREN))
                {
                    while(true)
                    {
                        call->args.push_back(parse_expression());
                        if(failed) return nullptr;
                        if(!eat(TokenType::COMMA)) break;
                        if(match(TokenType::RPAREN)) break;
                    }
                }

                if(!eat(TokenType::RPAREN)) 
                { 
                    fail("expected ')' after arguments"); 
                    return nullptr; 
                }
                expr = std::move(call);
            }
            else if(auto* var = dynamic_cast<VariableExpr*>(expr.get()))
            {
                auto call = std::make_unique<CallExpr>();
                get_pos(call.get());
                call->name = var->name;

                if(!match(TokenType::RPAREN))
                {
                    while(true)
                    {
                        call->args.push_back(parse_expression());
                        if(failed) return nullptr;
                        if(!eat(TokenType::COMMA)) break;
                        if(match(TokenType::RPAREN)) break;
                    }
                }

                if(!eat(TokenType::RPAREN)) { fail("expected ')' after arguments"); return nullptr; }
                expr = std::move(call);
            }
            else
            {
                fail("call target must be identifier or member access");
                return nullptr;
            }

            continue;
        }

        if(eat(TokenType::COLONCOLON))
        {
            Type t = parse_type();
            if(failed) return nullptr;
            auto node = std::make_unique<CastExpr>();
            node->expr = std::move(expr);
            node->target_type = t;
            expr = std::move(node);
            continue;
        }

        break;
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parse_primary()
{
    if(match(TokenType::INT_LITERAL))
    {
        auto node = std::make_unique<LiteralExpr>();
        get_pos(node.get());
        node->value = static_cast<int64_t>(std::stoll(cur()->lexeme));

        next();
        return node;
    }

    if(match(TokenType::FLOAT_LITERAL))
    {
        auto node = std::make_unique<LiteralExpr>();
        get_pos(node.get());
        node->value = std::stod(cur()->lexeme);

        next();
        return node;
    }

    if(match(TokenType::STRING_LITERAL))
    {
        auto node = std::make_unique<LiteralExpr>();
        get_pos(node.get());
        node->value = cur()->lexeme;

        next();
        return node;
    }

    if(match(TokenType::CHAR_LITERAL))
    {
        auto node = std::make_unique<LiteralExpr>();
        get_pos(node.get());
        node->value = cur()->lexeme.empty() ? '\0' : cur()->lexeme[0];

        next();
        return node;
    }

    if(match(TokenType::TRUE))
    {
        auto node = std::make_unique<LiteralExpr>();
        get_pos(node.get());
        node->value = true;

        next();
        return node;
    }

    if(match(TokenType::FALSE))
    {
        auto node = std::make_unique<LiteralExpr>();
        get_pos(node.get());
        node->value = false;

        next();
        return node;
    }

    if(match(TokenType::NONE))
    {
        next();
        auto none = std::make_unique<EmptyExpr>();
        get_pos(none.get());
        return none;
    }

    if(match(TokenType::IDENTIFIER))
    {
        auto node = std::make_unique<VariableExpr>();
        get_pos(node.get());
        node->name = cur()->lexeme;

        next();
        return node;
    }

    if(eat(TokenType::LSQUAREPAREN))
    {
        auto arr = std::make_unique<ArrayLiteralExpr>();
        get_pos(arr.get());

        if(!match(TokenType::RSQUAREPAREN))
        {
            while(true)
            {
                arr->elements.push_back(parse_expression());

                if(failed) return nullptr;
                if(!eat(TokenType::COMMA)) break;
                if(match(TokenType::RSQUAREPAREN)) break;
            }
        }

        if(!eat(TokenType::RSQUAREPAREN))
        {
            fail("expected ']'");
            return nullptr;
        }

        return arr;
    }

    fail("expected expression");
    return nullptr;
}

std::unique_ptr<Expr> Parser::parse_group()
{
    if(!eat(TokenType::LPAREN))
    {
        fail("internal parser error");
        return nullptr;
    }

    auto expr = parse_expression();
    if(failed) return nullptr;

    if(!eat(TokenType::RPAREN))
    {
        fail("expected ')'");
        return nullptr;
    }

    return expr;
}