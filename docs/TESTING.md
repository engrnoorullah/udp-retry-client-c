# Validation record

Validated on 27 October 2026 with GCC 13.3.0 on Linux.

- The maintained code builds with `-Wall -Wextra -Wpedantic -Werror`.
- `make test` passed in this environment.
- Tests run locally; they do not contact public services.
- Build products are excluded from version control.

Six Python integration tests passed, including two intentionally dropped requests followed by success on attempt three, retry exhaustion, invalid replies that do not extend the deadline, and payload bounds.
