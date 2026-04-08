#include <stdio.h>

int main()  {
    int a = 10;
    int* p = &a;

    printf("a = %d\n", a);
    printf("p = %p\n", p);
    printf("*p = %d\n", *p);
    printf("Address of a: %p\n", &a);
    printf("Address of p: %p\n", &p);
    
    return 0;
}