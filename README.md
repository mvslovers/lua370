# Lua370

**Lua370** is a port of the Lua 5.4 scripting engine for IBM MVS 3.8j, originally created by Michael Dean Rayborn. It provides a C89-compatible Lua engine as a **static library**, for embedding in MVS C applications (e.g. an HTTPD CGI) running on Hercules-emulated systems.

This project is maintained as part of the [mvslovers](https://github.com/mvslovers) community.

## Features

- Full Lua 5.4 language support (C89 mode)
- Standard libraries: base, coroutine, debug, I/O, math, OS, string, table, UTF-8
- Dynamic module loading
- Embeddable in other MVS C applications (e.g., HTTPD CGI)
- EBCDIC-compatible

## Building

Lua370 builds with [mbt](https://github.com/mvslovers/mbt) **v2** (MVS Build
Tools). The whole build runs **on the host** with the **cc370** toolchain and
produces a static library — nothing needs to run on MVS to build it.

### Prerequisites

- The **cc370** host toolchain (a GCC 3.4.6 fork: `cc370`, `as370`, `ar370`).
  It also provides the **libc370** sysroot (the C runtime plus `racf.h` / `acee.h`).
- **Python 3.12+**

### Clone

```bash
git clone --recursive https://github.com/mvslovers/lua370.git
cd lua370
```

### Build

```bash
make            # cross-compile src/*.c and archive build/liblua370.a
make package    # build dist/lua370-<version>-lib.tar.gz (archive + public headers)
make clean      # remove build/ and dist/
```

Each `src/*.c` is compiled to a `.o` with `cc370` and archived with `ar370`; no
MVS connection is needed.

### Using lua370 as a dependency

lua370 is a **library** — embed it from another mbt v2 project (e.g. **httplua**)
by declaring it in that project's `project.toml`:

```toml
[dependencies]
"mvslovers/lua370" = ">=1.0.3-dev"
```

`make deps` then stages `liblua370.a` plus the public headers (`lua.h`,
`luaconf.h`, `lualib.h`, `lauxlib.h`, `lua.hpp`) under `.mbt/deps/lua370/`, and
the consuming build links the archive by autocall.

## Project Structure

```
src/          C source files (Lua 5.4 core + standard libraries)
include/      Header files (public Lua API + internals)
project.toml  mbt project definition (type = library)
Makefile      two-line include of mbt/mk/mbt.mk
mbt/          mbt build tool (git submodule)
```

## Acknowledgments

This project was created by Michael Dean Rayborn, who has contributed several tools used within the MVS 3.8j open-source ecosystem.

Lua is originally developed by Roberto Ierusalimschy, Waldemar Celes, and Luiz Henrique de Figueiredo at PUC-Rio.

## License

This project is licensed under the [MIT License](LICENSE).
