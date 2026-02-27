# Lua370

**Lua370** is a port of the Lua 5.4 scripting engine for IBM MVS 3.8j, originally created by Michael Dean Rayborn. It provides a C89-compatible Lua interpreter and compiler for applications running on Hercules-emulated MVS systems.

This project is maintained as part of the [mvslovers](https://github.com/mvslovers) community.

## Features

- Full Lua 5.4 language support (C89 mode)
- Standard libraries: base, coroutine, debug, I/O, math, OS, string, table, UTF-8
- Dynamic module loading
- Embeddable in other MVS C applications (e.g., HTTPD CGI)
- EBCDIC-compatible

## Building

### Prerequisites

- Cross-compilation environment for MVS 3.8j
- `c2asm370` compiler (GCC-based, targeting S/370)
- `mvsasm` assembler script
- CRENT370 libraries by Mike Rayborn

### Clone

```bash
git clone --recursive https://github.com/mvslovers/lua370.git
cd lua370
```

### Configuration

Copy `.env.example` to `.env` and set your MVS host credentials and dataset names:

```bash
cp .env.example .env
# edit .env with your values
```

### Compile

```bash
make
make clean
```

The build pipeline is: C source → `c2asm370` → S/370 Assembly (.s) → `mvsasm` → Object decks (.o) → MVS datasets.

## Project Structure

```
src/          C source files (Lua core, libraries, compiler)
include/      Header files (Lua API and internals)
contrib/      Git submodules (SDK headers for dependencies)
scripts/      Build helper scripts
doc/          Documentation
```

## Acknowledgments

This project was created by Michael Dean Rayborn, who has contributed several tools used within the MVS 3.8j open-source ecosystem.

Lua is originally developed by Roberto Ierusalimschy, Waldemar Celes, and Luiz Henrique de Figueiredo at PUC-Rio.

## License

This project is licensed under the [MIT License](LICENSE).
