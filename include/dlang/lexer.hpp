#pragma once

#include "dlang/token.hpp"

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace dlang {

class LexicalError : public std::runtime_error {
public:
    LexicalError(std::size_t line, std::size_t column, const std::string& message);

    std::size_t line() const noexcept { return line_; }
    std::size_t column() const noexcept { return column_; }

private:
    std::size_t line_;
    std::size_t column_;
};

// The specification permits newlines as statement separators. A parser can
// request them; the token-dump CLI skips them along with other whitespace.
enum class NewlineMode { Skip, Emit };

class Lexer {
public:
    // Own the source so construction from a temporary string is safe.
    explicit Lexer(std::string source, NewlineMode newlines = NewlineMode::Skip);

    // Return one token. Once exhausted, repeatedly returns the same EOF token.
    // Throws LexicalError on the first invalid character or unclosed string.
    Token next();

    // Consume the remaining input, including exactly one final EOF token.
    std::vector<Token> tokenize();

private:
    bool at_end() const noexcept;
    char peek(std::size_t lookahead = 0) const noexcept;
    char advance();
    bool match(char expected);
    void skip_trivia();
    Token make_token(TokenType type, std::size_t start,
                     std::size_t line, std::size_t column) const;

    std::string source_;
    NewlineMode newlines_;
    std::size_t offset_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;
};

} // namespace dlang
