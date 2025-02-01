# Purpose
I'm using this repository for self-instruction of various subjects in c programming and ecosystem.

Specifically, I'm trying out the [Ninja](https://ninja-build.org/manual.html#_syntax_example) build system after watching a [Tsoding](https://www.youtube.com/@TsodingDaily) [video](https://www.youtube.com/watch?v=Z8znH5Grz7I) discussing the topic.

I'm also playing around with different allocators strategies, but focusing on making a serviceable Arena allocator.


## Building
I just use nix for my specific system at the moment, but that could be easily ammended to be more general. You can see the dependencies I use in my dev shell.

So, if you happen to have nix installed and are on a similar system:
```
    >_: nix develop
```
will get you started
