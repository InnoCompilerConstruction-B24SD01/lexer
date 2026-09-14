# Project D lexer

A hand-written C++17 lexer for **Project D — Dynamic Language**. This project
implements **source code → tokens** only. Parsing, AST construction, semantic
analysis, and interpretation are future stages. No lexer generators or external
C++ libraries are used.

## Build, run, and test

Building requires a C++17 compiler, GNU Make, and `ar`. The integration tests also
require Python 3 and use only its standard library.

```sh
make
./dlang examples/example.d
make test
```

To scan your own file:

```sh
./dlang example.d
```

For source `var x := 10`, the output is:

```text
VAR("var") 1:1
IDENTIFIER("x") 1:5
ASSIGN(":=") 1:7
INTEGER("10") 1:10
```

Each line shows the token type, original lexeme, and starting `line:column`.
String delimiters remain part of the lexeme. Quotes, backslashes, and control
characters are escaped **for display only**, keeping each token on one output line.
The CLI omits whitespace, comments, and the final EOF token.

Errors go to standard error, for example:

```text
example.d: lexical error at 2:3: unexpected character '@'
```

Exit status is `0` on success, `1` for lexical or file I/O errors, and `2` for
incorrect arguments. Invalid source produces no partial token dump.
Use `make clean` to remove build products.

## Lexical rules

The language rules come from the supplied **Project D.pdf** and
**Project D — Example Programs** document.

- Integers are decimal digit sequences; reals are `digits.digits`. Signs are
  separate tokens, and literal values are not converted or range-checked.
- A decimal point is consumed into a number only when followed by a digit:
  `1..5` becomes `INTEGER RANGE INTEGER`, `point.3` becomes
  `IDENTIFIER DOT INTEGER`, and `3.5` is one `REAL` token.
- Strings use matching single or double quotes. The PDF describes arbitrary
  characters between quotes and defines no escape sequences, so backslashes,
  the other quote character, and newlines are literal contents. An unclosed
  string reports its opening quote's position.
- Keywords match whole identifiers, case-sensitively. The PDF does not give a
  character-level grammar for `IDENT`; this implementation uses the conventional
  ASCII rule `[A-Za-z_][A-Za-z0-9_]*`. String contents retain arbitrary bytes.
- `//` starts a comment outside strings and runs to newline or EOF.
- Multi-character operators use longest match. A lone `:` is a lexical error.
  No exponent notation, numeric base prefixes, escapes, or block comments are
  introduced. Sequences of otherwise valid tokens are left for the future parser
  to validate; for example, `1e3` yields `INTEGER("1") IDENTIFIER("e3")`.
- Positions start at `1:1`. LF, CRLF, and lone CR each advance one line. Columns
  count bytes; a tab advances one column. Original line-ending bytes are preserved
  inside string and optional newline tokens.

All supported keywords are:

```text
var if then else end while loop for in exit return print func is
true false none int real bool string and or xor not
```

All supported operators and punctuation are:

```text
:= + - * / < <= > >= = /= => ..
( ) [ ] { } , . ;
```

Literal token names are `INTEGER`, `REAL`, and `STRING`. The type keywords use
`INT_TYPE`, `REAL_TYPE`, `BOOL_TYPE`, and `STRING_TYPE` to distinguish them from
literals. Boolean literals and `none` have their own keyword token types.

## Design and parser API

```text
include/dlang/token.hpp   Token types, owning lexemes, positions, output formatting
include/dlang/lexer.hpp   Lexer and LexicalError public API
src/lexer.cpp            Manual scanner with bounded lookahead and keyword lookup
src/token.cpp            Readable token formatting
src/main.cpp             File-reading CLI
tests/lexer_tests.cpp    C++ unit tests
tests/test_integration.py  CLI and supplied-example integration tests
tests/fixtures/          Original examples, PDF snippets, expected token sequences
```

The scanner tracks a byte offset, line, and column. It skips trivia, then recognizes
identifiers/keywords, numbers, strings, or operators and punctuation. Tokens own
their lexemes, and the lexer owns its source, so both are safe to use without
keeping the caller's input buffer alive. Scanning is linear in the input size.

`make` also builds `build/libdlang_lexer.a` for reuse independently of the CLI.
For example:

```cpp
#include "dlang/lexer.hpp"

dlang::Lexer lexer("var x := 10\n", dlang::NewlineMode::Emit);
dlang::Token first = lexer.next();     // VAR, "var", line 1, column 1
auto remaining = lexer.tokenize();     // Remaining tokens, ending with EOF
```

Use `next()` for incremental consumption or `tokenize()` for all remaining tokens.
The latter includes exactly one `TokenType::EndOfFile` with an empty lexeme at
the end-of-input position. Repeated `next()` calls after exhaustion return that
same EOF token. Errors throw `LexicalError`, whose `what()`, `line()`, and
`column()` expose the diagnostic; stop scanning that input after an error.

By default, `NewlineMode::Skip` discards newlines. A future parser can use
`NewlineMode::Emit` to retain `TokenType::Newline`, since the PDF allows newlines
as statement separators. This also retains newlines following comments, while
newlines inside strings remain part of the string token.

## Test coverage

`make test` runs 17 C++ unit-test groups and 33 integration/CLI tests. Coverage
includes all literals, keywords, operators and punctuation, longest match,
comments, positions, invalid input, EOF behavior, and source/token ownership.

Integration tests read the unchanged supplied Markdown document and tokenize all
15 prepared programs, its initial function clarification, and six additional
function snippets. Three more fixtures cover the PDF's loop, sparse-array, and
tuple examples. Each of these 25 fixtures checks its complete expected token-type
sequence, exact lexemes and positions, and source coverage. The remaining tests
check CLI formatting, newlines, error reporting, and file/argument failures.

See [fixture provenance](tests/fixtures/README.md) for source details. These tests
verify tokenization of functions, lambdas, closures, arrays, tuples, and loops;
they do not execute the example programs or compare runtime output.
