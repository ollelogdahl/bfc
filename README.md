# bfc
### A simple brainfuck compiler written in C
Olle Lögdahl, 25 December 2021

![make](https://img.shields.io/github/workflow/status/ollelogdahl/bfc/build)
![licence](https://img.shields.io/github/license/ollelogdahl/bfc)

---

**bfc** is a simple brainfuck compiler (or assembly generator), able to produce x86_64 assembly.
The compiler also calls `as` to compile the assembly into a linkable object.
Written as a one day project, it's purely for educational purposes.
