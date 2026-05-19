#include "types.h"
#include "stroke.h"
#include "render.h"
#include "eraser.h"
#include "fileio.h"
#include "shape.h"
#include "undo.h"

/* ═══════════════════════════════════════════
   전역 상태
═══════════════════════════════════════════ */
static AppState g_app;

/* ═══════════════════════════════════════════
   전방 선언
═══════════════════════════════════════════ */
static bool initWindow(void);
static void initCanvas(void);
static void initUI(void);
static void handleInput(void);
static void update(void);
static void render(void);
static void cleanup(void);

static void handleMouseDown(float x, float y);
static void handleMouseMove(float x, float y);
static void handleMouseUp(void);
static void handleKeyboard(SDL_Keycode key);
static float calcSpeed(float x, float y);

/* ═══════════════════════════════════════════
   main
═══════════════════════════════════════════ */
int main(void) {
    if (!initWindow()) return 1;
    initCanvas();
    initUI();

    while (!g_app.quit) {
        handleInput();
        update();
        render();
        SDL_Delay(16);
    }

    cleanup();
    return 0;
}

/* ═══════════════════════════════════════════
   초기화
═══════════════════════════════════════════ */
static bool initWindow(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "[ERROR] SDL_Init 실패: %s\n", SDL_GetError());
        return false;
    }

    g_app.window = SDL_CreateWindow(
        "Vector Painter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );
    if (!g_app.window) {
        fprintf(stderr, "[ERROR] 창 생성 실패: %s\n", SDL_GetError());
        return false;
    }

    g_app.renderer = SDL_CreateRenderer(
        g_app.window, -1,
        SDL_RENDERER_SOFTWARE   // OpenGL 없이 소프트웨어 렌더링
    );
    if (!g_app.renderer) {
        fprintf(stderr, "[ERROR] 렌더러 생성 실패: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawBlendMode(g_app.renderer, SDL_BLENDMODE_BLEND);
    g_app.quit = false;
    printf("[INFO] 초기화 완료\n");
    return true;
}

static void initCanvas(void) {
    g_app.canvas = (Canvas *)calloc(1, sizeof(Canvas));
    if (!g_app.canvas) { fprintf(stderr, "[ERROR] 캔버스 할당 실패\n"); exit(1); }
    g_app.canvas->bgColor = (Color){255, 255, 255, 255};
    g_app.canvas->nextId  = 1;
}

static void initUI(void) {
    g_app.ui.selectedPen   = PEN_FIXED;
    g_app.ui.toolMode      = MODE_PEN;
    g_app.ui.selectedColor = (Color){0, 0, 0, 255};
    g_app.ui.thickness     = 5.0f;
    g_app.ui.shapeCorrect  = false;
}

/* ═══════════════════════════════════════════
   입력 처리
═══════════════════════════════════════════ */
static void handleInput(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_QUIT:
            g_app.quit = true;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (e.button.button == SDL_BUTTON_LEFT) {
                g_app.mouseDown = true;
                handleMouseDown((float)e.button.x, (float)e.button.y);
            }
            break;

        case SDL_MOUSEMOTION:
            g_app.prevMouseX = g_app.mouseX;
            g_app.prevMouseY = g_app.mouseY;
            g_app.mouseX = (float)e.motion.x;
            g_app.mouseY = (float)e.motion.y;
            if (g_app.mouseDown)
                handleMouseMove((float)e.motion.x, (float)e.motion.y);
            break;

        case SDL_MOUSEBUTTONUP:
            if (e.button.button == SDL_BUTTON_LEFT) {
                g_app.mouseDown = false;
                handleMouseUp();
            }
            break;

        case SDL_KEYDOWN:
            handleKeyboard(e.key.keysym.sym);
            break;
        }
    }
}

static void handleMouseDown(float x, float y) {
    Canvas *cv = g_app.canvas;
    if (g_app.ui.toolMode == MODE_PEN) {
        cv->currentStroke = createStroke(
            g_app.ui.selectedPen,
            g_app.ui.selectedColor,
            g_app.ui.thickness
        );
        cv->isDrawing = true;
        addPointToStroke(cv->currentStroke, x, y, 1.0f, 0.0f);
    } else {
        eraseNearCursor(cv, x, y, ERASER_RADIUS);
    }
}

static void handleMouseMove(float x, float y) {
    Canvas *cv = g_app.canvas;
    if (g_app.ui.toolMode == MODE_PEN) {
        if (cv->isDrawing && cv->currentStroke) {
            float speed = calcSpeed(x, y);
            addPointToStroke(cv->currentStroke, x, y, 1.0f, speed);
        }
    } else {
        eraseNearCursor(cv, x, y, ERASER_RADIUS);
    }
}

