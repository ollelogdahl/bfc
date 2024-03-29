# bfc
### A simple optimizing brainfuck compiler written in C
Olle Lögdahl, 25 December 2021

[![build](https://github.com/ollelogdahl/bfc/actions/workflows/build.yml/badge.svg)](https://github.com/ollelogdahl/bfc/actions/workflows/build.yml)
![licence](https://img.shields.io/github/license/ollelogdahl/bfc)

---

**bfc** is a simple brainfuck compiler (or assembly generator), able to
produce x86_64 assembly. The compiler may also call `as` to compile the
assembly into a linkable object. Written as a one day project, it's purely
for educational purposes.

The compiler also implements some (albeit very simple) optimizations using
patterns (`[-]` can be optimized as a set 0 for example).

## Building & Usage

The only requirements to build are a `C99`+ POSIX compliant compiler and
`GNU make`. `bfc` can be built and installed by:

```bash
make build
sudo make install
```

Compiling some `branfuck` code is then done by:

```bash
echo "+[-->-[>>+>-----<<]<--<---]>-.>>>+.>>..+++[.>]<<<<.+++.------.<<-.>>>>+." > hello.b
bfc -o hello.o hello.b
ld -o hello hello.o
```

The assembly can be inspected by running:
```bash
bfc -gS -o hello.s hello.o
```

### Help
```
bfc [-gShv] [-o file] file
  S        skips the assembler and only outputs the generated assembly.
  g        generates assembly with comments about the bf instructions.
  o file   the file to write the assembly/linkable object to.
  h        shows this.
  v        shows the version of the program.
```

## Testing

A part of the challenge was also writing a testssuite in `sh` for testing the
compiler and the generated binaries. These can be run with `make test` to output
the following:

```
./check.sh out/bfc nocolor
rot13:  ok
t10:    ok
t11:    ok
t12:    ok
t1:     ok
t3:     ok
t4:     ok
t5:     ok
t6:     ok
t7:     ok
t8:     ok
t9:     ok
ran 14 tests with 79 scenarios.
```

## Licensing

This project, and all code it contains, is licensed under the *MIT License* and can be read [here](LICENSE).
