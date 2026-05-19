#include "types.h"
#include "stroke.h"
#include "render.h"
#include "eraser.h"
#include "fileio.h"
#include "shape.h"
#include "undo.h"
#include "camera.h"

/* ═══════════════════════════════════════════
   전방 선언
═══════════════════════════════════════════ */
static AppState g_app;

static bool initWindow(void);
static void initCanvas(void);
static void initUI(void);
static void handleInput(void);
static void update(void);
static void render(void);
static void cleanup(void);

/* 입력 세부 처리 */
static void handleMouseDown(float x, float y);
static void handleMouseMove(float x, float y);
static void handleMouseUp(void);
static void handleKeyboard(SDL_Keycode key);

/* 좌표 변환 */
static float screenToNDC_X(float px);
static float screenToNDC_Y(float py);
static float calcSpeed(float x, float y);

/* ═══════════════════════════════════════════
   main
═══════════════════════════════════════════ */
int main(void) {
    /* ── 초기화 ── */
    if (!initWindow()) return 1;
    initCanvas();
    initUI();

    /* ── 메인 루프 ── */
    while (!g_app.quit) {
        handleInput();
        update();
        render();
        SDL_Delay(16); // ~60fps
    }

    /* ── 종료 ── */
    cleanup();
    return 0;
}

/* ═══════════════════════════════════════════
   초기화 함수들
═══════════════════════════════════════════ */

/* ─────────────────────────────────────────
   initWindow
   - SDL2 + OpenGL 초기화
   - 반환: 성공 시 true
───────────────────────────────────────── */
static bool initWindow(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "[ERROR] SDL_Init 실패: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // 안티앨리어싱

    g_app.window = SDL_CreateWindow(
        "Vector Painter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    if (!g_app.window) {
        fprintf(stderr, "[ERROR] 창 생성 실패: %s\n", SDL_GetError());
        return false;
    }

    g_app.glContext = SDL_GL_CreateContext(g_app.window);
    if (!g_app.glContext) {
        fprintf(stderr, "[ERROR] GL 컨텍스트 생성 실패: %s\n", SDL_GetError());
        return false;
    }

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "[ERROR] GLEW 초기화 실패: %s\n", glewGetErrorString(err));
        return false;
    }

    SDL_GL_SetSwapInterval(1); // VSync
    glViewport(0, 0, WINDOW_W, WINDOW_H);

    /* 2D 정사영 투영 설정 */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    g_app.quit = false;
    return true;
}

/* ─────────────────────────────────────────
   initCanvas
   - Canvas 구조체 초기화
───────────────────────────────────────── */
static void initCanvas(void) {
    g_app.canvas = (Canvas *)calloc(1, sizeof(Canvas));
    if (!g_app.canvas) {
        fprintf(stderr, "[ERROR] initCanvas: 메모리 할당 실패\n");
        exit(1);
    }
    g_app.canvas->bgColor  = (Color){1.0f, 1.0f, 1.0f, 1.0f};
    g_app.canvas->nextId   = 1;

    /* 씬 설정 */
    g_app.scene.canvas   = g_app.canvas;
    g_app.scene.is3DMode = false;
    initCamera(&g_app.scene);
}

/* ─────────────────────────────────────────
   initUI
   - UIState 기본값 설정
───────────────────────────────────────── */
static void initUI(void) {
    g_app.ui.selectedPen   = PEN_FIXED;
    g_app.ui.toolMode      = MODE_PEN;
    g_app.ui.selectedColor = (Color){0.0f, 0.0f, 0.0f, 1.0f}; // 검정
    g_app.ui.thickness     = 3.0f;
    g_app.ui.shapeCorrect  = false;
    g_app.ui.show3D        = false;
}

/* ═══════════════════════════════════════════
   메인 루프 함수들
═══════════════════════════════════════════ */

