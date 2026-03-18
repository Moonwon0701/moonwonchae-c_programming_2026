#include<stdio.h>

int main() {
    int a = -3;
    printf("a %%d: %d\n", a);
    printf("a %%i: %i\n", a);
    printf("a %%c: %c\n", a);
    printf("a %%f: %f\n", (float)a);
    printf("a %%u: %u\n", a);
    printf("-----\n"); 

    /*
    음수인 정수 -> %d : 음수 그대로 출력
    음수인 정수 -> %u : 양수로 변환되어 출력 (2의 보수 표현)
        ex) -3 -> 4294967293 = 2^32 - 3
    */

    int b = 128;
    printf("b %%d: %d\n", b);
    printf("b %%i: %i\n", b);
    printf("b %%c: %c\n", b);
    printf("b %%f: %f\n", (float)b);
    printf("b %%e: %u\n", b);
    printf("-----\n"); 

    b = 127;
    printf("b %%d: %d\n", b);
    printf("b %%i: %i\n", b);
    printf("b %%c: %c\n", b);
    printf("b %%f: %f\n", (float)b);
    printf("b %%e: %u\n", b);
    printf("-----\n"); 

    unsigned char c = 254; 
    printf("c %%d: %d\n", c);
    printf("c %%i: %i\n", c);
    printf("c %%c: %c\n", c);
    printf("c %%f: %f\n", (float)c);
    printf("-----\n");


    char strc[5] = "abcd";
    printf("strc %%c: %c\n", strc);
    printf("strc %%d: %d\n", strc);
    printf("strc %%s: %s\n", strc);

    /*
    문자열 -> 종결 문자를 위한 공간 확보 해야함 -> 4글자면 공간 5개 필요
    */

    char strc2[5] = "abecdefg";
    printf("strc2 %%d: %d\n", strc2);
    printf("strc2 %%s: %s\n", strc2);
    printf("strc2 %%s: %s\n", strc2);

    return 0;

}