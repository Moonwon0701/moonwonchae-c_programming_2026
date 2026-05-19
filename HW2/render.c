#include "render.h"

/* ─────────────────────────────────────────
   내부 헬퍼: OpenGL 2D 선분 하나 그리기
───────────────────────────────────────── */
static void drawSegment(float x0, float y0, float x1, float y1,
                        float thickness, Color c) {
    glLineWidth(thickness);
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_LINES);
        glVertex2f(x0, y0);
        glVertex2f(x1, y1);
    glEnd();
}

/* ─────────────────────────────────────────
   renderStrokeFixed
   - 모든 선분을 동일한 두께와 불투명도로 렌더링
───────────────────────────────────────── */
void renderStrokeFixed(const Stroke *stroke) {
    if (!stroke || stroke->pointCount < 2) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < stroke->pointCount - 1; i++) {
        drawSegment(stroke->points[i].x,   stroke->points[i].y,
                    stroke->points[i+1].x, stroke->points[i+1].y,
                    stroke->thickness, stroke->color);
    }
}

/* ─────────────────────────────────────────
   renderStrokeOpacity
   - 선분이 겹칠수록 투명도가 누적되어 진해짐
   - 각 선분을 낮은 alpha로 개별 렌더링
───────────────────────────────────────── */
void renderStrokeOpacity(const Stroke *stroke) {
    if (!stroke || stroke->pointCount < 2) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Color c = stroke->color;
    c.a = 0.15f;   // 낮은 alpha → 겹칠수록 누적

    for (int i = 0; i < stroke->pointCount - 1; i++) {
        drawSegment(stroke->points[i].x,   stroke->points[i].y,
                    stroke->points[i+1].x, stroke->points[i+1].y,
                    stroke->thickness, c);
    }
}

/* ─────────────────────────────────────────
   renderStrokeSpeed
   - 속도가 빠를수록 선이 얇아짐
   - speeds[] 배열 값을 두께에 반영
───────────────────────────────────────── */
void renderStrokeSpeed(const Stroke *stroke) {
    if (!stroke || stroke->pointCount < 2) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < stroke->pointCount - 1; i++) {
        /* speed가 클수록 두께 감소, 최소 1.0f 보장 */
        float spd = stroke->speeds[i];
        float t = stroke->thickness / (1.0f + spd * 0.05f);
        if (t < 1.0f) t = 1.0f;

        drawSegment(stroke->points[i].x,   stroke->points[i].y,
                    stroke->points[i+1].x, stroke->points[i+1].y,
                    t, stroke->color);
    }
}

/* ─────────────────────────────────────────
   renderStroke
   - penType에 따라 적절한 렌더 함수 디스패치
───────────────────────────────────────── */
void renderStroke(const Stroke *stroke) {
    if (!stroke) return;
    switch (stroke->penType) {
        case PEN_FIXED:   renderStrokeFixed(stroke);   break;
        case PEN_OPACITY: renderStrokeOpacity(stroke); break;
        case PEN_SPEED:   renderStrokeSpeed(stroke);   break;
        default:          renderStrokeFixed(stroke);   break;
    }
}

/* ─────────────────────────────────────────
   renderAllStrokes
   - 캔버스의 모든 획을 등록 순서대로 렌더링
───────────────────────────────────────── */
void renderAllStrokes(const Canvas *canvas) {
    if (!canvas) return;
    for (int i = 0; i < canvas->strokeCount; i++) {
        renderStroke(canvas->strokes[i]);
    }
}

/* ─────────────────────────────────────────
   renderBackground
   - 단색 배경 사각형 렌더링
───────────────────────────────────────── */
void renderBackground(Color bgColor) {
    glDisable(GL_BLEND);
    glColor4f(bgColor.r, bgColor.g, bgColor.b, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f( 1.0f, -1.0f);
        glVertex2f( 1.0f,  1.0f);
        glVertex2f(-1.0f,  1.0f);
    glEnd();
}

/* ─────────────────────────────────────────
   renderEraserCursor
   - 지우개 모드일 때 커서 위치에 원 표시
───────────────────────────────────────── */
void renderEraserCursor(float x, float y, float radius) {
    glDisable(GL_BLEND);
    glColor4f(0.4f, 0.4f, 0.4f, 1.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    int segs = 32;
    for (int i = 0; i < segs; i++) {
        float angle = 2.0f * 3.14159f * i / segs;
        /* 스크린 좌표 → NDC 변환 (간단화: 외부에서 NDC로 넘겨받는다고 가정) */
        glVertex2f(x + cosf(angle) * radius, y + sinf(angle) * radius);
    }
    glEnd();
}

/* ─────────────────────────────────────────
   renderScene3D
   - 3D 씬 전체 렌더링
   - 카메라 행렬 설정 후 각 획 렌더링
───────────────────────────────────────── */
void renderScene3D(const Scene *scene) {
    if (!scene) return;

    /* TODO: 프로젝션 / 뷰 행렬 설정 (OpenGL glMatrixMode 또는 셰이더 유니폼) */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* gluPerspective(scene->camera.fov, (float)WINDOW_W/WINDOW_H, 0.1f, 1000.0f); */

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    /* gluLookAt(camera.position, camera.target, camera.up) 적용 */

    /* 3D 획 렌더링 */
    for (int i = 0; i < scene->canvas->strokeCount; i++) {
        Stroke *s = scene->canvas->strokes[i];
        if (!s->is3D || s->pointCount < 2) continue;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glLineWidth(s->thickness);
        glColor4f(s->color.r, s->color.g, s->color.b, s->color.a);
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < s->pointCount; j++) {
            glVertex3f(s->points3D[j].x, s->points3D[j].y, s->points3D[j].z);
        }
        glEnd();
    }
}
