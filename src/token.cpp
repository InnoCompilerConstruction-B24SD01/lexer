#include "dlang/token.hpp"

#include <ostream>

namespace dlang {

std::string_view token_type_name(TokenType type) noexcept {
    switch (type) {
    case TokenType::EndOfFile: return "EOF";
    case TokenType::Newline: return "NEWLINE";
    case TokenType::Identifier: return "IDENTIFIER";
    case TokenType::IntegerLiteral: return "INTEGER";
    case TokenType::RealLiteral: return "REAL";
    case TokenType::StringLiteral: return "STRING";
    case TokenType::Var: return "VAR";
    case TokenType::If: return "IF";
    case TokenType::Then: return "THEN";
    case TokenType::Else: return "ELSE";
    case TokenType::End: return "END";
    case TokenType::While: return "WHILE";
    case TokenType::Loop: return "LOOP";
    case TokenType::For: return "FOR";
    case TokenType::In: return "IN";
    case TokenType::Exit: return "EXIT";
    case TokenType::Return: return "RETURN";
    case TokenType::Print: return "PRINT";
    case TokenType::Func: return "FUNC";
    case TokenType::Is: return "IS";
    case TokenType::True: return "TRUE";
    case TokenType::False: return "FALSE";
    case TokenType::None: return "NONE";
    case TokenType::IntType: return "INT_TYPE";
    case TokenType::RealType: return "REAL_TYPE";
    case TokenType::BoolType: return "BOOL_TYPE";
    case TokenType::StringType: return "STRING_TYPE";
    case TokenType::And: return "AND";
    case TokenType::Or: return "OR";
    case TokenType::Xor: return "XOR";
    case TokenType::Not: return "NOT";
    case TokenType::Assign: return "ASSIGN";
    case TokenType::Plus: return "PLUS";
    case TokenType::Minus: return "MINUS";
    case TokenType::Star: return "STAR";
    case TokenType::Slash: return "SLASH";
    case TokenType::Less: return "LESS";
    case TokenType::LessEqual: return "LESS_EQUAL";
    case TokenType::Greater: return "GREATER";
    case TokenType::GreaterEqual: return "GREATER_EQUAL";
    case TokenType::Equal: return "EQUAL";
    case TokenType::NotEqual: return "NOT_EQUAL";
    case TokenType::Arrow: return "ARROW";
    case TokenType::Range: return "RANGE";
    case TokenType::LeftParen: return "LEFT_PAREN";
    case TokenType::RightParen: return "RIGHT_PAREN";
    case TokenType::LeftBracket: return "LEFT_BRACKET";
    case TokenType::RightBracket: return "RIGHT_BRACKET";
    case TokenType::LeftBrace: return "LEFT_BRACE";
    case TokenType::RightBrace: return "RIGHT_BRACE";
    case TokenType::Comma: return "COMMA";
    case TokenType::Dot: return "DOT";
    case TokenType::Semicolon: return "SEMICOLON";
    }
    return "UNKNOWN";
}

std::ostream& operator<<(std::ostream& out, const Token& token) {
    out << token_type_name(token.type) << "(\"";
    // Escape only for display; Token::lexeme always contains the original bytes.
    constexpr char hex[] = "0123456789abcdef";
    for (unsigned char ch : token.lexeme) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20 || ch == 0x7f) {
                out << "\\u00" << hex[ch >> 4] << hex[ch & 0x0f];
            } else {
                out << static_cast<char>(ch);
            }
        }
    }
    return out << "\") " << token.line << ':' << token.column;
}

} // namespace dlang
