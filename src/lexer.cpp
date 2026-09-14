#include "dlang/lexer.hpp"

#include <string_view>
#include <unordered_map>
#include <utility>

namespace dlang {
namespace {

// ASCII rules are explicit so lexing does not change with the process locale.
bool is_digit(char ch) noexcept {
    return ch >= '0' && ch <= '9';
}

bool is_identifier_start(char ch) noexcept {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

TokenType identifier_type(std::string_view text) {
    static const std::unordered_map<std::string_view, TokenType> keywords{
        {"var", TokenType::Var}, {"if", TokenType::If},
        {"then", TokenType::Then}, {"else", TokenType::Else},
        {"end", TokenType::End}, {"while", TokenType::While},
        {"loop", TokenType::Loop}, {"for", TokenType::For},
        {"in", TokenType::In}, {"exit", TokenType::Exit},
        {"return", TokenType::Return}, {"print", TokenType::Print},
        {"func", TokenType::Func}, {"is", TokenType::Is},
        {"true", TokenType::True}, {"false", TokenType::False},
        {"none", TokenType::None}, {"int", TokenType::IntType},
        {"real", TokenType::RealType}, {"bool", TokenType::BoolType},
        {"string", TokenType::StringType}, {"and", TokenType::And},
        {"or", TokenType::Or}, {"xor", TokenType::Xor},
        {"not", TokenType::Not},
    };
    const auto found = keywords.find(text);
    return found == keywords.end() ? TokenType::Identifier : found->second;
}

std::string unexpected_character(char ch) {
    const auto byte = static_cast<unsigned char>(ch);
    if (byte >= 0x20 && byte <= 0x7e) {
        return std::string("unexpected character '") + ch + "'";
    }
    constexpr char hex[] = "0123456789ABCDEF";
    return std::string("unexpected byte 0x") + hex[byte >> 4] + hex[byte & 0x0f];
}

} // namespace

LexicalError::LexicalError(std::size_t line, std::size_t column,
                         const std::string& message)
    : std::runtime_error("lexical error at " + std::to_string(line) + ':' +
                         std::to_string(column) + ": " + message),
      line_(line), column_(column) {}

Lexer::Lexer(std::string source, NewlineMode newlines)
    : source_(std::move(source)), newlines_(newlines) {}

bool Lexer::at_end() const noexcept {
    return offset_ == source_.size();
}

char Lexer::peek(std::size_t lookahead) const noexcept {
    return lookahead < source_.size() - offset_ ? source_[offset_ + lookahead] : '\0';
}

char Lexer::advance() {
    const char ch = source_[offset_++];
    if (ch == '\r') {
        // Treat CRLF as one newline while preserving both bytes in lexemes.
        if (!at_end() && peek() == '\n') {
            ++offset_;
        }
        ++line_;
        column_ = 1;
    } else if (ch == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return ch;
}

bool Lexer::match(char expected) {
    if (at_end() || peek() != expected) {
        return false;
    }
    advance();
    return true;
}

void Lexer::skip_trivia() {
    while (!at_end()) {
        const char ch = peek();
        if (ch == ' ' || ch == '\t' || ch == '\v' || ch == '\f') {
            advance();
        } else if ((ch == '\n' || ch == '\r') && newlines_ == NewlineMode::Skip) {
            advance();
        } else if (ch == '/' && peek(1) == '/') {
            advance();
            advance();
            while (!at_end() && peek() != '\n' && peek() != '\r') {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::make_token(TokenType type, std::size_t start,
                        std::size_t line, std::size_t column) const {
    return {type, source_.substr(start, offset_ - start), line, column};
}

Token Lexer::next() {
    skip_trivia();
    const std::size_t start = offset_;
    const std::size_t line = line_;
    const std::size_t column = column_;
    if (at_end()) {
        return {TokenType::EndOfFile, "", line, column};
    }

    const char ch = advance();
    if (is_identifier_start(ch)) {
        while (is_identifier_start(peek()) || is_digit(peek())) {
            advance();
        }
        const auto text = std::string_view(source_).substr(start, offset_ - start);
        return make_token(identifier_type(text), start, line, column);
    }

    if (is_digit(ch)) {
        while (is_digit(peek())) {
            advance();
        }
        TokenType type = TokenType::IntegerLiteral;
        // A decimal point must be followed by a digit. In particular, the first
        // dot in 1..5 stays available for RANGE, and .3 stays DOT + INTEGER.
        if (peek() == '.' && is_digit(peek(1))) {
            type = TokenType::RealLiteral;
            advance();
            while (is_digit(peek())) {
                advance();
            }
        }
        return make_token(type, start, line, column);
    }

    if (ch == '\'' || ch == '"') {
        // The specification defines arbitrary characters between matching
        // quotes, with no escapes. Backslashes and newlines are literal data.
        while (!at_end() && peek() != ch) {
            advance();
        }
        if (at_end()) {
            throw LexicalError(line, column, "unterminated string literal; expected closing " +
                               std::string(1, ch));
        }
        advance();
        return make_token(TokenType::StringLiteral, start, line, column);
    }

    TokenType type;
    switch (ch) {
    case '\n': case '\r': type = TokenType::Newline; break;
    case ':':
        if (!match('=')) {
            throw LexicalError(line, column, "expected '=' after ':' to form ':='");
        }
        type = TokenType::Assign;
        break;
    case '+': type = TokenType::Plus; break;
    case '-': type = TokenType::Minus; break;
    case '*': type = TokenType::Star; break;
    case '/': type = match('=') ? TokenType::NotEqual : TokenType::Slash; break;
    case '<': type = match('=') ? TokenType::LessEqual : TokenType::Less; break;
    case '>': type = match('=') ? TokenType::GreaterEqual : TokenType::Greater; break;
    case '=': type = match('>') ? TokenType::Arrow : TokenType::Equal; break;
    case '.': type = match('.') ? TokenType::Range : TokenType::Dot; break;
    case '(': type = TokenType::LeftParen; break;
    case ')': type = TokenType::RightParen; break;
    case '[': type = TokenType::LeftBracket; break;
    case ']': type = TokenType::RightBracket; break;
    case '{': type = TokenType::LeftBrace; break;
    case '}': type = TokenType::RightBrace; break;
    case ',': type = TokenType::Comma; break;
    case ';': type = TokenType::Semicolon; break;
    default: throw LexicalError(line, column, unexpected_character(ch));
    }
    return make_token(type, start, line, column);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    do {
        tokens.push_back(next());
    } while (tokens.back().type != TokenType::EndOfFile);
    return tokens;
}

} // namespace dlang
