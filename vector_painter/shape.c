#include "shape.h"
#include "stroke.h"

static void boundingBox(const Stroke *s,
                        float *minX, float *minY,
                        float *maxX, float *maxY) {
    *minX = *minY =  1e9f;
    *maxX = *maxY = -1e9f;
    for (int i = 0; i < s->pointCount; i++) {
        if (s->points[i].x < *minX) *minX = s->points[i].x;
        if (s->points[i].y < *minY) *minY = s->points[i].y;
        if (s->points[i].x > *maxX) *maxX = s->points[i].x;
        if (s->points[i].y > *maxY) *maxY = s->points[i].y;
    }
}

static float maxDeviationFromLine(const Stroke *s) {
    if (s->pointCount < 2) return 0.0f;
    float x0 = s->points[0].x, y0 = s->points[0].y;
    float x1 = s->points[s->pointCount-1].x;
    float y1 = s->points[s->pointCount-1].y;
    float dx = x1-x0, dy = y1-y0;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 1e-6f) return 0.0f;
    float maxDev = 0.0f;
    for (int i = 1; i < s->pointCount-1; i++) {
        float px = s->points[i].x - x0;
        float py = s->points[i].y - y0;
        float cross = fabsf(px*dy - py*dx) / len;
        if (cross > maxDev) maxDev = cross;
    }
    return maxDev;
}

ShapeType detectShapeType(const Stroke *stroke) {
    if (!stroke || stroke->pointCount < 3) return SHAPE_NONE;
    float minX, minY, maxX, maxY;
    boundingBox(stroke, &minX, &minY, &maxX, &maxY);
    float w = maxX-minX, h = maxY-minY;
    float diag = sqrtf(w*w + h*h);
    if (maxDeviationFromLine(stroke) < diag * 0.08f) return SHAPE_LINE;
    float sx = stroke->points[0].x, sy = stroke->points[0].y;
    float ex = stroke->points[stroke->pointCount-1].x;
    float ey = stroke->points[stroke->pointCount-1].y;
    float closeDist = sqrtf((sx-ex)*(sx-ex) + (sy-ey)*(sy-ey));
    float aspect = (w > h) ? w/(h+1e-6f) : h/(w+1e-6f);
    if (closeDist < diag*0.25f && aspect < 1.5f) return SHAPE_CIRCLE;
    return SHAPE_RECT;
}

Stroke *correctToLine(const Stroke *stroke) {
    if (!stroke || stroke->pointCount < 2) return NULL;
    Stroke *s = createStroke(stroke->penType, stroke->color, stroke->thickness);
    addPointToStroke(s, stroke->points[0].x, stroke->points[0].y, 1.0f, 0.0f);
    addPointToStroke(s, stroke->points[stroke->pointCount-1].x,
                        stroke->points[stroke->pointCount-1].y, 1.0f, 0.0f);
    s->id = stroke->id;
    return s;
}

Stroke *correctToCircle(const Stroke *stroke) {
    if (!stroke || stroke->pointCount < 3) return NULL;
    float minX, minY, maxX, maxY;
    boundingBox(stroke, &minX, &minY, &maxX, &maxY);
    float cx = (minX+maxX)/2.0f, cy = (minY+maxY)/2.0f;
    float r  = ((maxX-minX) + (maxY-minY)) / 4.0f;
    Stroke *s = createStroke(stroke->penType, stroke->color, stroke->thickness);
    int segs = 64;
    for (int i = 0; i <= segs; i++) {
        float angle = 2.0f * 3.14159265f * i / segs;
        addPointToStroke(s, cx+cosf(angle)*r, cy+sinf(angle)*r, 1.0f, 0.0f);
    }
    s->id = stroke->id;
    return s;
}

Stroke *correctToRect(const Stroke *stroke) {
    if (!stroke || stroke->pointCount < 3) return NULL;
    float minX, minY, maxX, maxY;
    boundingBox(stroke, &minX, &minY, &maxX, &maxY);
    Stroke *s = createStroke(stroke->penType, stroke->color, stroke->thickness);
    addPointToStroke(s, minX, minY, 1.0f, 0.0f);
    addPointToStroke(s, maxX, minY, 1.0f, 0.0f);
    addPointToStroke(s, maxX, maxY, 1.0f, 0.0f);
    addPointToStroke(s, minX, maxY, 1.0f, 0.0f);
    addPointToStroke(s, minX, minY, 1.0f, 0.0f);
    s->id = stroke->id;
    return s;
}

Stroke *applyShapeCorrection(const Stroke *stroke) {
    switch (detectShapeType(stroke)) {
        case SHAPE_LINE:   return correctToLine(stroke);
        case SHAPE_CIRCLE: return correctToCircle(stroke);
        case SHAPE_RECT:   return correctToRect(stroke);
        default:           return NULL;
    }
}
