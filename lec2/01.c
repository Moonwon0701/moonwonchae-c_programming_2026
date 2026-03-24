#include<stdio.h>

int main() {
    int a = 3; 
    int b = 0;
    printf("a: %d\n", !a);
    printf("b: %d\n", !b);

    printf("%d\n", a||b);
    printf("%d\n", a&&b);

    printf("3 == 2 : %c\n", 3 == 2 ? 'T' : 'F');
    printf("3 > 2 : %c\n", 3 > 2 ? 'T' : 'F');
    printf("3 < 2 : %c\n", 3 < 2 ? 'T' : 'F');

    printf("%d\n", 0.1 + 0.9 == 1.0);
    return 0;
}