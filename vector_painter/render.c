#include "render.h"

/* ─────────────────────────────────────────
   두꺼운 선 그리기 (SDL은 기본 1px라 원으로 두께 표현)
───────────────────────────────────────── */
static void drawThickLine(SDL_Renderer *renderer,
                          float x0, float y0, float x1, float y1,
                          float thickness, Color c, Uint8 alpha) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, alpha);

    float dx = x1 - x0, dy = y1 - y0;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 1.0f) {
        /* 점 하나: 원으로 표시 */
        int r = (int)(thickness / 2.0f);
        for (int dy2 = -r; dy2 <= r; dy2++)
            for (int dx2 = -r; dx2 <= r; dx2++)
                if (dx2*dx2 + dy2*dy2 <= r*r)
                    SDL_RenderDrawPoint(renderer, (int)x0+dx2, (int)y0+dy2);
        return;
    }

    /* 선분을 thickness/2 반경의 원들로 채워서 두껍게 */
    int r = (int)(thickness / 2.0f);
    if (r < 1) r = 1;
    int steps = (int)(len) + 1;
    for (int s = 0; s <= steps; s++) {
        float t = (float)s / steps;
        float cx = x0 + t * dx;
        float cy = y0 + t * dy;
        for (int dy2 = -r; dy2 <= r; dy2++)
            for (int dx2 = -r; dx2 <= r; dx2++)
                if (dx2*dx2 + dy2*dy2 <= r*r)
                    SDL_RenderDrawPoint(renderer, (int)cx+dx2, (int)cy+dy2);
    }
}

/* ─────────────────────────────────────────
   renderBackground
───────────────────────────────────────── */
void renderBackground(SDL_Renderer *renderer, Color bgColor) {
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
    SDL_RenderClear(renderer);
}

/* ─────────────────────────────────────────
   renderStroke - penType에 따라 렌더링
───────────────────────────────────────── */
void renderStroke(SDL_Renderer *renderer, const Stroke *stroke) {
    if (!stroke || stroke->pointCount < 2) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < stroke->pointCount - 1; i++) {
        float x0 = stroke->points[i].x,   y0 = stroke->points[i].y;
        float x1 = stroke->points[i+1].x, y1 = stroke->points[i+1].y;
        float thickness = stroke->thickness;
        Uint8 alpha = stroke->color.a;

        switch (stroke->penType) {
            case PEN_FIXED:
                /* 두께 고정, 완전 불투명 */
                drawThickLine(renderer, x0, y0, x1, y1,
                              thickness, stroke->color, alpha);
                break;

            case PEN_OPACITY:
                /* 낮은 alpha로 겹칠수록 진해짐 */
                drawThickLine(renderer, x0, y0, x1, y1,
                              thickness, stroke->color, 40);
                break;

            case PEN_SPEED: {
                /* 빠를수록 얇아짐 */
                float spd = stroke->speeds[i];
                float t = thickness / (1.0f + spd * 0.05f);
                if (t < 1.0f) t = 1.0f;
                drawThickLine(renderer, x0, y0, x1, y1,
                              t, stroke->color, alpha);
                break;
            }
        }
    }
}

/* ─────────────────────────────────────────
   renderAllStrokes
───────────────────────────────────────── */
void renderAllStrokes(SDL_Renderer *renderer, const Canvas *canvas) {
    if (!canvas) return;
    for (int i = 0; i < canvas->strokeCount; i++)
        renderStroke(renderer, canvas->strokes[i]);
}

/* ─────────────────────────────────────────
   renderEraserCursor - 원형 커서 표시
───────────────────────────────────────── */
void renderEraserCursor(SDL_Renderer *renderer, float x, float y, float radius) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 200);
    int segs = 32;
    for (int i = 0; i < segs; i++) {
        float a0 = 2.0f * 3.14159f * i / segs;
        float a1 = 2.0f * 3.14159f * (i+1) / segs;
        SDL_RenderDrawLine(renderer,
            (int)(x + cosf(a0)*radius), (int)(y + sinf(a0)*radius),
            (int)(x + cosf(a1)*radius), (int)(y + sinf(a1)*radius));
    }
}
