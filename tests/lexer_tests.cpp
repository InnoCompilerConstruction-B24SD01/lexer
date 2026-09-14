#include "dlang/lexer.hpp"

#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

using T = dlang::TokenType;
using Spelling = std::pair<T, std::string>;

#define CHECK(condition) do { \
    if (!(condition)) { \
        throw std::runtime_error(std::string(__FILE__) + ':' + \
            std::to_string(__LINE__) + ": " #condition); \
    } \
} while (false)

void check_token(const dlang::Token& actual, const dlang::Token& expected) {
    if (actual.type != expected.type || actual.lexeme != expected.lexeme ||
        actual.line != expected.line || actual.column != expected.column) {
        std::ostringstream message;
        message << "expected " << expected << ", got " << actual;
        throw std::runtime_error(message.str());
    }
}

void expect_spellings(const std::string& source, const std::vector<Spelling>& expected,
                      dlang::NewlineMode mode = dlang::NewlineMode::Skip) {
    const auto tokens = dlang::Lexer(source, mode).tokenize();
    CHECK(tokens.size() == expected.size() + 1);
    for (std::size_t i = 0; i < expected.size(); ++i) {
        CHECK(tokens[i].type == expected[i].first);
        CHECK(tokens[i].lexeme == expected[i].second);
    }
    CHECK(tokens.back().type == T::EndOfFile);
    CHECK(tokens.back().lexeme.empty());
}

void expect_separated(const std::vector<Spelling>& expected) {
    std::string source;
    for (const auto& entry : expected) {
        source += entry.second + ' ';
    }
    const auto tokens = dlang::Lexer(source).tokenize();
    CHECK(tokens.size() == expected.size() + 1);
    std::size_t column = 1;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        check_token(tokens[i], {expected[i].first, expected[i].second, 1, column});
        CHECK(dlang::token_type_name(tokens[i].type) != "UNKNOWN");
        column += expected[i].second.size() + 1;
    }
    check_token(tokens.back(), {T::EndOfFile, "", 1, column});
}

void expect_error(const std::string& source, std::size_t line,
                  std::size_t column, const std::string& message_part) {
    try {
        dlang::Lexer(source).tokenize();
    } catch (const dlang::LexicalError& error) {
        CHECK(error.line() == line);
        CHECK(error.column() == column);
        const std::string message = error.what();
        CHECK(message.find("lexical error at " + std::to_string(line) + ':' +
                           std::to_string(column)) != std::string::npos);
        CHECK(message.find(message_part) != std::string::npos);
        return;
    }
    throw std::runtime_error("expected a LexicalError");
}

void empty_and_whitespace() {
    dlang::Lexer empty("");
    check_token(empty.next(), {T::EndOfFile, "", 1, 1});
    check_token(empty.next(), {T::EndOfFile, "", 1, 1});
    const auto tokens = dlang::Lexer(" \t\v\f\r\n\n \t").tokenize();
    CHECK(tokens.size() == 1);
    check_token(tokens[0], {T::EndOfFile, "", 3, 3});
}

void identifiers() {
    expect_separated({
        {T::Identifier, "a"}, {T::Identifier, "A"}, {T::Identifier, "_"},
        {T::Identifier, "_value2"}, {T::Identifier, "realValue"},
        {T::Identifier, "point"}, {T::Identifier, "y2"},
        {T::Identifier, "Var"}, {T::Identifier, "TRUE"},
        {T::Identifier, "var1"}, {T::Identifier, "if_else"},
        {T::Identifier, "none_"}, {T::Identifier, "andrew"},
        {T::Identifier, "integer"}, {T::Identifier, "array"},
        {T::Identifier, "tuple"}, {T::Identifier, "function"},
    });
}

void keywords() {
    expect_separated({
        {T::Var, "var"}, {T::If, "if"}, {T::Then, "then"},
        {T::Else, "else"}, {T::End, "end"}, {T::While, "while"},
        {T::Loop, "loop"}, {T::For, "for"}, {T::In, "in"},
        {T::Exit, "exit"}, {T::Return, "return"}, {T::Print, "print"},
        {T::Func, "func"}, {T::Is, "is"}, {T::True, "true"},
        {T::False, "false"}, {T::None, "none"}, {T::IntType, "int"},
        {T::RealType, "real"}, {T::BoolType, "bool"},
        {T::StringType, "string"}, {T::And, "and"}, {T::Or, "or"},
        {T::Xor, "xor"}, {T::Not, "not"},
    });
}

void decimal_literals() {
    expect_separated({
        {T::IntegerLiteral, "0"}, {T::IntegerLiteral, "10"},
        {T::IntegerLiteral, "000123"}, {T::RealLiteral, "3.5"},
        {T::RealLiteral, "0.0"}, {T::RealLiteral, "00.0250"},
        {T::IntegerLiteral, std::string(1000, '9')},
        {T::RealLiteral, std::string(1000, '9') + "." + std::string(1000, '1')},
    });
    expect_spellings("-12+3.5", {
        {T::Minus, "-"}, {T::IntegerLiteral, "12"},
        {T::Plus, "+"}, {T::RealLiteral, "3.5"},
    });
}

