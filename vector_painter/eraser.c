#include "eraser.h"
#include "stroke.h"

int findStrokesNearCursor(const Canvas *canvas, float x, float y,
                          float radius, int *outIds, int maxOut) {
    if (!canvas || !outIds) return 0;
    int count = 0;
    for (int i = 0; i < canvas->strokeCount && count < maxOut; i++) {
        float dist = distancePointToStroke(canvas->strokes[i], x, y);
        if (dist <= radius)
            outIds[count++] = canvas->strokes[i]->id;
    }
    return count;
}

void eraseNearCursor(Canvas *canvas, float x, float y, float radius) {
    if (!canvas) return;
    int ids[MAX_STROKES];
    int n = findStrokesNearCursor(canvas, x, y, radius, ids, MAX_STROKES);
    for (int i = 0; i < n; i++)
        removeStrokeFromCanvas(canvas, ids[i]);
}
