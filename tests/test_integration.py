"""Exercise the actual CLI using source blocks from the supplied examples.

Only source blocks are selected: the document's expected interpreter output is
not D source. Expected token-type sequences are hand-authored fixtures.
"""

import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import textwrap
import unittest


BINARY = Path(sys.argv.pop(1) if len(sys.argv) > 1 else "./dlang").resolve()
FIXTURES = Path(__file__).resolve().parent / "fixtures"
CODE_BLOCK = re.compile(
    r"^(?P<indent> *)```text\n(?P<code>.*?)^(?P=indent)```[ \t]*$",
    re.MULTILINE | re.DOTALL,
)
TOKEN_LINE = re.compile(r'([A-Z_]+)\(("(?:[^"\\]|\\.)*")\) ([1-9][0-9]*):([1-9][0-9]*)')
TRIVIA = re.compile(rb"(?:[ \t\r\n\v\f]|//[^\r\n]*)*")


def prepared_programs():
    document = (FIXTURES / "project_d_example_programs.md").read_text(encoding="utf-8")
    headings = list(re.finditer(r"^## ([0-9]+)\. [^\n]+$", document, re.MULTILINE))
    if [int(heading[1]) for heading in headings] != list(range(1, 16)):
        raise ValueError("Expected exactly the 15 numbered Project D programs")

    programs = {}
    for index, heading in enumerate(headings):
        end = headings[index + 1].start() if index + 1 < len(headings) else len(document)
        blocks = list(CODE_BLOCK.finditer(document[heading.end():end]))
        if len(blocks) < 2:
            raise ValueError("Each numbered program must have source and expected output")
        programs["example_{:02d}".format(int(heading[1]))] = blocks[0]["code"]

    clarification = list(CODE_BLOCK.finditer(document[:headings[0].start()]))
    if len(clarification) != 1:
        raise ValueError("Expected the function-declaration clarification snippet")
    programs["clarification"] = clarification[0]["code"]

    function_section = document.split("# Implementation-Relevant Function Cases\n", 1)[1]
    functions = list(CODE_BLOCK.finditer(function_section))
    if len(functions) != 6:
        raise ValueError("Expected six additional function cases")
    for number, block in enumerate(functions, 1):
        programs["function_{}".format(number)] = textwrap.dedent(block["code"])

    for fixture in sorted((FIXTURES / "specification").glob("*.d")):
        programs["spec_" + fixture.stem] = fixture.read_text(encoding="utf-8")
    return programs


PROGRAMS = prepared_programs()
EXPECTED_TYPES = json.loads((FIXTURES / "expected_types.json").read_text(encoding="utf-8"))
if set(PROGRAMS) != set(EXPECTED_TYPES):
    raise ValueError("Every source fixture must have an expected token sequence")


class CliTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="dlang tests ")
        self.addCleanup(self.directory.cleanup)
        self.source_path = Path(self.directory.name) / "source file.d"

    def run_cli(self, *arguments):
        return subprocess.run([str(BINARY), *map(str, arguments)], capture_output=True, timeout=10)

    def run_source(self, source):
        self.source_path.write_bytes(source.encode("utf-8") if isinstance(source, str) else source)
        return self.run_cli(self.source_path)

    def check_source(self, source, expected_types):
        result = self.run_source(source)
        self.assertEqual(result.returncode, 0, result.stderr.decode("utf-8"))
        self.assertEqual(result.stderr, b"")
        source_bytes = source.encode("utf-8")
        line_starts = [0] + [match.end() for match in re.finditer(rb"\r\n|\r|\n", source_bytes)]
        types = []
        lexemes = []
        cursor = 0
        for output_line in result.stdout.decode("utf-8").splitlines():
            match = TOKEN_LINE.fullmatch(output_line)
            self.assertIsNotNone(match, "Invalid token output: " + output_line)
            token_type, quoted_lexeme, line, column = match.groups()
            lexeme = json.loads(quoted_lexeme)
            token_bytes = lexeme.encode("utf-8")
            line, column = int(line), int(column)
            self.assertLessEqual(line, len(line_starts))
            offset = line_starts[line - 1] + column - 1
            if line < len(line_starts):
                self.assertLess(offset, line_starts[line])
            self.assertGreaterEqual(offset, cursor)
            self.assertTrue(token_bytes, "The CLI must omit EOF")
            self.assertTrue(source_bytes.startswith(token_bytes, offset), output_line)
            self.assertIsNotNone(TRIVIA.fullmatch(source_bytes[cursor:offset]),
                                 "Non-trivia source was skipped before " + output_line)
            cursor = offset + len(token_bytes)
            types.append(token_type)
            lexemes.append(lexeme)
        self.assertIsNotNone(TRIVIA.fullmatch(source_bytes[cursor:]), "Unconsumed source")
        self.assertEqual(types, expected_types.split())
        return lexemes

    def test_exact_requested_format(self):
        result = self.run_source("var x := 10\n")
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stderr, b"")
        self.assertEqual(result.stdout, b'VAR("var") 1:1\nIDENTIFIER("x") 1:5\n'
                                       b'ASSIGN(":=") 1:7\nINTEGER("10") 1:10\n')

    def test_empty_input_and_comments(self):
        for source in ["", " \t\r\n", "// comment without final newline"]:
            with self.subTest(source=source):
                result = self.run_source(source)
                self.assertEqual((result.returncode, result.stdout, result.stderr), (0, b"", b""))

    def test_crlf_tabs_and_comments(self):
        self.check_source("// heading\r\n\tprint 'hi'\rvar x:=1..5 // tail",
                          "PRINT STRING VAR IDENTIFIER ASSIGN INTEGER RANGE INTEGER")

    def test_multiline_string_output_is_one_line(self):
        source = "\"line one\nline two\\path\t'\""
        self.assertEqual(self.check_source(source, "STRING"), [source])

    def test_errors_include_positions_and_no_partial_dump(self):
        cases = [(b"var x:=1\n  @", "2:3", "unexpected character '@'"),
                 (b"var x : 1", "1:7", "expected '=' after ':'"),
                 (b"print\n  'unfinished", "2:3", "unterminated string literal"),
                 (b"var x:=1\x00", "1:9", "unexpected byte 0x00")]
        for source, position, message in cases:
            with self.subTest(source=source):
                result = self.run_source(source)
                self.assertEqual(result.returncode, 1)
                self.assertEqual(result.stdout, b"")
                diagnostic = result.stderr.decode("utf-8")
                self.assertIn(str(self.source_path), diagnostic)
                self.assertIn("lexical error at " + position, diagnostic)
                self.assertIn(message, diagnostic)

    def test_missing_file(self):
        result = self.run_cli(self.source_path)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, b"")
        self.assertIn(b"cannot open source file", result.stderr)

    def test_directory_is_not_a_source_file(self):
        result = self.run_cli(self.directory.name)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout, b"")
        self.assertIn(b"source file", result.stderr)

    def test_wrong_argument_count(self):
        for arguments in [(), ("first.d", "second.d")]:
            with self.subTest(arguments=arguments):
                result = self.run_cli(*arguments)
                self.assertEqual(result.returncode, 2)
                self.assertEqual(result.stdout, b"")
                self.assertIn(b"Usage:", result.stderr)


def fixture_test(name, source):
    def test(self):
        self.check_source(source, EXPECTED_TYPES[name])
    return test


for name, source in PROGRAMS.items():
    setattr(CliTests, "test_fixture_" + name, fixture_test(name, source))


if __name__ == "__main__":
    unittest.main(verbosity=2)
