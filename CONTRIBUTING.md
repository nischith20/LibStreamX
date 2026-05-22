# Contributing to LibStreamX

Thanks for your interest in contributing. The guidelines below keep
the review loop tight and the codebase healthy.

---

## Before you start

- Search the [issue tracker](../../issues) and open pull requests to
  avoid duplicate work.
- For non-trivial changes (new modules, API changes, on-the-wire
  format changes), open an issue first and agree on the approach
  before writing code.
- All contributions are released under the project's [MIT license](LICENSE).
  By submitting a PR you certify you have the right to license your
  contribution under those terms.

## One PR = one change

Keep pull requests focused. If you find a second issue while working,
open a second PR. This makes review, discussion, and bisecting
cleaner.

## What a good PR looks like

1. **Title:** `module: short description`, e.g.
   `parser: free payload_buf on reset path`.
2. **Body:**
   - **What** the change does, in one or two sentences.
   - **Why** it is needed — the bug, the use case, the motivation.
   - **How** to verify it — repro steps, a failing test, an ASan
     trace, or whatever evidence is appropriate.
3. **Code:**
   - Touch only what is needed for this change.
   - Don't reformat unrelated code.
   - Don't bundle drive-by edits in other modules.
4. **Tests:**
   - Add or update a unit test in `tests/test_<module>.c` that
     **fails before your change and passes after**.
   - Verify under sanitizers: `make asan && make test`.

## What gets pushed back

- Drive-by reformatting alongside a fix.
- Multiple unrelated changes in a single diff.
- Bug fixes without a regression test.
- Tests that simply pin existing behavior as the spec.
- Changes whose author cannot explain the root cause in their own
  words.

## Code style

- C99. No C++ features.
- 4-space indent, no tabs.
- Braces on the same line as the control statement.
- `snake_case` for functions and locals; `MACRO_CASE` for macros.
- Public API in `include/<module>.h`; implementation details stay in
  `src/`.
- Public symbols are documented with Doxygen comments in the header.

## Building locally

```
make            # normal build
make asan       # AddressSanitizer + UBSan build (recommended pre-PR)
make test       # run tests
```

The project also builds with CMake — see [README.md](README.md).

## Reporting bugs

Open an issue using the bug-report template. Include:

- LibStreamX version (commit hash or release tag).
- Compiler and platform.
- Minimal reproducer — ideally a few bytes of input and the expected
  vs. observed behavior.
- Sanitizer output if available.

## Reporting security issues

Please **do not** open public issues for security vulnerabilities.
See [SECURITY.md](SECURITY.md) for the disclosure process.

## Code of Conduct

By participating in this project you agree to abide by the
[Code of Conduct](CODE_OF_CONDUCT.md).
