# Security Policy

## Supported Versions

| Version | Supported          |
|---------|--------------------|
| 0.1.x   | :white_check_mark: |
| < 0.1   | :x:                |


### What to include

Whichever channel you use, please include:

- A clear description of the issue and its impact (what an attacker
  can achieve, and under what conditions).
- Steps to reproduce, ideally a minimal proof-of-concept input or
  test case.
- Affected version (commit hash or release tag).
- Your build environment (compiler + version, OS, sanitizers
  enabled).
- Whether you have already disclosed the issue elsewhere.

### Response expectations

- **Acknowledgement:** within 3 business days of receipt.
- **Triage and severity assessment:** within 7 business days.
- **Coordinated disclosure window:** typically no more than 90 days
  from the initial report. We will agree a disclosure date with you
  and credit you in the advisory unless you ask to remain anonymous.

If you do not receive an acknowledgement within 3 business days,
please assume the message was lost and resend through the other
channel.

## Out of Scope

- Issues that require running the CLI on attacker-controlled binaries
  with attacker-controlled flags — that is the operator's threat
  model.
- Performance regressions.
- Style or documentation issues — please use the normal issue tracker.
