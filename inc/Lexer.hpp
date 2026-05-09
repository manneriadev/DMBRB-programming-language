// Lexer.hpp
#pragma once

#include <string_view> 
#include <vector>
#include "token.hpp"

class Lexer{
public:
    explicit Lexer(const std::string_view source) : source(source) {}
    ~Lexer() = default;
    std::vector<Token> Tokenize();

private:
    std::string_view source;
    size_t pos = 0;
    size_t line = 1;
    size_t column = 1;

    void push_index(size_t n);
    char peek() const;
    char peek_next() const;

    bool is_eof() const;
    bool is_nl() const;
    bool is_ident_start(char) const;
    bool is_ident(char c) const;
    
    void skip_space();
    void skip_comment();

    Token next_token();

    Token extract_identifier_or_keyword();
    Token extract_number();
    Token extract_string();
    Token extract_operator();
    Token extract_char();

    Token make_token(TokenType type, const std::string& lexeme);
};