#include "fileio.h"
#include "stroke.h"

/*
   파일 포맷 (텍스트):
   ─────────────────────────────────────────
   STROKE id penType r g b a thickness pointCount
   POINT x y pressure speed
   POINT x y pressure speed
   ...
   END
   ─────────────────────────────────────────
*/

/* ─────────────────────────────────────────
   serializeStroke
   - 획 데이터를 위 포맷의 문자열로 변환
   - 반환된 char*는 호출자가 free() 해야 함
───────────────────────────────────────── */
char *serializeStroke(const Stroke *stroke) {
    if (!stroke) return NULL;

    /* 최대 버퍼 크기 추정: 헤더 + 점마다 약 60자 */
    int bufSize = 256 + stroke->pointCount * 64;
    char *buf = (char *)malloc(bufSize);
    if (!buf) return NULL;

    int offset = 0;
    offset += snprintf(buf + offset, bufSize - offset,
        "STROKE %d %d %.4f %.4f %.4f %.4f %.4f %d\n",
        stroke->id, (int)stroke->penType,
        stroke->color.r, stroke->color.g,
        stroke->color.b, stroke->color.a,
        stroke->thickness, stroke->pointCount);

    for (int i = 0; i < stroke->pointCount; i++) {
        offset += snprintf(buf + offset, bufSize - offset,
            "POINT %.4f %.4f %.4f %.4f\n",
            stroke->points[i].x, stroke->points[i].y,
            stroke->pressures[i], stroke->speeds[i]);
    }

    offset += snprintf(buf + offset, bufSize - offset, "END\n");
    return buf;
}

/* ─────────────────────────────────────────
   deserializeStroke
   - 문자열에서 Stroke 복원
───────────────────────────────────────── */
Stroke *deserializeStroke(const char *str) {
    if (!str) return NULL;

    int id, penType, pointCount;
    float r, g, b, a, thickness;

    /* 헤더 파싱 */
    const char *ptr = str;
    if (sscanf(ptr, "STROKE %d %d %f %f %f %f %f %d",
               &id, &penType, &r, &g, &b, &a, &thickness, &pointCount) != 8) {
        fprintf(stderr, "[ERROR] deserializeStroke: 헤더 파싱 실패\n");
        return NULL;
    }

    Color col = {r, g, b, a};
    Stroke *s = createStroke((PenType)penType, col, thickness);
    if (!s) return NULL;
    s->id = id;

    /* 첫 번째 POINT 행으로 이동 */
    ptr = strchr(ptr, '\n');
    if (!ptr) { freeStroke(s); return NULL; }
    ptr++;

    for (int i = 0; i < pointCount; i++) {
        float x, y, pressure, speed;
        if (sscanf(ptr, "POINT %f %f %f %f", &x, &y, &pressure, &speed) == 4) {
            addPointToStroke(s, x, y, pressure, speed);
        }
        ptr = strchr(ptr, '\n');
        if (!ptr) break;
        ptr++;
    }

    return s;
}

/* ─────────────────────────────────────────
   saveCanvas
   - 모든 획을 파일로 저장
───────────────────────────────────────── */
bool saveCanvas(const Canvas *canvas, const char *filepath) {
    if (!canvas || !filepath) return false;

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        fprintf(stderr, "[ERROR] saveCanvas: 파일 열기 실패 (%s)\n", filepath);
        return false;
    }

    fprintf(fp, "CANVAS_VERSION 1\n");
    fprintf(fp, "STROKE_COUNT %d\n", canvas->strokeCount);

    for (int i = 0; i < canvas->strokeCount; i++) {
        char *s = serializeStroke(canvas->strokes[i]);
        if (s) {
            fputs(s, fp);
            free(s);
        }
    }

    fclose(fp);
    printf("[INFO] 저장 완료: %s\n", filepath);
    return true;
}

/* ─────────────────────────────────────────
   loadCanvas
   - 파일에서 캔버스 복원
   - 반환된 Canvas*는 호출자가 해제해야 함
───────────────────────────────────────── */
Canvas *loadCanvas(const char *filepath) {
    if (!filepath) return NULL;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "[ERROR] loadCanvas: 파일 열기 실패 (%s)\n", filepath);
        return NULL;
    }

    Canvas *canvas = (Canvas *)calloc(1, sizeof(Canvas));
    if (!canvas) { fclose(fp); return NULL; }
    canvas->bgColor = (Color){1.0f, 1.0f, 1.0f, 1.0f};

    char line[64];
    int strokeCount = 0;

    /* 헤더 읽기 */
    fgets(line, sizeof(line), fp); // CANVAS_VERSION
    fscanf(fp, "STROKE_COUNT %d\n", &strokeCount);

    /* 각 획 읽기 */
    char strokeBuf[MAX_POINTS * 64 + 256];
    for (int i = 0; i < strokeCount; i++) {
        strokeBuf[0] = '\0';
        char rowBuf[128];

        /* "STROKE" 헤더부터 "END"까지 한 덩어리로 수집 */
        while (fgets(rowBuf, sizeof(rowBuf), fp)) {
            strncat(strokeBuf, rowBuf, sizeof(strokeBuf) - strlen(strokeBuf) - 1);
            if (strncmp(rowBuf, "END", 3) == 0) break;
        }

        Stroke *s = deserializeStroke(strokeBuf);
        if (s) addStrokeToCanvas(canvas, s);
    }

    fclose(fp);
    printf("[INFO] 불러오기 완료: %s (%d획)\n", filepath, canvas->strokeCount);
    return canvas;
}
