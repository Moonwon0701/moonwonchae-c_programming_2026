#include "eraser.h"
#include "stroke.h"

/* ─────────────────────────────────────────
   findStrokesNearCursor
   - 커서(x, y)로부터 radius 이내에 있는 획 ID 목록 반환
   - outIds: 결과 ID 배열 (호출자 제공)
   - maxOut: 배열 최대 크기
   - 반환값: 찾은 획 수
───────────────────────────────────────── */
int findStrokesNearCursor(const Canvas *canvas, float x, float y,
                          float radius, int *outIds, int maxOut) {
    if (!canvas || !outIds) return 0;

    int count = 0;
    for (int i = 0; i < canvas->strokeCount && count < maxOut; i++) {
        float dist = distancePointToStroke(canvas->strokes[i], x, y);
        if (dist <= radius) {
            outIds[count++] = canvas->strokes[i]->id;
        }
    }
    return count;
}

/* ─────────────────────────────────────────
   eraseNearCursor
   - 커서 반경 내 모든 획을 캔버스에서 삭제
   - 뒤에서부터 삭제해 인덱스 어긋남 방지
───────────────────────────────────────── */
void eraseNearCursor(Canvas *canvas, float x, float y, float radius) {
    if (!canvas) return;

    int ids[MAX_STROKES];
    int n = findStrokesNearCursor(canvas, x, y, radius, ids, MAX_STROKES);

    for (int i = 0; i < n; i++) {
        removeStrokeFromCanvas(canvas, ids[i]);
    }
}
