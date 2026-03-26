#include <stdio.h>

int main() {
    char c;
    short int s;
    int i;
    unsigned int u;
    long int l;
    unsigned long int ul;
    float f;
    double d;
    long double ld;

    i = i + c;      // c는 정수로 변환됨
    i = i + s;      // s는 정수로 변환됨
    u = u + i;      // (*)i는 무부호 정수로 변환됨
    l = l + u;      // u는 장정수로 변환됨
    ul = ul + l;    // (*)l은 무부호 장정수로 변환됨
    f = f + ul;     // (*)ul은 소수형으로 변환됨
    d = d + f;      // (*)f는 2배소수형으로 변환됨
    ld = ld + d;    // d는 장2배소수형으로 변환됨

    char c2;
    int i2;
    float f2;
    double d2;

    i2 = c2;    // c2는 정수형으로 변환됨
    f2 = i2;    // i2는 소수형으로 변환됨
    d2 = f2;    // f2는 2배소수형으로 변환됨

    return 0;
}