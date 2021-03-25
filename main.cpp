#include <iostream>
#include <list>
#include <math.h>

using namespace std;

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
};

class Xorshift32: public Int32Generator {
public:
    Xorshift32(uint32_t a) {
        this->a = a;
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

template <typename T>

void print_list(list<T> &values, char* format) {
    bool isFirst=true;
    for (auto const& x : values) {
        printf(isFirst ? "\t" : ",\n\t");
        isFirst = false;
        char valueStr[256];
        sprintf(valueStr, format, x);
        printf("\"%s\"", valueStr);	
    }    
}

void print32(Int32Generator* generator) {
    list<uint32_t> ints;
    list<double_t> randbl32;

    for (int i=0; i<10; ++i) {
        int32_t x = generator->next();
        ints.push_back(x);
        randbl32.push_back(RANDBL_32(x));
    }
    cout << "\"ints\": [\n";
    print_list<uint32_t>(ints, "%08x");
    cout << "\n],";
    cout << endl;

    cout << "\"randbl32\": [\n";
    print_list<double_t>(randbl32, "%.20e");
    cout << "\n],";    
}

int main() {

    Xorshift32 xorsh(1);

    print32(&xorsh);
    cout << endl;
    //cout << "hi!";
    //cout << xorsh.next() << " " << xorsh.next();
    //cout << endl;
}