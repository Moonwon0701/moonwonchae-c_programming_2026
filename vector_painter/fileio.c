#include "fileio.h"
#include "stroke.h"

char *serializeStroke(const Stroke *stroke) {
    if (!stroke) return NULL;
    int bufSize = 256 + stroke->pointCount * 64;
    char *buf = (char *)malloc(bufSize);
    if (!buf) return NULL;
    int offset = 0;
    offset += snprintf(buf + offset, bufSize - offset,
        "STROKE %d %d %d %d %d %d %.4f %d\n",
        stroke->id, (int)stroke->penType,
        stroke->color.r, stroke->color.g,
        stroke->color.b, stroke->color.a,
        stroke->thickness, stroke->pointCount);
    for (int i = 0; i < stroke->pointCount; i++)
        offset += snprintf(buf + offset, bufSize - offset,
            "POINT %.4f %.4f %.4f %.4f\n",
            stroke->points[i].x, stroke->points[i].y,
            stroke->pressures[i], stroke->speeds[i]);
    offset += snprintf(buf + offset, bufSize - offset, "END\n");
    return buf;
}

Stroke *deserializeStroke(const char *str) {
    if (!str) return NULL;
    int id, penType, r, g, b, a, pointCount;
    float thickness;
    if (sscanf(str, "STROKE %d %d %d %d %d %d %f %d",
               &id, &penType, &r, &g, &b, &a, &thickness, &pointCount) != 8)
        return NULL;
    Color col = {(Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a};
    Stroke *s = createStroke((PenType)penType, col, thickness);
    if (!s) return NULL;
    s->id = id;
    const char *ptr = strchr(str, '\n');
    if (!ptr) { freeStroke(s); return NULL; }
    ptr++;
    for (int i = 0; i < pointCount; i++) {
        float x, y, pressure, speed;
        if (sscanf(ptr, "POINT %f %f %f %f", &x, &y, &pressure, &speed) == 4)
            addPointToStroke(s, x, y, pressure, speed);
        ptr = strchr(ptr, '\n');
        if (!ptr) break;
        ptr++;
    }
    return s;
}

bool saveCanvas(const Canvas *canvas, const char *filepath) {
    if (!canvas || !filepath) return false;
    FILE *fp = fopen(filepath, "w");
    if (!fp) return false;
    fprintf(fp, "CANVAS_VERSION 1\nSTROKE_COUNT %d\n", canvas->strokeCount);
    for (int i = 0; i < canvas->strokeCount; i++) {
        char *s = serializeStroke(canvas->strokes[i]);
        if (s) { fputs(s, fp); free(s); }
    }
    fclose(fp);
    printf("[INFO] 저장 완료: %s\n", filepath);
    return true;
}

Canvas *loadCanvas(const char *filepath) {
    if (!filepath) return NULL;
    FILE *fp = fopen(filepath, "r");
    if (!fp) return NULL;
    Canvas *canvas = (Canvas *)calloc(1, sizeof(Canvas));
    if (!canvas) { fclose(fp); return NULL; }
    canvas->bgColor = (Color){255, 255, 255, 255};
    int strokeCount = 0;
    char line[64];
    fgets(line, sizeof(line), fp);
    fscanf(fp, "STROKE_COUNT %d\n", &strokeCount);
    char strokeBuf[MAX_POINTS * 64 + 256];
    for (int i = 0; i < strokeCount; i++) {
        strokeBuf[0] = '\0';
        char rowBuf[128];
        while (fgets(rowBuf, sizeof(rowBuf), fp)) {
            strncat(strokeBuf, rowBuf, sizeof(strokeBuf) - strlen(strokeBuf) - 1);
            if (strncmp(rowBuf, "END", 3) == 0) break;
        }
        Stroke *s = deserializeStroke(strokeBuf);
        if (s) addStrokeToCanvas(canvas, s);
    }
    fclose(fp);
    return canvas;
}