static void handleMouseUp(void) {
    Canvas *cv = g_app.canvas;
    if (!cv->isDrawing || !cv->currentStroke) return;
    cv->isDrawing = false;

    /* 도형 보정 */
    if (g_app.ui.shapeCorrect) {
        Stroke *corrected = applyShapeCorrection(cv->currentStroke);
        if (corrected) {
            freeStroke(cv->currentStroke);
            cv->currentStroke = corrected;
        }
    }

    addStrokeToCanvas(cv, cv->currentStroke);
    pushUndoStack(cv, ACTION_ADD_STROKE, cv->currentStroke);
    cv->currentStroke = NULL;
}

static void handleKeyboard(SDL_Keycode key) {
    SDL_Keymod mod = SDL_GetModState();
    bool ctrl = (mod & KMOD_CTRL) != 0;

    switch (key) {
    case SDLK_z: if (ctrl) undo(g_app.canvas); break;
    case SDLK_y: if (ctrl) redo(g_app.canvas); break;
    case SDLK_s: if (ctrl) saveCanvas(g_app.canvas, "canvas.vp"); break;
    case SDLK_o:
        if (ctrl) {
            Canvas *loaded = loadCanvas("canvas.vp");
            if (loaded) {
                for (int i = 0; i < g_app.canvas->strokeCount; i++)
                    freeStroke(g_app.canvas->strokes[i]);
                free(g_app.canvas);
                g_app.canvas = loaded;
            }
        }
        break;
    case SDLK_e:
        g_app.ui.toolMode =
            (g_app.ui.toolMode == MODE_ERASER) ? MODE_PEN : MODE_ERASER;
        printf("[INFO] 모드: %s\n", g_app.ui.toolMode == MODE_ERASER ? "지우개" : "펜");
        break;
    case SDLK_F3:
        g_app.ui.shapeCorrect = !g_app.ui.shapeCorrect;
        printf("[INFO] 도형 보정: %s\n", g_app.ui.shapeCorrect ? "ON" : "OFF");
        break;
    /* 펜 종류 */
    case SDLK_1: g_app.ui.selectedPen = PEN_FIXED;   printf("[INFO] 펜: 고정\n");   break;
    case SDLK_2: g_app.ui.selectedPen = PEN_OPACITY;  printf("[INFO] 펜: 투명도\n"); break;
    case SDLK_3: g_app.ui.selectedPen = PEN_SPEED;    printf("[INFO] 펜: 속도\n");   break;
    /* 색상 */
    case SDLK_q: g_app.ui.selectedColor = (Color){0,   0,   0,   255}; break; // 검정
    case SDLK_w: g_app.ui.selectedColor = (Color){255, 0,   0,   255}; break; // 빨강
    case SDLK_r: g_app.ui.selectedColor = (Color){0,   0,   255, 255}; break; // 파랑
    case SDLK_t: g_app.ui.selectedColor = (Color){0,   180, 0,   255}; break; // 초록
    /* 두께 */
    case SDLK_EQUALS: g_app.ui.thickness += 2.0f; break; // +
    case SDLK_MINUS:
        g_app.ui.thickness -= 2.0f;
        if (g_app.ui.thickness < 1.0f) g_app.ui.thickness = 1.0f;
        break;
    case SDLK_ESCAPE:
        g_app.quit = true;
        break;
    }
}

/* ═══════════════════════════════════════════
   업데이트 / 렌더
═══════════════════════════════════════════ */
static void update(void) {
    /* 추후 확장 */
}

static void render(void) {
    renderBackground(g_app.renderer, g_app.canvas->bgColor);
    renderAllStrokes(g_app.renderer, g_app.canvas);

    /* 현재 그리는 중인 획 실시간 표시 */
    if (g_app.canvas->isDrawing && g_app.canvas->currentStroke)
        renderStroke(g_app.renderer, g_app.canvas->currentStroke);

    /* 지우개 커서 */
    if (g_app.ui.toolMode == MODE_ERASER)
        renderEraserCursor(g_app.renderer, g_app.mouseX, g_app.mouseY, ERASER_RADIUS);

    SDL_RenderPresent(g_app.renderer);
}

/* ═══════════════════════════════════════════
   종료
═══════════════════════════════════════════ */
static void cleanup(void) {
    for (int i = 0; i < g_app.canvas->strokeCount; i++)
        freeStroke(g_app.canvas->strokes[i]);
    if (g_app.canvas->currentStroke)
        freeStroke(g_app.canvas->currentStroke);
    free(g_app.canvas);
    SDL_DestroyRenderer(g_app.renderer);
    SDL_DestroyWindow(g_app.window);
    SDL_Quit();
    printf("[INFO] 종료\n");
}

static float calcSpeed(float x, float y) {
    float dx = x - g_app.prevMouseX;
    float dy = y - g_app.prevMouseY;
    return sqrtf(dx*dx + dy*dy);
}
