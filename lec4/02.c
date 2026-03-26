#include <stdio.h>

int main() {
    int a = 10;
    float b = 3.14;
    char c = 'D'; // 문자형 상수는 작은 따옴표.

    // 문자 비교
    if ('a' < 'b') {
        printf("b(%c-%d) is greater than a(%c-%d).\n", 'b', 'b', 'a', 'a');
    }

    // 대문자로 전환
    printf("before c: %c\n", c);
    if ('a' <= c && c <= 'z') {
        c = c - 'a' + 'A';
    }
    printf("after c: %c\n", c);

    return 0;
}