#include <stdio.h>

int main() {
    int abc = 1;
    int* other = &abc;
    int* other2 = other;

    printf("abc: %d\n", abc);
    printf("other: %d\n", *other);
    printf("other2: %d\n", *other2);
    abc++;
    printf("abc: %d\n", abc);
    printf("other: %d\n", *other);
    printf("other2: %d\n", *other2);
    // *other2++;
    // *(other2++);
    other2 = other2 + 1;

    printf("abc address: %p\n", &abc);
    printf("other2 *address: %p\n", *other2);
    printf("other2 address: %p\n", &other2);

    *other2;
    // (*other2)++;  -> abc++;
    // *other2 = *other2 + 1;
    printf("abc: %d\n", abc);
    printf("other: %d\n", *other);
    printf("other2: %d\n", *other2);
}