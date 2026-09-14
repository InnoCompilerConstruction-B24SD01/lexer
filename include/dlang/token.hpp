#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <string_view>

namespace dlang {

enum class TokenType {
    EndOfFile,
    Newline,
    Identifier,
    IntegerLiteral,
    RealLiteral,
    StringLiteral,

    Var, If, Then, Else, End, While, Loop, For, In, Exit, Return, Print,
    Func, Is, True, False, None,
    IntType, RealType, BoolType, StringType,
    And, Or, Xor, Not,

    Assign,
    Plus, Minus, Star, Slash,
    Less, LessEqual, Greater, GreaterEqual, Equal, NotEqual,
    Arrow, Range,

    LeftParen, RightParen,
    LeftBracket, RightBracket,
    LeftBrace, RightBrace,
    Comma, Dot, Semicolon,
};

struct Token {
    TokenType type;
    std::string lexeme; // Exact source spelling, including a string's quotes.
    std::size_t line;   // One-based.
    std::size_t column; // One-based byte column; a tab counts as one column.
};

std::string_view token_type_name(TokenType type) noexcept;
std::ostream& operator<<(std::ostream& out, const Token& token);

} // namespace dlang
