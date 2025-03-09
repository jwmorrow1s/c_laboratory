# Purpose
I'm using this repository for self-instruction of various subjects in c programming and ecosystem.

Specifically, I'm trying out the [Ninja](https://ninja-build.org/manual.html#_syntax_example) build system after watching a [Tsoding](https://www.youtube.com/@TsodingDaily) [video](https://www.youtube.com/watch?v=Z8znH5Grz7I) discussing the topic.

I'm also playing around with different allocators strategies, but focusing on making a serviceable Arena allocator.

## Project Dependencies
I just use nix for my specific system at the moment, but that could be easily ammended to be more general. You can see the dependencies I use in my dev shell.

So, if you happen to have nix installed and are on a similar system:
```
    >_: nix develop
```
will get you started with necessary dependencies

## Source Dependency Changes
The directory structure is intended to be as follows
```
src
|_ main.c
include
|_ ${...c_impl_files}.c
|_ ${...c_header_files}.h
```
with a flat directory structure. If it gets more complex, the `gen-build.sh` will need to be ammended.

To create a new `build.ninja` file after making changes to files, run:
```
 >_: sh gen-build.sh
```
for a non-debug build
or
```
 >_: sh gen-build.sh --debug
```
for a debug build

and
```
>_: ninja
```

to generate a new executable.

Additionally, 

this script creates a number of utility scripts:
- `debug.sh`
- `clean.sh`
- `memcheck.sh`
- `lint.sh`
- `format.sh`

## Cleaning built files:
```
>_: ninja -t clean
```

## Running tests:
- Tests run from src/main.c
- running the build script with a test command runs the tests
- E,g
```
>_: sh gen-build --test-arena
>_: ninja
>_: ./main
```
