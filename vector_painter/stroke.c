#include "stroke.h"

/* ─────────────────────────────────────────
   createStroke
   - 새 획 객체를 힙에 할당하고 초기화
   - 반환: 초기화된 Stroke 포인터
───────────────────────────────────────── */
Stroke *createStroke(PenType penType, Color color, float thickness) {
    Stroke *s = (Stroke *)malloc(sizeof(Stroke));
    if (!s) {
        fprintf(stderr, "[ERROR] createStroke: 메모리 할당 실패\n");
        return NULL;
    }
    memset(s, 0, sizeof(Stroke));
    s->penType   = penType;
    s->color     = color;
    s->thickness = thickness;
    s->pointCount = 0;
    s->is3D      = false;
    return s;
}

/* ─────────────────────────────────────────
   addPointToStroke
   - 획에 2D 점 추가 (최대 MAX_POINTS까지)
───────────────────────────────────────── */
void addPointToStroke(Stroke *stroke, float x, float y, float pressure, float speed) {
    if (!stroke || stroke->pointCount >= MAX_POINTS) return;

    int i = stroke->pointCount;
    stroke->points[i].x    = x;
    stroke->points[i].y    = y;
    stroke->pressures[i]   = pressure;
    stroke->speeds[i]      = speed;
    stroke->pointCount++;
}

/* ─────────────────────────────────────────
   freeStroke
   - 획 메모리 해제
───────────────────────────────────────── */
void freeStroke(Stroke *stroke) {
    if (stroke) free(stroke);
}

/* ─────────────────────────────────────────
   addStrokeToCanvas
   - 완성된 획을 캔버스 목록에 등록
   - 획 ID 자동 부여
───────────────────────────────────────── */
void addStrokeToCanvas(Canvas *canvas, Stroke *stroke) {
    if (!canvas || !stroke) return;
    if (canvas->strokeCount >= MAX_STROKES) {
        fprintf(stderr, "[WARN] addStrokeToCanvas: 최대 획 수 초과\n");
        return;
    }
    stroke->id = canvas->nextId++;
    canvas->strokes[canvas->strokeCount++] = stroke;
}

/* ─────────────────────────────────────────
   removeStrokeFromCanvas
   - ID로 획을 찾아 목록에서 제거
   - 반환: 성공 시 true
───────────────────────────────────────── */
bool removeStrokeFromCanvas(Canvas *canvas, int strokeId) {
    if (!canvas) return false;

    for (int i = 0; i < canvas->strokeCount; i++) {
        if (canvas->strokes[i]->id == strokeId) {
            freeStroke(canvas->strokes[i]);
            /* 뒤 원소 앞으로 당기기 */
            for (int j = i; j < canvas->strokeCount - 1; j++) {
                canvas->strokes[j] = canvas->strokes[j + 1];
            }
            canvas->strokes[--canvas->strokeCount] = NULL;
            return true;
        }
    }
    return false;
}

/* ─────────────────────────────────────────
   distancePointToStroke
   - 점 (x, y)와 획 사이의 최소 거리 반환
   - 각 선분까지 거리 중 최솟값 사용
───────────────────────────────────────── */
float distancePointToStroke(const Stroke *stroke, float x, float y) {
    if (!stroke || stroke->pointCount == 0) return 1e9f;

    float minDist = 1e9f;

    for (int i = 0; i < stroke->pointCount - 1; i++) {
        float ax = stroke->points[i].x,   ay = stroke->points[i].y;
        float bx = stroke->points[i+1].x, by = stroke->points[i+1].y;

        /* 선분 AB에 대한 점 P의 최근접 거리 */
        float dx = bx - ax, dy = by - ay;
        float lenSq = dx*dx + dy*dy;
        float t = 0.0f;
        if (lenSq > 1e-6f) {
            t = ((x - ax)*dx + (y - ay)*dy) / lenSq;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
        }
        float cx = ax + t*dx, cy = ay + t*dy;
        float d = sqrtf((x-cx)*(x-cx) + (y-cy)*(y-cy));
        if (d < minDist) minDist = d;
    }

    /* 점이 1개인 획 처리 */
    if (stroke->pointCount == 1) {
        float d = sqrtf((x - stroke->points[0].x)*(x - stroke->points[0].x)
                      + (y - stroke->points[0].y)*(y - stroke->points[0].y));
        if (d < minDist) minDist = d;
    }

    return minDist;
}
