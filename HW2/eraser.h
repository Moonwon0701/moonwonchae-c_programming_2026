#ifndef ERASER_H
#define ERASER_H

#include "types.h"

/* 커서 반경 내 획 ID 목록 반환 (outIds 배열에 저장, 반환값: 개수) */
int  findStrokesNearCursor(const Canvas *canvas, float x, float y,
                           float radius, int *outIds, int maxOut);

/* 반경 내 획 전체 삭제 */
void eraseNearCursor(Canvas *canvas, float x, float y, float radius);

#endif /* ERASER_H */
