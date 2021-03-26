// SPDX-FileCopyrightText: (c) 2021 Art Galkin <github.com/rtmigo>
// SPDX-License-Identifier: UPL-1.0

// runs reference implementations of random number generators.
// Prints the output of the generators to stdout in JSON format.

#include <iostream>
#include <list>
#include <vector>
#include <memory>
#include <string>
#include <math.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////	
// "in C99 a 64-bit unsigned integer x should be converted to a 64-bit 
// double using the expression"
// by Sebastiano Vigna <https://prng.di.unimi.it/>

static double vigna_uint64_to_double_mult(uint64_t x) {
	return (x >> 11) * 0x1.0p-53;
}

// "An alternative, multiplication-free conversion" suggestion 
// by Sebastiano Vigna <https://prng.di.unimi.it/>
static inline double vigna_uint64_to_double_alt(uint64_t x) {
	const union { uint64_t i; double d; } u = { 
		.i = UINT64_C(0x3FF) << 52 | x >> 12 
	};
	return u.d - 1.0;
}

////////////////////////////////////////////////////////////////////////////////	
// Jurgen A. Doornik. 2007. Conversion of high-period random numbers to 
// floating point. ACM Trans. Model. Comput. Simul. 17, 1, Article 3 
// (January 2007). DOI=10.1145/1189756.1189759 
// http://doi.acm.org/10.1145/1189756.118975

#define M_RAN_INVM32 2.32830643653869628906e-010
#define M_RAN_INVM52 2.22044604925031308085e-016

#define RANDBL_32(iRan1) \
	((int)(iRan1)*M_RAN_INVM32 + 0.5)

#define RANDBL_32_NO_ZERO(iRan1) \
	((int)(iRan1)*M_RAN_INVM32 + (0.5 + M_RAN_INVM32 / 2))

//float number with 52bits
#define RANDBL_52_NO_ZERO(iRan1, iRan2) \
	((int)(iRan1)*M_RAN_INVM32 + (0.5 + M_RAN_INVM52 / 2) + \
	 (int)((iRan2)&0x000FFFFF) * M_RAN_INVM52)

////////////////////////////////////////////////////////////////////////////////
// GENERATORS BASE

template <typename T>

void print_list(vector<T> &values, string fmt) {
    bool isFirst=true;
    for (auto const& x : values) {
        cout << (isFirst ? "\t" : ",\n\t");
        isFirst = false;
        char valueStr[256];
        sprintf(valueStr, fmt.c_str(), x);
        cout << '"' << valueStr << '"';
        //printf("\"%s\"", valueStr);	
    }    
}

class Int32Generator {
public:
    virtual ~Int32Generator() {};

    virtual uint32_t next() = 0;
    string seed_str, seed_name, algo;

    void print32() {
        vector<uint32_t> ints;
        vector<double_t> randbl32;

        for (uint32_t i=0; i<10; ++i) {
            int32_t x = this->next();
            ints.push_back(x);
            randbl32.push_back(RANDBL_32(x));
        }

        vector<double_t> doubles_mult, doubles_alt;

        for (int32_t i=0; i<ints.size(); i+=2) {
            uint64_t combined = (((uint64_t)ints[i])<<32)|ints[i+1];
            doubles_mult.push_back(vigna_uint64_to_double_mult(combined));
            doubles_alt.push_back(vigna_uint64_to_double_alt(combined));
        }

        cout << "{\n";

        cout << "\"algorithm\": " << '"' << this->algo << "\"," << endl;
        cout << "\"sample_id\": " << '"' << this->seed_name << "\"," << endl;
        cout << "\"comment\": " << "\"seed " << this->seed_str << "\"," << endl;

        cout << "\"uint32\": [\n";
        print_list<uint32_t>(ints, "%08x");
        cout << " ],";
        cout << endl;

        cout << "\"double_doornik_randbl32\": [\n";
        print_list<double_t>(randbl32, "%.20e");
        cout << " ],\n";    

        cout << "\"double_vigna_multiplication\": [\n";
        print_list<double_t>(doubles_mult, "%.20e");
        cout << " ],\n";    

        cout << "\"double_vigna_memcast\": [\n";
        print_list<double_t>(doubles_alt, "%.20e");
        cout << " ]\n";    

        cout << "}";
    }        
};

