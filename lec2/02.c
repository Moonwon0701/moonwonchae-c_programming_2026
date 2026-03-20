#include<stdio.h>

int main() {
    int a;
    if(a = 0) printf("print 0 run!\n"); // 할당하는 값에 따라 true / false임
    if(1) printf("print 1 run!\n");
    if(-5) printf("print -5 run!\n");   // 0외의 값은 True임

    if(a == 0) {
        printf("ha ha ha\n");
        printf("a = 0\n");
    }

    a = 6;
    if(a >= -5 && a < 4){
        printf("correct");
    }

    return 0;
}