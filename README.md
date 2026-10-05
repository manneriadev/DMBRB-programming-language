# DMBRB

[![CI](https://github.com/manneriadev/DMBRB-programming-language/actions/workflows/ci.yml/badge.svg)](https://github.com/manneriadev/DMBRB-programming-language/actions/workflows/ci.yml)

A statically typed, compiled programming language that transpiles to C.
Syntax inspired by Julia and Kotlin, architecture inspired by Rust and C.

**Goals:** simple readable syntax, static typing with type inference, value semantics
(assignment creates an independent copy), a minimal but strict execution model.

## Example

```
# comments start with #
var const PI: float64 = 3.14159     # constant
var c = 10 + 20                     # type inferred

function max(a: int32, b: int32)::int32
begin
    return a > b ? a : b
end

function main()::int32
begin
    var opt: int32? = none          # optional type
    var px: int32 = 100
    var ptr: int32* = &px           # pointer
    *ptr = 200

    var f: float64 = 3.14
    var fi = f::int32               # cast with ::

    for i = 0:20:2                  # range loop with step
    begin
        c += i                      # compound assignment
    end
    return 0
end
```

More programs: [`examples/snake.dmb`](examples/snake.dmb), [`examples/donut.dmb`](examples/donut.dmb), [`examples/tree.dmb`](examples/tree.dmb).

## Features

Modules, structs with methods, `when`, `for` ranges, `while` / `break` / `continue`,
ternary operator, optional types, pointers, casts, bitwise and compound operators.
Full specification: [`docs/spec.txt`](docs/spec.txt).

## Quick start

**Requirements:** `gcc` in your PATH (the compiler generates C and builds it with gcc).
To build the compiler itself you need `g++` with C++23 support (GCC 13+).

- **Windows:** install [MSYS2](https://www.msys2.org), open the *MSYS2 UCRT64* terminal and run
  `pacman -S mingw-w64-ucrt-x86_64-gcc make git`
- **Linux:** `sudo apt install build-essential git`

```
git clone https://github.com/manneriadev/DMBRB-programming-language
cd DMBRB-programming-language
make
./dmbrb examples/snake.dmb
./examples/snake        # snake.exe on Windows
```

## Usage

```
dmbrb <file.dmb> [options]

  -o <name>       output executable name
  --emit-c        keep the generated C file
  --dump-tokens   print lexer tokens and exit
  -h, --help      show help
  -v, --version   show version
```

## How it works

```
source.dmb -> Lexer -> Parser -> ModuleLoader -> Analyzer (types) -> Codegen (C) -> gcc -> executable
```

| Directory | Contents |
|-----------|----------|
| `src/`, `inc/` | compiler sources and headers |
| `examples/` | example programs |
| `tests/` | test cases (`make test`) |
| `docs/` | language specification |

## Tests

```
make test
```

## License

MIT, see [LICENSE](LICENSE).