////////////////////////////////////////////////////////////////////////////////

class Xorshift32: public Int32Generator {
public:
    Xorshift32(string seed_name, uint32_t a) {
        this->a = a;
        this->seed_name = seed_name;
        this->seed_str = to_string(a);
        this->algo = "xorshift32";
    }

    uint32_t a;    
    
    uint32_t next() {
        // Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
        uint32_t x = this->a;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return this->a = x;
    }
};

////////////////////////////////////////////////////////////////////////////////
// XORSHIFT128
//
// sample from https://en.wikipedia.org/wiki/Xorshift
//
//	Refactored from
//	George Marsaglia 2003 "Xorshift RNGs" 
// 	https://www.jstatsoft.org/article/view/v008i14
//
// 	page 5: 
//	Suppose we compare a xorshift RNG, period 2128−1, with a multiply-with-carry 
//	RNG of comparable period. First,the xorshift:
//		unsigned long xor128(){
//			static unsigned long x=123456789,y=362436069,z=521288629,w=88675123;
//			unsigned long t;t=(xˆ(x<<11));x=y;y=z;z=w;
//			return( w=(wˆ(w>>19))ˆ(tˆ(t>>8)) );}

class Xorshift128: public Int32Generator {
public:
    Xorshift128(string seed_name, 
                uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
        this->seed_name = seed_name;
        this->seed_str = to_string(a)+" "+to_string(b)
                         +" "+to_string(c)+" "+to_string(d);
        this->algo = "xorshift128+";
    }

    /* The state array must be initialized to not be all zero */
    uint32_t a, b, c, d;    
    
    uint32_t next() {
        /* Algorithm "xor128" from p. 5 of Marsaglia, "Xorshift RNGs" */
        uint32_t t = this->d;

        uint32_t const s = this->a;
        this->d = this->c;
        this->c = this->b;
        this->b = s;

        t ^= t << 11;
        t ^= t >> 8;
        return this->a = t ^ s ^ (s >> 19);
    }
};

// ////////////////////////////////////////////////////////////////////////////////
// //	XORSHIFT128+ (V2 ?)
// //  Implemented in xrandom as Xorshift128p.
// //
// //	Sebastiano Vigna
// //	Further scramblings of Marsaglia’s xorshift generators
// //	https://arxiv.org/abs/1404.0390 [v2] Mon, 14 Dec 2015 - page 6
// //	https://arxiv.org/abs/1404.0390 [v3] Mon, 23 May 2016 - page 6

// class Xorshift128p: public Int32Generator {
// public:
//     Xorshift128p(string seed_name, uint32_t a) {
//         this->a = a;
//         this->seed_name = seed_name;
//         this->seed_str = to_string(a);
//         this->algo = "xorshift128+";
//     }

//     uint64_t s[2];
    
//     uint32_t next() {
//     }
// };

// MAIN ////////////////////////////////////////////////////////////////////////

int main() {

	#define PI32 314159265
	#define PI64 3141592653589793238ll    

    //Xorshift32 xorshA("a", 1), xorshB("b", 42), xorshC("c", PI32);

    vector<Int32Generator*> print_us = {
        new Xorshift32("a", 1),
        new Xorshift32("b", 42),
        new Xorshift32("c", PI32),

        new Xorshift128("a", 1, 2, 3, 4),
        new Xorshift128("b", 5, 23, 42, 777),
        new Xorshift128("c", 1081037251u, 1975530394u, 2959134556u, 1579461830u),        
    };

    bool isFirst = true;    

    for (auto const& pgen : print_us) {
        cout << (isFirst ? "" : ",\n\n");
        isFirst = false;        
        pgen->print32();
        delete pgen;
    }

    cout << endl;
}