Creates JSON with reference values that are the output of different random 
number generators (PRNG).

The reference PRNG algorithm implementations are usually in C99. C++ behaves 
the same way when handling numbers. So we use C++ to generate the reference 
numbers exactly.

The generated values can be used to test the same algorithms rewritten 
in other languages.

For example, this is how the [xrandom](https://github.com/rtmigo/xrandom) 
library is tested.