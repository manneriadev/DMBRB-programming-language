// token.hpp
#pragma once
#include <string>

enum class TokenType{
    // operators

    PLUS, // +
    MINUS, // - 
    STAR, // *
    STARSTAR, // ** (power)
    SLASH, // / 
    SLASHSLASH, // // 
    PERCENT, // %

    // assignment operators

    PLUSEQ, // +=
    MINUSEQ, // -=
    SLASHEQ, // /=
    PERCENTEQ, // %=
    AMPEQ, // &=
    PIPEEQ, // |=
    STAREQ, // *=
    EQ, // =

    // boolean 
    NOT, // !
    EQEQ, // ==
    NOTEQ, // !=
    GREATER, // >
    LESS, // <
    GREATEREQ, // >=
    LESSEQ, // <=
    AMPERAMPER, // &&
    PIPEPIPE, // ||

    // bit operators

    LESSLESS, // <<
    GREATERGREATER, // >>
    XOR, // ^
    PIPE, // |
    AMP, // &
    TILDA, // ~

    // separators and groups

    COMMA, // ,
    DOT, // .
    QUESTION, // ?
    COLON, // :
    COLONCOLON, // ::
    SEMICOLON, // ;
    LPAREN, // (
    RPAREN, // )
    LSQUAREPAREN, // [
    RSQUAREPAREN, // ]
    ARROW, // ->

    // names

    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,

    // special names

    IF,
    ELSE,
    FOR,
    WHILE,
    WHEN,
    RETURN,
    BREAK,
    CONTINUE,
    FUNCTION,
    BEGIN,
    END,
    TRUE,
    FALSE,
    VAR,
    CONST,
    MODULE,
    NONE,
    SELF,

    //types

    INT8, 
    INT16, 
    INT32, 
    INT64, 
    UINT8, 
    UINT16, 
    UINT32, 
    UINT64, 
    FLOAT32, 
    FLOAT64, 
    ARRAY, 
    VOID, 
    BOOL, 
    CHAR, 
    STRUCT,

    // other
    IMPORT,
    EXPORT,
    AS,

    NEWLINE,
    ERROR,
    END_TOKEN
};

struct Token{
    TokenType type;
    std::string lexeme;
    size_t line;
    size_t column;
};