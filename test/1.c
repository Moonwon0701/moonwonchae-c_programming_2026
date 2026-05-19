#include <stdio.h>

int main()
{
    int num1;
    float f1;
    char c1;

    printf("정수 입력 : ");
    scanf("%d", &num1);

    printf("실수 입력 : ");
    scanf("%f", &f1);

    printf("문자 입력 : ");
    scanf("%c", &c1);

    printf("입력한 값은 각각 [%d], [%f] 및 [%c]입니다.\n", num1, f1, c1);
    return 0;
}