void range_dot_and_real() {
    expect_spellings("1..5 point.3 3.5 1.0..5.25 1...5", {
        {T::IntegerLiteral, "1"}, {T::Range, ".."}, {T::IntegerLiteral, "5"},
        {T::Identifier, "point"}, {T::Dot, "."}, {T::IntegerLiteral, "3"},
        {T::RealLiteral, "3.5"}, {T::RealLiteral, "1.0"}, {T::Range, ".."},
        {T::RealLiteral, "5.25"}, {T::IntegerLiteral, "1"},
        {T::Range, ".."}, {T::Dot, "."}, {T::IntegerLiteral, "5"},
    });
}

void only_specified_number_forms() {
    // These are separate legal tokens, not added exponent/hex/fraction syntax.
    // Whether their adjacency forms a valid expression belongs to the parser.
    expect_spellings("1e3 0x10 .5 5. 1_000", {
        {T::IntegerLiteral, "1"}, {T::Identifier, "e3"},
        {T::IntegerLiteral, "0"}, {T::Identifier, "x10"},
        {T::Dot, "."}, {T::IntegerLiteral, "5"},
        {T::IntegerLiteral, "5"}, {T::Dot, "."},
        {T::IntegerLiteral, "1"}, {T::Identifier, "_000"},
    });
}

void string_literals() {
    expect_separated({
        {T::StringLiteral, "''"}, {T::StringLiteral, "\"\""},
        {T::StringLiteral, "'Hello, D!'"}, {T::StringLiteral, "\"Hello, D!\""},
        {T::StringLiteral, "'say \"hello\"'"}, {T::StringLiteral, "\"it's D\""},
        {T::StringLiteral, "'// := .. {} []'"},
        {T::StringLiteral, "'back\\slash\\n\\t'"},
        {T::StringLiteral, "'tab\tinside'"},
        {T::StringLiteral, "'Привет'"},
        {T::StringLiteral, std::string("'a\0b'", 5)},
    });
    // A backslash does not escape a closing quote.
    expect_spellings("'a\\' x", {{T::StringLiteral, "'a\\'"}, {T::Identifier, "x"}});
}

void multiline_strings() {
    const std::string literal = "\"a\r\nb\rc\nd\"";
    const auto tokens = dlang::Lexer(literal + "\nx", dlang::NewlineMode::Emit).tokenize();
    CHECK(tokens.size() == 4);
    check_token(tokens[0], {T::StringLiteral, literal, 1, 1});
    check_token(tokens[1], {T::Newline, "\n", 4, 3});
    check_token(tokens[2], {T::Identifier, "x", 5, 1});
    check_token(tokens[3], {T::EndOfFile, "", 5, 2});
}

void operators_and_punctuation() {
    expect_separated({
        {T::Assign, ":="}, {T::Plus, "+"}, {T::Minus, "-"},
        {T::Star, "*"}, {T::Slash, "/"}, {T::Less, "<"},
        {T::LessEqual, "<="}, {T::Greater, ">"}, {T::GreaterEqual, ">="},
        {T::Equal, "="}, {T::NotEqual, "/="}, {T::Arrow, "=>"},
        {T::Range, ".."}, {T::LeftParen, "("}, {T::RightParen, ")"},
        {T::LeftBracket, "["}, {T::RightBracket, "]"},
        {T::LeftBrace, "{"}, {T::RightBrace, "}"},
        {T::Comma, ","}, {T::Dot, "."}, {T::Semicolon, ";"},
    });
}

void longest_match() {
    expect_spellings(":=<=>==>/=.../=// comment", {
        {T::Assign, ":="}, {T::LessEqual, "<="}, {T::GreaterEqual, ">="},
        {T::Arrow, "=>"}, {T::NotEqual, "/="}, {T::Range, ".."},
        {T::Dot, "."}, {T::NotEqual, "/="},
    });
    expect_spellings("/= / == => /", {
        {T::NotEqual, "/="}, {T::Slash, "/"}, {T::Equal, "="},
        {T::Equal, "="}, {T::Arrow, "=>"}, {T::Slash, "/"},
    });
}

void single_line_comments() {
    expect_spellings("// \" ' @ : comment\nvar// next\r\nx// last", {
        {T::Var, "var"}, {T::Identifier, "x"},
    });
    expect_spellings("// no trailing newline", {});
    expect_spellings("// first\r// second\n// third\r\nx", {
        {T::Newline, "\r"}, {T::Newline, "\n"}, {T::Newline, "\r\n"},
        {T::Identifier, "x"},
    }, dlang::NewlineMode::Emit);
    expect_spellings("/* text */", {
        {T::Slash, "/"}, {T::Star, "*"}, {T::Identifier, "text"},
        {T::Star, "*"}, {T::Slash, "/"},
    });
}

