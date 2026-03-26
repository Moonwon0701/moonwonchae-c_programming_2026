#include <stdio.h>

int main() {
    float a = 10.0;
    int b = 3;

    int c = a/b; // a는 정수로 변환됨
    printf("%f / %d = %d\n", a, b, c);

    float d = a/b; // b는 소수형으로 변환됨
    printf("%f / %d = %f\n", a, b, d);

    int e = 10;
    float f = e / b; // e는 소수형으로 변환됨
    printf("%d / %d = %f\n", e, b, f);
    return 0;
}