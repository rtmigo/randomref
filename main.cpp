#include <iostream>
#include <list>
#include <vector>
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

class Int32Generator {
    public:
        virtual uint32_t next() = 0;
        string seed_str, algo;
};

class Xorshift32: public Int32Generator {
public:
    Xorshift32(uint32_t a) {
        this->a = a;
        this->seed_str = to_string(a);
        this->algo = "xorshift32";
    }

    //uint32_t seedA;

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

void print32(Int32Generator& generator, string id) {
    vector<uint32_t> ints;
    vector<double_t> randbl32;

    for (uint32_t i=0; i<10; ++i) {
        int32_t x = generator.next();
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

    cout << "\"algorithm\": " << '"' << generator.algo << "\"," << endl;
    cout << "\"sample_id\": " << '"' << id << "\"," << endl;
    cout << "\"comment\": " << "\"seed " << generator.seed_str << "\"," << endl;

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

int main() {

	#define PI32 314159265
	#define PI64 3141592653589793238ll    

    Xorshift32 xorshA(1), xorshB(42), xorshC(PI32);

    print32(xorshA, "a");    cout << ",\n\n";
    print32(xorshB, "b");    cout << ",\n\n";
    print32(xorshC, "c");    cout << endl;

    //cout << "hi!";
    //cout << xorsh.next() << " " << xorsh.next();
    //cout << endl;
}