/* ─────────────────────────────────────────
   handleInput
   - SDL 이벤트 큐 폴링 및 분기
───────────────────────────────────────── */
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
            if (g_app.mouseDown) {
                handleMouseMove((float)e.motion.x, (float)e.motion.y);
            }
            g_app.prevMouseX = g_app.mouseX;
            g_app.prevMouseY = g_app.mouseY;
            g_app.mouseX = (float)e.motion.x;
            g_app.mouseY = (float)e.motion.y;
            break;

        case SDL_MOUSEBUTTONUP:
            if (e.button.button == SDL_BUTTON_LEFT) {
                g_app.mouseDown = false;
                handleMouseUp();
            }
            break;

        case SDL_MOUSEWHEEL:
            if (g_app.scene.is3DMode) {
                handleCameraInput(&g_app.scene, 0.0f, 0.0f,
                                  (float)e.wheel.y * -1.0f);
            }
            break;

        case SDL_KEYDOWN:
            handleKeyboard(e.key.keysym.sym);
            break;
        }
    }
}

/* ─────────────────────────────────────────
   handleMouseDown
   - 펜 모드: 새 획 시작
   - 지우개 모드: 즉시 지우기 시작
───────────────────────────────────────── */
static void handleMouseDown(float x, float y) {
    Canvas *cv = g_app.canvas;

    if (g_app.ui.toolMode == MODE_PEN) {
        cv->currentStroke = createStroke(
            g_app.ui.selectedPen,
            g_app.ui.selectedColor,
            g_app.ui.thickness
        );
        cv->isDrawing = true;

        addPointToStroke(cv->currentStroke,
                         screenToNDC_X(x), screenToNDC_Y(y),
                         1.0f, 0.0f);
    } else {
        /* 지우개: 클릭 즉시 지우기 */
        eraseNearCursor(cv,
                        screenToNDC_X(x), screenToNDC_Y(y),
                        ERASER_RADIUS / (WINDOW_W / 2.0f));
    }
}

/* ─────────────────────────────────────────
   handleMouseMove
   - 펜 모드: 현재 획에 점 추가
   - 지우개 모드: 이동하며 지우기
   - 3D 모드: 카메라 회전 처리
───────────────────────────────────────── */
static void handleMouseMove(float x, float y) {
    Canvas *cv = g_app.canvas;
    float ndcX = screenToNDC_X(x);
    float ndcY = screenToNDC_Y(y);

    if (g_app.ui.toolMode == MODE_PEN) {
        if (cv->isDrawing && cv->currentStroke) {
            float speed = calcSpeed(x, y);
            addPointToStroke(cv->currentStroke, ndcX, ndcY, 1.0f, speed);
        }
    } else {
        eraseNearCursor(cv, ndcX, ndcY,
                        ERASER_RADIUS / (WINDOW_W / 2.0f));
    }

    /* 3D 카메라 회전 (우클릭 드래그로 분리 가능) */
    if (g_app.scene.is3DMode) {
        float dx = x - g_app.prevMouseX;
        float dy = y - g_app.prevMouseY;
        /* 필요 시 우클릭 드래그 조건 추가 */
    }
}

/* ─────────────────────────────────────────
   handleMouseUp
   - 획 완료: 캔버스에 등록, 도형 보정 적용
───────────────────────────────────────── */
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

    /* 3D 모드: 획을 3D 평면에 배치 */
    if (g_app.scene.is3DMode) {
        Vec3 planeNormal = {0.0f, 0.0f, 1.0f};
        Vec3 planeOrigin = {0.0f, 0.0f, 0.0f};
        addStroke3D(&g_app.scene, cv->currentStroke, planeNormal, planeOrigin);
    } else {
        addStrokeToCanvas(cv, cv->currentStroke);
    }

    /* Undo 스택에 추가 */
    pushUndoStack(cv, ACTION_ADD_STROKE, cv->currentStroke);
    cv->currentStroke = NULL;
}

