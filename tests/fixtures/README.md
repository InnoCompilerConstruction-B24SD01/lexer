# Fixture provenance

`project_d_example_programs.md` is an unchanged copy of the supplied **Project D —
Example Programs** document from `project-d-examples/project_d_example_programs.md`.
Integration tests extract the first source block under each numbered heading
(examples 1–15), the initial function-declaration clarification, and the six final
function snippets. Expected runtime output blocks are excluded. Indentation used
only to nest the final Markdown snippets is removed before tokenization.

The `specification/` files transcribe the concrete source examples from the supplied
**Project D.pdf**, with indentation normalized:

- `loops.d`: page 3, infinite loop, range loop without a variable, and array loop.
- `sparse_array.d`: page 5, sparse array holding numbers, a lambda, and a tuple.
- `tuples.d`: page 5, tuple construction, concatenation, named and positional access.

`expected_types.json` contains hand-authored complete token-type sequences for
all 25 source fixtures. In addition to these sequences, the integration tests
check exact lexemes against their reported source positions and ensure no source
is skipped except whitespace and `//` comments. Fixture success means lexical
success only; output values, scope, and runtime behavior are not evaluated.
