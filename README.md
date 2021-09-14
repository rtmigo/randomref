Creates JSON with output values from different random number generators (PRNG).

The generated values can be used to test the same PRNG algorithms rewritten 
in other languages. This is how the [xrandom](https://github.com/rtmigo/xrandom) library is tested.

The reference PRNG algorithm implementations are usually in C99. C++ behaves 
the same way when handling numbers. So we use C++ to generate the reference 
values.

## Algorithms

Random 32-bit integer:
- xorshift32
- xorshift128
- xoshiro128++
- mulberry32

Random 64-bit integer:
- xorshift64
- xorshift128+
- xoshiro256++
- xoshiro256**
- splitmix64

Bounded integer:
- [divisionless](https://arxiv.org/abs/1805.10941)

Int-to-double:
- [multiplication](https://prng.di.unimi.it/)
- [bitcast](https://prng.di.unimi.it/)
- [randbl32](http://doi.acm.org/10.1145/1189756.118975)

## Compiling and running 

On POSIX system with GCC:

``` shell
$ g++ main.cpp --std=c++2a -o randomref.com
$ ./randomref.com
```