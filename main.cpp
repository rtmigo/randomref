// SPDX-FileCopyrightText: (c) 2021 Artëm IG <github.com/rtmigo>
// SPDX-License-Identifier: UPL-1.0

// runs reference implementations of random number generators.
// Prints the output of the generators to stdout in JSON format.

#define INTS_PER_SAMPLE 1000

#define XORSHIFT32 true
#define XORSHIFT64 true
#define XORSHIFT128 true
#define XORSHIFT128P true
#define XOSHIRO128PP true
#define XOSHIRO256PP true
#define SPLITMIX64 true
#define MULBERRY32 true
#define LEMIRE true

////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <list>
#include <vector>
#include <memory>
#include <string>
#include <math.h>
#include <assert.h>

using namespace std;

/////////////////////////////////////////////////////////////// INT-TO-DOUBLE //
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

/////////////////////////////////////////////////////////////// INT-TO-DOUBLE //
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
// BASE CLASSES

template <typename T>

void print_list(vector<T> &values, string fmt) {
    bool isFirst=true;
    for (auto const& x : values) {
        cout << (isFirst ? "\t\t" : ",\n\t\t");
        isFirst = false;
        char valueStr[256];
        sprintf(valueStr, fmt.c_str(), x);
        cout << '"' << valueStr << '"';
        //printf("\"%s\"", valueStr);	
    }    
}

class Alg {
public:
    virtual void print() = 0;
    virtual ~Alg() {};
};

class Alg32: public Alg {
public:

    virtual uint32_t next() = 0;
    string seed_str, seed_name, alg_name;

    void print() {
        vector<uint32_t> ints;
        vector<double_t> randbl32;

        for (int i=0; i<INTS_PER_SAMPLE; ++i) {
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

        std::cout << "{\n";

        std::cout << "\t\"sample_class\": " << '"' << this->alg_name << "\"," 
            << std::endl;

        std::cout << "\t\"sample_name\": " << '"' << this->seed_name << "\"," 
            << std::endl;

        std::cout << "\t\"description\": " << "\"seed " << this->seed_str 
            << "\"," << std::endl;

        std::cout << "\t\"uint\": [\n";
        print_list<uint32_t>(ints, "%08x");
        cout << " ],";
        cout << endl;

        std::cout << "\t\"double_doornik_randbl32\": [\n";
        print_list<double_t>(randbl32, "%.20e");
        cout << " ],\n";    

        std::cout << "\t\"double_vigna_multiplication\": [\n";
        print_list<double_t>(doubles_mult, "%.20e");
        cout << " ],\n";    

        std::cout << "\t\"double_vigna_bitcast\": [\n";
        print_list<double_t>(doubles_alt, "%.20e");
        cout << " ]\n";    

        cout << "}";
    }        
};

class Alg64: public Alg {
public:

    virtual uint64_t next() = 0;
    string seed_str, seed_name, alg_name;

    void print() {
        vector<uint64_t> ints;
        //vector<double_t> randbl32;
        vector<double_t> doubles_mult, doubles_alt;

        for (int i=0; i<INTS_PER_SAMPLE; ++i) {
            int64_t x = this->next();
            ints.push_back(x);
            //randbl32.push_back(RANDBL_32(x));
            doubles_mult.push_back(vigna_uint64_to_double_mult(x));
            doubles_alt.push_back(vigna_uint64_to_double_alt(x));
        }

        cout << "{\n";
        cout << "\t\"sample_class\": " << '"' << this->alg_name << "\"," << endl;
        cout << "\t\"sample_name\": " << '"' << this->seed_name << "\"," << endl;
        cout << "\t\"description\": " << "\"seed " << this->seed_str << "\"," 
            << endl;

        cout << "\t\"uint\": [\n";
        print_list<uint64_t>(ints, "%016llx");
        cout << " ],";
        cout << endl;

        // cout << "\t\"double_doornik_randbl32\": [\n";
        // print_list<double_t>(randbl32, "%.20e");
        // cout << " ],\n";    

        cout << "\t\"double_vigna_multiplication\": [\n";
        print_list<double_t>(doubles_mult, "%.20e");
        cout << " ],\n";    

        cout << "\t\"double_vigna_bitcast\": [\n";
        print_list<double_t>(doubles_alt, "%.20e");
        cout << " ]\n";    

        cout << "}";
    }        
};

class BoundedInt32: public Alg {
public: 
    BoundedInt32(string name, uint32_t range, Alg32* g): generator(g) {
        this->sample_name = name;
        this->range = range;
    }

