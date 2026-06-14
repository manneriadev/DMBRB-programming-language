// lexer.cpp

#include "Lexer.hpp"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

static std::unordered_map<std::string, TokenType> keywords = {
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"for", TokenType::FOR},
    {"while", TokenType::WHILE},
    {"when", TokenType::WHEN},
    {"return", TokenType::RETURN},
    {"break", TokenType::BREAK},
    {"continue", TokenType::CONTINUE},
    {"function", TokenType::FUNCTION},
    {"begin", TokenType::BEGIN},
    {"end", TokenType::END},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"var", TokenType::VAR},
    {"const", TokenType::CONST},
    {"module", TokenType::MODULE},
    {"none", TokenType::NONE},
    {"self", TokenType::SELF},

    {"int8", TokenType::INT8},
    {"int16", TokenType::INT16},
    {"int32", TokenType::INT32},
    {"int64", TokenType::INT64},
    {"int", TokenType::INT64},

    {"uint8", TokenType::UINT8},
    {"uint16", TokenType::UINT16},
    {"uint32", TokenType::UINT32},
    {"uint64", TokenType::UINT64},

    {"float32", TokenType::FLOAT32},
    {"float64", TokenType::FLOAT64},
    {"float", TokenType::FLOAT64},

    {"bool", TokenType::BOOL},
    {"char", TokenType::CHAR},
    {"void", TokenType::VOID},
    {"struct", TokenType::STRUCT},

    {"import", TokenType::IMPORT},
    {"export", TokenType::EXPORT},
    {"as",     TokenType::AS},
};

void Lexer::push_index(size_t n)
{
    this->pos += n;
    this->column += n;
}

char Lexer::peek() const
{
    if(pos >= source.size()) return '\0';
    return source[pos];
}

char Lexer::peek_next() const
{
    if(pos + 1 >= source.size()) return '\0';
    return source[pos + 1];
}

bool Lexer::is_eof() const
{
    return pos >= source.size();
}

bool Lexer::is_ident_start(char c) const
{
    return std::isalpha((unsigned char)c) || c == '_';
}

bool Lexer::is_ident(char c) const
{
    return std::isalnum((unsigned char)c) || c == '_';
}

void Lexer::skip_space()
{
    while(!is_eof() && std::isspace((unsigned char)peek()))
    {
        if(peek() == '\n')
        {
            column = 1;
            line += 1;
        }
        push_index(1);
    }
}

void Lexer::skip_comment()
{
    while(!is_eof() && peek() != '\n') push_index(1);
}

std::vector<Token> Lexer::Tokenize()
{
    std::vector<Token> tokens;
    while(true)
    {
        while(!is_eof() && (peek() == ' ' || peek() == '\t' || peek() == '\r'))
            push_index(1);

        if(is_eof()) break;

        if(peek() == '\n')  // add this for all expr to help divide pointer and expr
        {
            line++;
            column = 1;
            push_index(1);

            if(!tokens.empty())
            {
                TokenType last = tokens.back().type;
                if(last == TokenType::IDENTIFIER  ||
                   last == TokenType::INT_LITERAL  ||
                   last == TokenType::FLOAT_LITERAL||
                   last == TokenType::STRING_LITERAL||
                   last == TokenType::CHAR_LITERAL ||
                   last == TokenType::TRUE         ||
                   last == TokenType::FALSE        ||
                   last == TokenType::NONE         ||
                   last == TokenType::RPAREN       ||
                   last == TokenType::RSQUAREPAREN)
                {
                    tokens.push_back({TokenType::NEWLINE, "\\n", line, column});
                }
            }
            continue;
        }

        if(peek() == '#')
        {
            push_index(1);
            while(!is_eof() && peek() != '\n') push_index(1);
            continue;
        }

        if(peek() == '/' && peek_next() == '*')
        {
            push_index(2);
            int depth = 1;
            while(!is_eof() && depth > 0)
            {
                if(peek() == '/' && peek_next() == '*') { push_index(2); ++depth; }
                else if(peek() == '*' && peek_next() == '/') { push_index(2); --depth; }
                else
                {
                    if(peek() == '\n') { line++; column = 1; }
                    push_index(1);
                }
            }
            if(depth != 0) throw std::runtime_error("unclosed block comment at line " + std::to_string(line));
            continue;
        }

        Token token = next_token();
        if(token.type == TokenType::ERROR) throw std::runtime_error("unexpected character at line " + std::to_string(line) + " column " + std::to_string(column) + " -> '" + token.lexeme + "'");

        tokens.push_back(token);
    }
    tokens.push_back({TokenType::END_TOKEN, "", line, column});
    return tokens;
}

Token Lexer::next_token()
{
    skip_space();
    if(is_eof()) return make_token(TokenType::END_TOKEN, "");

    char c = peek();

    if(is_ident_start(c)) return extract_identifier_or_keyword();
    if(std::isdigit((unsigned char)c)) return extract_number();
    if(c == '"') return extract_string();
    if(c == '\'') return extract_char();

    return extract_operator();
}

Token Lexer::extract_identifier_or_keyword()
{
    size_t start = pos;

    while(!is_eof() && is_ident(peek())) push_index(1);

    std::string word = std::string(source.substr(start, pos - start));

    auto it = keywords.find(word);
    if(it != keywords.end()) return make_token(it->second, word);
    return make_token(TokenType::IDENTIFIER, word);
}