void positions_and_newlines() {
    const std::string source = "\tvar x:=1;\r\n// comment\rprint 'ok'\n\nx";
    const std::vector<dlang::Token> expected{
        {T::Var, "var", 1, 2}, {T::Identifier, "x", 1, 6},
        {T::Assign, ":=", 1, 7}, {T::IntegerLiteral, "1", 1, 9},
        {T::Semicolon, ";", 1, 10}, {T::Newline, "\r\n", 1, 11},
        {T::Newline, "\r", 2, 11}, {T::Print, "print", 3, 1},
        {T::StringLiteral, "'ok'", 3, 7}, {T::Newline, "\n", 3, 11},
        {T::Newline, "\n", 4, 1}, {T::Identifier, "x", 5, 1},
        {T::EndOfFile, "", 5, 2},
    };
    for (const auto mode : {dlang::NewlineMode::Skip, dlang::NewlineMode::Emit}) {
        const auto actual = dlang::Lexer(source, mode).tokenize();
        std::size_t index = 0;
        for (const auto& token : expected) {
            if (mode == dlang::NewlineMode::Skip && token.type == T::Newline) {
                continue;
            }
            CHECK(index < actual.size());
            check_token(actual[index++], token);
        }
        CHECK(index == actual.size());
    }
}

void invalid_characters() {
    for (const char ch : std::string("@!#%&|\\?")) {
        expect_error(std::string("var\n  ") + ch, 2, 3, "unexpected character");
    }
    expect_error(":", 1, 1, "expected '=' after ':'");
    expect_error("x : = 1", 1, 3, "expected '=' after ':'");
    expect_error("x\r\n \t@", 2, 3, "unexpected character '@'");
    expect_error(std::string("x\0y", 3), 1, 2, "unexpected byte 0x00");
    expect_error(std::string(1, static_cast<char>(0xff)), 1, 1, "unexpected byte 0xFF");
}

void unterminated_strings() {
    expect_error("'", 1, 1, "unterminated string literal");
    expect_error("\"", 1, 1, "unterminated string literal");
    expect_error("var x := 'hello", 1, 10, "expected closing '");
    expect_error("\n  \"hello\nworld", 2, 3, "expected closing \"");
    expect_error("print 'opposite\"", 1, 7, "unterminated string literal");
}

void incremental_api_and_ownership() {
    std::string source = "var x := 10";
    dlang::Lexer lexer(source);
    source.assign(100, '@');
    check_token(lexer.next(), {T::Var, "var", 1, 1});
    check_token(lexer.next(), {T::Identifier, "x", 1, 5});
    const auto remaining = lexer.tokenize();
    CHECK(remaining.size() == 3);
    check_token(remaining[0], {T::Assign, ":=", 1, 7});
    check_token(remaining[1], {T::IntegerLiteral, "10", 1, 10});
    check_token(remaining[2], {T::EndOfFile, "", 1, 12});
    check_token(lexer.next(), {T::EndOfFile, "", 1, 12});
    CHECK(lexer.tokenize().size() == 1);

    const auto owned_tokens = dlang::Lexer(std::string("'owned'")).tokenize();
    check_token(owned_tokens[0], {T::StringLiteral, "'owned'", 1, 1});
}

void lexer_does_not_parse() {
    expect_spellings("return func(]", {
        {T::Return, "return"}, {T::Func, "func"},
        {T::LeftParen, "("}, {T::RightBracket, "]"},
    });
}

void readable_output() {
    std::ostringstream output;
    output << dlang::Token{T::StringLiteral, "\"a\n\r\t\\\"", 2, 3};
    CHECK(output.str() == R"(STRING("\"a\n\r\t\\\"") 2:3)");
    std::ostringstream control_output;
    control_output << dlang::Token{T::StringLiteral, std::string("'\0\x7f'", 4), 1, 1};
    CHECK(control_output.str() == R"(STRING("'\u0000\u007f'") 1:1)");
}

} // namespace

int main() {
    const std::vector<std::pair<const char*, std::function<void()>>> tests{
        {"empty input and whitespace", empty_and_whitespace},
        {"identifiers and keyword boundaries", identifiers},
        {"all keywords", keywords},
        {"decimal literals and separate signs", decimal_literals},
        {"ranges, tuple access, and decimal points", range_dot_and_real},
        {"only specified number forms", only_specified_number_forms},
        {"both string delimiters and raw contents", string_literals},
        {"multiline strings preserve bytes and positions", multiline_strings},
        {"all operators and punctuation", operators_and_punctuation},
        {"longest match", longest_match},
        {"single-line comments", single_line_comments},
        {"positions and optional newline tokens", positions_and_newlines},
        {"invalid characters and incomplete assignment", invalid_characters},
        {"unterminated strings", unterminated_strings},
        {"incremental API, EOF, and ownership", incremental_api_and_ownership},
        {"no parsing or semantic checks", lexer_does_not_parse},
        {"readable escaped output", readable_output},
    };
    std::size_t failed = 0;
    for (const auto& test : tests) {
        try {
            test.second();
            std::cout << "PASS " << test.first << '\n';
        } catch (const std::exception& error) {
            ++failed;
            std::cerr << "FAIL " << test.first << ": " << error.what() << '\n';
        }
    }
    std::cout << tests.size() - failed << '/' << tests.size() << " unit tests passed\n";
    return failed == 0 ? 0 : 1;
}
