#include <stdio.h>

int main() {
    int result = 0;
    int ad = -1;
    while(1){
        scanf("%d", &ad);
        if(ad == 0) {
            break;
        }
        result += ad;
    }
    printf("더한 결과는 %d입니다.\n", result);
    return 0;
}