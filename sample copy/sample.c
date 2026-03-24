/**
 * sample.c - 별(*)로 다이아몬드 모양 출력하기
 *
 * 간단한 for loop 예제
 */
#include <stdio.h>

int main() {
  float height = 7; // 다이아몬드의 높이

  printf("=== 별로 다이아몬드 만들기 ===\n\n");

  for (int i = 1; i <= height/2 + 0.5; i++) {
    // 공백 출력 (오른쪽 정렬용)
    for (int j = 1; j <= height/2 + 0.5 - i; j++) {
      printf(" ");
    }
    // 별 출력 (홀수 개씩: 1, 3, 5, 7, 9...)
    for (int k = 1; k <= 2 * i - 1; k++) {
      printf("*");
    }
    printf("\n");
  }

  for (int i = 1; i <= height/2 - 0.5; i++) {
    // 공백 출력 (오른쪽 정렬용)
    for (int j = 1; j <= i; j++) {
      printf(" ");
    }
    // 별 출력 (홀수 개씩: 1, 3, 5, 7, 9...)
    for (int k = 1; k <= height -2 * i; k++) {
      printf("*");
    }
    printf("\n");
  }

  return 0;
}
