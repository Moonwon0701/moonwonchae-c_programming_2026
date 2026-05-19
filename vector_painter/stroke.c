#include "stroke.h"

Stroke *createStroke(PenType penType, Color color, float thickness) {
    Stroke *s = (Stroke *)malloc(sizeof(Stroke));
    if (!s) return NULL;
    memset(s, 0, sizeof(Stroke));
    s->penType   = penType;
    s->color     = color;
    s->thickness = thickness;
    return s;
}

void addPointToStroke(Stroke *stroke, float x, float y, float pressure, float speed) {
    if (!stroke || stroke->pointCount >= MAX_POINTS) return;
    int i = stroke->pointCount;
    stroke->points[i].x  = x;
    stroke->points[i].y  = y;
    stroke->pressures[i] = pressure;
    stroke->speeds[i]    = speed;
    stroke->pointCount++;
}

void freeStroke(Stroke *stroke) {
    if (stroke) free(stroke);
}

void addStrokeToCanvas(Canvas *canvas, Stroke *stroke) {
    if (!canvas || !stroke) return;
    if (canvas->strokeCount >= MAX_STROKES) return;
    stroke->id = canvas->nextId++;
    canvas->strokes[canvas->strokeCount++] = stroke;
}

bool removeStrokeFromCanvas(Canvas *canvas, int strokeId) {
    if (!canvas) return false;
    for (int i = 0; i < canvas->strokeCount; i++) {
        if (canvas->strokes[i]->id == strokeId) {
            freeStroke(canvas->strokes[i]);
            for (int j = i; j < canvas->strokeCount - 1; j++)
                canvas->strokes[j] = canvas->strokes[j + 1];
            canvas->strokes[--canvas->strokeCount] = NULL;
            return true;
        }
    }
    return false;
}

float distancePointToStroke(const Stroke *stroke, float x, float y) {
    if (!stroke || stroke->pointCount == 0) return 1e9f;
    float minDist = 1e9f;

    if (stroke->pointCount == 1) {
        float dx = x - stroke->points[0].x;
        float dy = y - stroke->points[0].y;
        return sqrtf(dx*dx + dy*dy);
    }

    for (int i = 0; i < stroke->pointCount - 1; i++) {
        float ax = stroke->points[i].x,   ay = stroke->points[i].y;
        float bx = stroke->points[i+1].x, by = stroke->points[i+1].y;
        float dx = bx - ax, dy = by - ay;
        float lenSq = dx*dx + dy*dy;
        float t = 0.0f;
        if (lenSq > 1e-6f) {
            t = ((x-ax)*dx + (y-ay)*dy) / lenSq;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
        }
        float cx = ax + t*dx, cy = ay + t*dy;
        float d = sqrtf((x-cx)*(x-cx) + (y-cy)*(y-cy));
        if (d < minDist) minDist = d;
    }
    return minDist;
}