    string alg_name;
    string sample_name;
    uint32_t range;

    virtual uint32_t nextBounded() = 0;

    void print() {

        cout << "{\n";
        cout << "\t\"sample_class\": " << '"' << this->alg_name << "\"," << endl;
        cout << "\t\"sample_name\": " << '"' << this->sample_name << "\"," 
            << endl;
        cout << "\t\"description\": " << "\"" 
            << this->generator->alg_name << " with seed "
            << this->generator->seed_str
        
        << " bounded to [0, " << this->range << ")\"," << endl;


        vector<uint32_t> bounded;
        for (int i=0; i<INTS_PER_SAMPLE; ++i) {
            auto x = this->nextBounded();
            assert(x>=0);
            assert(x<this->range);
            bounded.push_back(x);
        }        

        cout << "\t\"uint\": [\n";
        print_list<uint32_t>(bounded, "%08x");
        cout << " ]";

        cout << endl;          
        cout << "}";        
        //cout << endl;  

    }

protected:
    shared_ptr<Alg32> generator;
    uint32_t nextRaw32() {
      return this->generator->next();  
    }    
};

////////////////////////////////////////////////////////////////////////////////
// XORSHIFT32
//
// sample from https://en.wikipedia.org/wiki/Xorshift
//
// Refactored from
// George Marsaglia 2003 "Xorshift RNGs"
// https://www.jstatsoft.org/article/view/v008i14
//		page 3: "Here is a basic 32-bit xorshift C procedure that takes
//      a 32-bit seed value y:"
// 		unsigned long xor(){ 
//			static unsigned long y=2463534242; 
//			yˆ=(y<<13); y=(y>>17); return (yˆ=(y<<5)); 
//		}

class Xorshift32: public Alg32 {
public:
    Xorshift32(string seed_name, uint32_t a) {
        this->a = a;
        this->seed_name = seed_name;
        this->seed_str = to_string(a);
        this->alg_name = "xorshift32";
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
// XORSHIFT64
//
// sample from https://en.wikipedia.org/wiki/Xorshift
//
//	Refactored from
//	George Marsaglia 2003 "Xorshift RNGs" 
// 	https://www.jstatsoft.org/article/view/v008i14
//
// 	page 4: For C compilers that have 64-bit integers, the following will 
//  provide an excellent period 264−1 RNG, given a 64-bitseed x:
//		unsigned long long xor64(){
//			static unsigned long long x=88172645463325252LL;
//			xˆ=(x<<13); xˆ=(x>>7); return (xˆ=(x<<17));
//		}

class Xorshift64: public Alg64 {
public:
    Xorshift64(string seed_name, 
                uint64_t seedA) {
        this->a = seedA;
        this->seed_name = seed_name;
        this->seed_str = to_string(a);
        this->alg_name = "xorshift64";
    }

    /* The state array must be initialized to not be all zero */
    uint64_t a;
    
    uint64_t next() {
        uint64_t x = this->a;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
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

class Xorshift128: public Alg32 {
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
        this->alg_name = "xorshift128";
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

////////////////////////////////////////////////////////////////////////////////
//	XORSHIFT128+ (V2)
//  Implemented in xrandom as Xorshift128p.
//
//	Sebastiano Vigna
//	Further scramblings of Marsaglia’s xorshift generators
//	https://arxiv.org/abs/1404.0390 [v2] Mon, 14 Dec 2015 - page 6
//	https://arxiv.org/abs/1404.0390 [v3] Mon, 23 May 2016 - page 6

class Xorshift128p: public Alg64 {
public:
    Xorshift128p(string seed_name, uint64_t seedA, uint64_t seedB) {
        this->s[0] = seedA;
        this->s[1] = seedB;
        this->seed_name = seed_name;
        this->seed_str = to_string(seedA)+" "+to_string(seedB);
        this->alg_name = "xorshift128+";
    }

    uint64_t s[2];
    
    uint64_t next() {
		uint64_t s1 = s[0];
		const uint64_t s0 = s[1];
		const uint64_t result = s0 + s1;
		s[0] = s0;
		s1 ^= s1 << 23; // a
		s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5); // b, c
		return result;        
    }
};

////////////////////////////////////////////////////////////////////////////////
// XOSHIRO128++ 1.0
//
// https://prng.di.unimi.it/xoshiro128plusplus.c
// Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org) CC-0
//
// "This is xoshiro128++ 1.0, one of our 32-bit all-purpose, rock-solid
//  generators. It has excellent speed, a state size (128 bits) that is
//  large enough for mild parallelism, and it passes all tests we are aware
//  of."

class Xoshiro128pp: public Alg32 {
public:

    static inline uint32_t xoshiro128pp_rotl(const uint32_t x, int k) {
        return (x << k) | (x >> (32 - k));
    }

    Xoshiro128pp(   string seed_name, uint32_t seedA, uint32_t seedB, 
                    uint32_t seedC, uint32_t seedD  )
    {
        this->s[0] = seedA;
        this->s[1] = seedB;
        this->s[2] = seedC;
        this->s[3] = seedD;

        this->seed_name = seed_name;
        this->seed_str = to_string(seedA) + " " + to_string(seedB) + " " 
                        + to_string(seedC) + " " + to_string(seedD);
        this->alg_name = "xoshiro128++";
    }

    uint32_t s[4];
    
    uint32_t next() {
        const uint32_t result = xoshiro128pp_rotl(s[0] + s[3], 7) + s[0];

        const uint32_t t = s[1] << 9;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = xoshiro128pp_rotl(s[3], 11);

        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////
// XOSHIRO256++ 1.0
//
// https://prng.di.unimi.it/xoshiro256plusplus.c
// Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org) CC-0
//
// "This is xoshiro256++ 1.0, one of our all-purpose, rock-solid generators.
//  It has excellent (sub-ns) speed, a state (256 bits) that is large
//  enough for any parallel application, and it passes all tests we are
//  aware of.
//
//  For generating just floating-point numbers, xoshiro256+ is even faster.
//
//  The state must be seeded so that it is not everywhere zero. If you have
//  a 64-bit seed, we suggest to seed a splitmix64 generator and use its
//  output to fill s.

class Xoshiro256pp: public Alg64 {
public:

    static inline uint64_t xoshiro256pp_rotl(const uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }

    Xoshiro256pp(   string seed_name, uint64_t seedA, uint64_t seedB, 
                    uint64_t seedC, uint64_t seedD  ) {
        this->s[0] = seedA;
        this->s[1] = seedB;
        this->s[2] = seedC;
        this->s[3] = seedD;

        this->seed_name = seed_name;
        this->seed_str = to_string(seedA)+" "+to_string(seedB)+" "
                        +to_string(seedC)+" "+to_string(seedD);
        this->alg_name = "xoshiro256++";
    }

    uint64_t s[4];
    
    uint64_t next() {
        const uint64_t result = xoshiro256pp_rotl(s[0] + s[3], 23) + s[0];

        const uint64_t t = s[1] << 17;

        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];

        s[2] ^= t;

        s[3] = xoshiro256pp_rotl(s[3], 45);

        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////
// SPLITMIX64
//
// https://prng.di.unimi.it/splitmix64.c
// Written in 2015 by Sebastiano Vigna (CC-0)
//
// "It is a very fast generator passing BigCrush, and it can be useful if
//  for some reason you absolutely want 64 bits of state."

// static uint64_t x; /* The state can be seeded with any value. */

class Splitmix64: public Alg64 {
public:
    Splitmix64(string seed_name, uint64_t seedA) {
        this->x = seedA;

        this->seed_name = seed_name;
        this->seed_str = to_string(seedA);
        this->alg_name = "splitmix64";
    }

    uint64_t x;
    
    uint64_t next() {
        uint64_t z = (this->x += 0x9e3779b97f4a7c15);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }
};

////////////////////////////////////////////////////////////////////////////////
// MULBERRY32
// (c) 2017 by Tommy Ettinger (CC0)
// https://gist.github.com/tommyettinger/46a874533244883189143505d203312c

class Mulberry32: public Alg32 {
public:
    Mulberry32(string seed_name, uint64_t seedA) {
        this->x = seedA;

        this->seed_name = seed_name;
        this->seed_str = to_string(seedA);
        this->alg_name = "mulberry32";
    }

    uint64_t x;
    
    uint32_t next() {
        uint32_t z = (this->x += 0x6D2B79F5UL);
        z = (z ^ (z >> 15)) * (z | 1UL);
        z ^= z + (z ^ (z >> 7)) * (z | 61UL);
        return z ^ (z >> 14);
    }
};

////////////////////////////////////////////////////////////////////////////////
// The "Lemire Method" <https://arxiv.org/abs/1805.10941> implemented 
// by D. Lemire for Python (License: Apache):
// <https://github.com/lemire/fastrand/blob/master/fastrandmodule.c>

class Lemire: public BoundedInt32 {
public:
    Lemire(string sample_name, uint32_t range, Alg32* generator): 
        BoundedInt32(sample_name,range, generator) {
            this->alg_name = "lemire_divisionless";
        }

    uint32_t nextBounded() {
        // renamed from "pcg32_random_bounded_divisionless"
        uint64_t random32bit, multiresult;
        uint32_t leftover;
        uint32_t threshold;
        random32bit =  nextRaw32();
        multiresult = random32bit * range;
        leftover = (uint32_t) multiresult;
        if(leftover < range ) {
            threshold = -range % range ;
            while (leftover < threshold) {
                random32bit =  nextRaw32();
                multiresult = random32bit * range;
                leftover = (uint32_t) multiresult;
            }
        }
        return multiresult >> 32; // [0, range)
    }    
};

////////////////////////////////////////////////////////////////////////////////
// http://www.pcg-random.org/posts/bounded-rands.html
// (c) 2018 Melissa E. O'Neill (License: MIT)
// "The fastest (unbiased) method is Lemire's (with an extra tweak)"

class LemireOneil: public BoundedInt32 {
public:
    LemireOneil(string sample_name, uint32_t range, Alg32* generator): 
        BoundedInt32(sample_name,range, generator) {
            this->alg_name = "lemire_oneil_divisionless";
        }

    uint32_t nextBounded() {
        uint32_t x = nextRaw32();
        uint64_t m = (uint64_t)x * (uint64_t)range;
        uint32_t l = (uint32_t)m;
        if (l < range) {
            uint32_t t = -range;
            if (t >= range) {
                t -= range;
                if (t >= range) 
                    t %= range;
            }
            while (l < t) {
                x = nextRaw32();
                m = (uint64_t)x * (uint64_t)range;
                l = (uint32_t)m;
            }
        }
        return m >> 32;
    }    
};

void checkOneilMatсhesLemire(uint32_t range) {
    auto a = Lemire("", range, new Xorshift32("", 1));
    auto b = LemireOneil("", range, new Xorshift32("", 1));
    for (int i=0; i<10000; ++i) {
        assert(a.nextBounded()==b.nextBounded());
    }
}

// checking that the method by O'Neil (with extra tweak) returns the same
// results as the method without tweaks
void checkOneilMatchesLemireAll() {
    checkOneilMatсhesLemire(1);
    checkOneilMatсhesLemire(100);
    checkOneilMatсhesLemire(169834);
    checkOneilMatсhesLemire(0x7FFFFFFF);
    checkOneilMatсhesLemire(0x80000000);
    checkOneilMatсhesLemire(0xFFFFFFFF);
}

// MAIN ////////////////////////////////////////////////////////////////////////

int main() {

    checkOneilMatchesLemireAll();
    //exit(0);

	#define PI32 314159265
	#define PI64 3141592653589793238ll    

    vector<shared_ptr<Alg>> print_us = {
        #if XORSHIFT32
        shared_ptr<Alg>(new Xorshift32("a", 1)),
        shared_ptr<Alg>(new Xorshift32("b", 42)),
        shared_ptr<Alg>(new Xorshift32("c", PI32)),
        #endif

        #if XORSHIFT64
        shared_ptr<Alg>(new Xorshift64("a", 1)),
        shared_ptr<Alg>(new Xorshift64("b", 42)),
        shared_ptr<Alg>(new Xorshift64("c", PI64)),
        #endif

        #if XORSHIFT128
        shared_ptr<Alg>(new Xorshift128("a", 1, 2, 3, 4)),
        shared_ptr<Alg>(new Xorshift128("b", 5, 23, 42, 777)),
        shared_ptr<Alg>(new Xorshift128(
            "c", 1081037251u, 1975530394u, 2959134556u, 1579461830u)),        
        #endif    

        #if XORSHIFT128P
        shared_ptr<Alg>(new Xorshift128p("a", 1, 2)),
        shared_ptr<Alg>(new Xorshift128p("b", 42, 777)),
        shared_ptr<Alg>(new Xorshift128p(
            "c", 8378522730901710845llu, 1653112583875186020llu)),
        #endif

        #if XOSHIRO128PP
        shared_ptr<Alg>(new Xoshiro128pp("a", 1, 2, 3, 4)),
        shared_ptr<Alg>(new Xoshiro128pp("b", 5, 23, 42, 777)),
        shared_ptr<Alg>(new Xoshiro128pp("c", 1081037251u, 1975530394u, 
	 						2959134556u, 1579461830u)),
        #endif

        #if XOSHIRO256PP
        shared_ptr<Alg>(new Xoshiro256pp("a", 1, 2, 3, 4)),
        shared_ptr<Alg>(new Xoshiro256pp("b", 5, 23, 42, 777)),
        shared_ptr<Alg>(new Xoshiro256pp(
            "c", 0x621b97ff9b08ce44ull, 0x92974ae633d5ee97ull, 
            0x9c7e491e8f081368ull, 0xf7d3b43bed078fa3ull)),
        #endif

        #if SPLITMIX64
        shared_ptr<Alg>(new Splitmix64("a", 1)),
        shared_ptr<Alg>(new Splitmix64("b", 0)),
        shared_ptr<Alg>(new Splitmix64("c", 777)),
        shared_ptr<Alg>(new Splitmix64("d", 0xf7d3b43bed078fa3ull)),
        #endif

        #if MULBERRY32
        shared_ptr<Alg>(new Mulberry32("a", 1)),
        shared_ptr<Alg>(new Mulberry32("b", 0)),
        shared_ptr<Alg>(new Mulberry32("c", 777)),
        shared_ptr<Alg>(new Mulberry32("d", 1081037251u)),
        #endif

        #if LEMIRE
        shared_ptr<Alg>(new Lemire("1000", 1000, new Xorshift32("", 777))),
        shared_ptr<Alg>(new Lemire("1", 1, new Xorshift32("", 777))),
        shared_ptr<Alg>(new Lemire("FFx", 0xFFFFFFFFu, new Xorshift32("", 777))),
        shared_ptr<Alg>(new Lemire("7Fx", 0x7FFFFFFFu, new Xorshift32("", 777))),
        shared_ptr<Alg>(new Lemire("80x", 0x80000000u, new Xorshift32("", 777))),
        shared_ptr<Alg>(new Lemire("R1", 0x0f419dc8u, new Xorshift32("", 777))),
        shared_ptr<Alg>(new Lemire("R2", 0x32e7aeecu, new Xorshift32("", 777))),
        #endif
    };

    bool isFirst = true;    
    for (auto const& pgen : print_us) {
        std::cout << (isFirst ? "[\n" : ",\n\n");
        isFirst = false;        
        pgen->print();
    }
    std::cout << "\n]";

    std::cout << endl;
}