#include <stdio.h>
int main() {
    int a = 97;
    printf("a %%d: %d\n", a);
    printf("a %%i: %i\n", a);
    printf("a %%c: %c\n", a);
    printf("a %%f: %f\n", a);
    printf("-----\n"); 

    /*
    정수형 데이터 -> %c : ASCII 코드값에 해당하는 문자로 출력
    정수형 데이터 -> %f : 오류가 발생하고 0.000000
    */

    char b = 'A';
    printf("b %%d: %d\n", b);
    printf("b %%i: %i\n", b);
    printf("b %%c: %c\n", b + 32); // 'A' + 32 = 'a'
    printf("b %%f: %f\n", b);
    printf("-----\n"); 

    /*
    문자형 데이터 -> %d : ASCII 코드값에 해당하는 숫자로 출력
    문자형 데이터 + 정수 : ASCII 코드값에 해당하는 숫자로 계산
    */

return 0;
}