Token Lexer::extract_number()
{
    size_t start = pos;
    bool dot = false;

    while(!is_eof())
    {
        char c = peek();
        if(std::isdigit((unsigned char)c))
        {
            push_index(1);
        }
        else if(c == '.' && !dot && std::isdigit((unsigned char)peek_next()))
        {
            dot = true;
            push_index(1);
        }
        else break;
    }

    std::string num = std::string(source.substr(start, pos - start));
    if(dot) return make_token(TokenType::FLOAT_LITERAL, num);
    return make_token(TokenType::INT_LITERAL, num);
}

Token Lexer::extract_string()
{
    push_index(1);

    size_t start = pos;
    while(!is_eof() && peek() != '"')
    {
        if(peek() == '\\') push_index(2);
        else push_index(1);
    }

    if(is_eof()) return make_token(TokenType::ERROR, "unclosed string literal");

    std::string str = std::string(source.substr(start, pos - start));
    push_index(1);

    return make_token(TokenType::STRING_LITERAL, str);
}

Token Lexer::extract_char()
{
    push_index(1);

    if(is_eof()) return make_token(TokenType::ERROR, "unclosed char literal");

    char value = peek();
    push_index(1);

    if(peek() != '\'') return make_token(TokenType::ERROR, "unclosed char literal");
    push_index(1);

    return make_token(TokenType::CHAR_LITERAL, std::string(1, value));
}

Token Lexer::extract_operator()
{
    char c = peek();
    char n = peek_next();

    if(c == ':' && n == ':') { push_index(2); return make_token(TokenType::COLONCOLON, "::"); }
    if(c == '*' && n == '*') { push_index(2); return make_token(TokenType::STARSTAR, "**"); }
    if(c == '/' && n == '/') { push_index(2); return make_token(TokenType::SLASHSLASH, "//"); }
    if(c == '<' && n == '=') { push_index(2); return make_token(TokenType::LESSEQ, "<="); }
    if(c == '>' && n == '=') { push_index(2); return make_token(TokenType::GREATEREQ, ">="); }
    if(c == '=' && n == '=') { push_index(2); return make_token(TokenType::EQEQ, "=="); }
    if(c == '!' && n == '=') { push_index(2); return make_token(TokenType::NOTEQ, "!="); }
    if(c == '&' && n == '&') { push_index(2); return make_token(TokenType::AMPERAMPER, "&&"); }
    if(c == '|' && n == '|') { push_index(2); return make_token(TokenType::PIPEPIPE, "||"); }
    if(c == '<' && n == '<') { push_index(2); return make_token(TokenType::LESSLESS, "<<"); }
    if(c == '>' && n == '>') { push_index(2); return make_token(TokenType::GREATERGREATER, ">>"); }
    if(c == '+' && n == '=') { push_index(2); return make_token(TokenType::PLUSEQ, "+="); }
    if(c == '-' && n == '=') { push_index(2); return make_token(TokenType::MINUSEQ, "-="); }
    if(c == '*' && n == '=') { push_index(2); return make_token(TokenType::STAREQ, "*="); }
    if(c == '/' && n == '=') { push_index(2); return make_token(TokenType::SLASHEQ, "/="); }
    if(c == '%' && n == '=') { push_index(2); return make_token(TokenType::PERCENTEQ, "%="); }
    if(c == '&' && n == '=') { push_index(2); return make_token(TokenType::AMPEQ, "&="); }
    if(c == '|' && n == '=') { push_index(2); return make_token(TokenType::PIPEEQ, "|="); }
    if(c == '-' && n == '>') { push_index(2); return make_token(TokenType::ARROW, "->"); }

    push_index(1);
    switch(c)
    {
        case '+': return make_token(TokenType::PLUS, "+");
        case '-': return make_token(TokenType::MINUS, "-");
        case '*': return make_token(TokenType::STAR, "*");
        case '/': return make_token(TokenType::SLASH, "/");
        case '%': return make_token(TokenType::PERCENT, "%");
        case '=': return make_token(TokenType::EQ, "=");
        case '!': return make_token(TokenType::NOT, "!");
        case '<': return make_token(TokenType::LESS, "<");
        case '>': return make_token(TokenType::GREATER, ">");
        case '&': return make_token(TokenType::AMP, "&");
        case '|': return make_token(TokenType::PIPE, "|");
        case '^': return make_token(TokenType::XOR, "^");
        case '~': return make_token(TokenType::TILDA, "~");
        case '?': return make_token(TokenType::QUESTION, "?");
        case ':': return make_token(TokenType::COLON, ":");
        case ';': return make_token(TokenType::SEMICOLON, ";");
        case ',': return make_token(TokenType::COMMA, ",");
        case '.': return make_token(TokenType::DOT, ".");
        case '(': return make_token(TokenType::LPAREN, "(");
        case ')': return make_token(TokenType::RPAREN, ")");
        case '[': return make_token(TokenType::LSQUAREPAREN,  "[");
        case ']': return make_token(TokenType::RSQUAREPAREN,  "]");
        default: return make_token(TokenType::ERROR, std::string(1, c));
    }
}

Token Lexer::make_token(TokenType type, const std::string& lexeme)
{
    return {type, lexeme, line, column};
}