/* ─────────────────────────────────────────
   handleKeyboard
   - 단축키 처리
───────────────────────────────────────── */
static void handleKeyboard(SDL_Keycode key) {
    SDL_Keymod mod = SDL_GetModState();
    bool ctrl = (mod & KMOD_CTRL) != 0;

    switch (key) {
    case SDLK_z:
        if (ctrl) undo(g_app.canvas);
        break;
    case SDLK_y:
        if (ctrl) redo(g_app.canvas);
        break;
    case SDLK_s:
        if (ctrl) saveCanvas(g_app.canvas, "canvas.vp");
        break;
    case SDLK_o:
        if (ctrl) {
            Canvas *loaded = loadCanvas("canvas.vp");
            if (loaded) {
                /* 기존 캔버스 해제 후 교체 */
                for (int i = 0; i < g_app.canvas->strokeCount; i++)
                    freeStroke(g_app.canvas->strokes[i]);
                free(g_app.canvas);
                g_app.canvas        = loaded;
                g_app.scene.canvas  = loaded;
            }
        }
        break;
    case SDLK_e:
        g_app.ui.toolMode =
            (g_app.ui.toolMode == MODE_ERASER) ? MODE_PEN : MODE_ERASER;
        break;
    case SDLK_F3:
        g_app.ui.shapeCorrect = !g_app.ui.shapeCorrect;
        printf("[INFO] 도형 보정: %s\n", g_app.ui.shapeCorrect ? "ON" : "OFF");
        break;
    case SDLK_F4:
        g_app.scene.is3DMode = !g_app.scene.is3DMode;
        printf("[INFO] 3D 모드: %s\n", g_app.scene.is3DMode ? "ON" : "OFF");
        break;
    case SDLK_1: g_app.ui.selectedPen = PEN_FIXED;   break;
    case SDLK_2: g_app.ui.selectedPen = PEN_OPACITY;  break;
    case SDLK_3: g_app.ui.selectedPen = PEN_SPEED;    break;
    case SDLK_ESCAPE:
        g_app.quit = true;
        break;
    }
}

/* ─────────────────────────────────────────
   update
   - 상태 업데이트 (현재는 handleInput에서 대부분 처리)
───────────────────────────────────────── */
static void update(void) {
    /* 추후 애니메이션, 타이머 기반 로직 추가 가능 */
}

/* ─────────────────────────────────────────
   render
   - 화면 전체 렌더링
───────────────────────────────────────── */
static void render(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* 배경 */
    renderBackground(g_app.canvas->bgColor);

    /* 획 렌더링 */
    if (g_app.scene.is3DMode) {
        renderScene3D(&g_app.scene);
    } else {
        renderAllStrokes(g_app.canvas);
    }

    /* 현재 그리는 중인 획 실시간 표시 */
    if (g_app.canvas->isDrawing && g_app.canvas->currentStroke) {
        renderStroke(g_app.canvas->currentStroke);
    }

    /* 지우개 커서 원 표시 */
    if (g_app.ui.toolMode == MODE_ERASER) {
        float r = ERASER_RADIUS / (WINDOW_W / 2.0f);
        renderEraserCursor(screenToNDC_X(g_app.mouseX),
                           screenToNDC_Y(g_app.mouseY), r);
    }

    SDL_GL_SwapWindow(g_app.window);
}

/* ─────────────────────────────────────────
   cleanup
   - 모든 리소스 해제
───────────────────────────────────────── */
static void cleanup(void) {
    /* 획 메모리 해제 */
    for (int i = 0; i < g_app.canvas->strokeCount; i++) {
        freeStroke(g_app.canvas->strokes[i]);
    }
    if (g_app.canvas->currentStroke)
        freeStroke(g_app.canvas->currentStroke);
    free(g_app.canvas);

    /* OpenGL / SDL 종료 */
    SDL_GL_DeleteContext(g_app.glContext);
    SDL_DestroyWindow(g_app.window);
    SDL_Quit();
    printf("[INFO] 정상 종료\n");
}

/* ═══════════════════════════════════════════
   유틸 함수
═══════════════════════════════════════════ */

/* 픽셀 X → NDC (-1.0 ~ 1.0) */
static float screenToNDC_X(float px) {
    return (px / WINDOW_W) * 2.0f - 1.0f;
}

/* 픽셀 Y → NDC (SDL Y축 반전 보정) */
static float screenToNDC_Y(float py) {
    return 1.0f - (py / WINDOW_H) * 2.0f;
}

/* 마우스 이동 속도 계산 (픽셀/프레임) */
static float calcSpeed(float x, float y) {
    float dx = x - g_app.prevMouseX;
    float dy = y - g_app.prevMouseY;
    return sqrtf(dx*dx + dy*dy);
}
