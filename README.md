# Lua370

**Lua370** is a port of the Lua 5.4 scripting engine for IBM MVS 3.8j, originally created by Michael Dean Rayborn. It provides both a **standalone interpreter** (`LUA`) and **bytecode compiler** (`LUAC`) as MVS load modules, and an **embeddable C library** (`liblua370.a`) for building Lua into other MVS programs (e.g. an HTTPD CGI). It runs on Hercules-emulated systems.

This project is maintained as part of the [mvslovers](https://github.com/mvslovers) community.

## Features

- Standalone `LUA` interpreter and `LUAC` bytecode compiler (load modules)
- Embeddable C library (`liblua370.a`) for other MVS C applications (e.g. HTTPD CGI)
- Full Lua 5.4 language support (C89 mode)
- Standard libraries: base, coroutine, debug, I/O, math, OS, string, table, UTF-8
- Dynamic module loading
- EBCDIC-compatible (the lexer uses `LUA_USE_CTYPE=1` for EBCDIC source)

## Building

Lua370 builds with [mbt](https://github.com/mvslovers/mbt) **v2** (MVS Build
Tools). The whole build runs **on the host** with the **cc370** toolchain and
produces the `LUA`/`LUAC` load modules and the static library — nothing needs to
run on MVS to build it.

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
make            # build the LUA + LUAC load modules (on the host)
make all        # the modules + the liblua370.a library archive
make deploy     # XMIT + upload + RECEIVE the modules into a LINKLIB (touches MVS)
make package    # dist/ tarballs: <ver>-load (modules) + <ver>-lib (archive + headers)
make clean      # remove build/ and dist/
```

Everything compiles on the host with `cc370`/`ar370`/`ld370`; only `make deploy`
needs an MVS connection.

### Using lua370 as a dependency

lua370 also ships an embeddable **library** — link it from another mbt v2
project (e.g. **httplua**) by declaring it in that project's `project.toml`:

```toml
[dependencies]
"mvslovers/lua370" = ">=1.1.0-dev"
```

`make deps` then stages `liblua370.a` plus the public headers (`lua.h`,
`luaconf.h`, `lualib.h`, `lauxlib.h`, `lua.hpp`) under `.mbt/deps/lua370/`, and
the consuming build links the archive by autocall.

## Project Structure

```
app/          Standalone mains (lua.c -> LUA, luac.c -> LUAC)
src/          Lua 5.4 core + standard libraries (the shared core + the library)
include/      Header files (public Lua API + internals)
project.toml  mbt project definition (type = application; modules + [internal] + [lib])
Makefile      two-line include of mbt/mk/mbt.mk
mbt/          mbt build tool (git submodule)
```

## Acknowledgments

This project was created by Michael Dean Rayborn, who has contributed several tools used within the MVS 3.8j open-source ecosystem.

Lua is originally developed by Roberto Ierusalimschy, Waldemar Celes, and Luiz Henrique de Figueiredo at PUC-Rio.

## License

This project is licensed under the [MIT License](LICENSE).
