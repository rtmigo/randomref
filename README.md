Creates JSON with output values from different random number generators (PRNG).

The generated values can be used to test the same PRNG algorithms rewritten 
in other languages. This is how the [xrandom](https://github.com/rtmigo/xrandom) library is tested.

The reference PRNG algorithm implementations are usually in C99. C++ behaves 
the same way when handling numbers. So we use C++ to generate the reference 
values.




## Compiling and running 

On POSIX system with GCC:

``` shell
$ g++ main.cpp --std=c++2a -o randomref.com
$ ./randomref.com
```