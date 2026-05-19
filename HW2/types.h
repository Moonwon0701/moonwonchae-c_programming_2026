#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/* ─────────────────────────────────────────
   상수 정의
───────────────────────────────────────── */
#define WINDOW_W        1280
#define WINDOW_H        800
#define MAX_POINTS      4096      // 획 하나당 최대 점 수
#define MAX_STROKES     1024      // 캔버스 최대 획 수
#define UNDO_STACK_SIZE 64
#define ERASER_RADIUS   30.0f
#define MAX_FILEPATH    256

/* ─────────────────────────────────────────
   열거형
───────────────────────────────────────── */
typedef enum {
    PEN_FIXED    = 0,   // 두께 고정
    PEN_OPACITY  = 1,   // 투명도 반응
    PEN_SPEED    = 2    // 속도 반응
} PenType;

typedef enum {
    MODE_PEN    = 0,
    MODE_ERASER = 1
} ToolMode;

typedef enum {
    SHAPE_NONE   = 0,
    SHAPE_LINE   = 1,
    SHAPE_CIRCLE = 2,
    SHAPE_RECT   = 3
} ShapeType;

typedef enum {
    ACTION_ADD_STROKE    = 0,
    ACTION_REMOVE_STROKE = 1
} ActionType;

/* ─────────────────────────────────────────
   기본 자료형
───────────────────────────────────────── */
typedef struct {
    float x, y;
} Vec2;

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float r, g, b, a;
} Color;

/* ─────────────────────────────────────────
   획(Stroke) 구조체
───────────────────────────────────────── */
typedef struct {
    int    id;
    PenType penType;
    Color  color;
    float  thickness;

    Vec2   points[MAX_POINTS];
    float  pressures[MAX_POINTS]; // 압력 (미래 확장용)
    float  speeds[MAX_POINTS];    // 속도 (PEN_SPEED용)
    int    pointCount;

    bool   is3D;
    Vec3   points3D[MAX_POINTS];  // 3D 모드 전용
} Stroke;

/* ─────────────────────────────────────────
   Undo/Redo 액션
───────────────────────────────────────── */
typedef struct {
    ActionType type;
    Stroke     stroke;   // 추가/삭제된 획 복사본
} Action;

/* ─────────────────────────────────────────
   캔버스 구조체
───────────────────────────────────────── */
typedef struct {
    Stroke *strokes[MAX_STROKES];
    int     strokeCount;
    int     nextId;

    Stroke *currentStroke;   // 현재 그리는 중인 획
    bool    isDrawing;

    Action  undoStack[UNDO_STACK_SIZE];
    int     undoTop;
    Action  redoStack[UNDO_STACK_SIZE];
    int     redoTop;

    Color   bgColor;
} Canvas;

/* ─────────────────────────────────────────
   카메라 (3D 모드)
───────────────────────────────────────── */
typedef struct {
    Vec3  position;
    Vec3  target;
    Vec3  up;
    float fov;
    float yaw, pitch;
    float zoom;
} Camera;

/* ─────────────────────────────────────────
   씬 (3D 모드)
───────────────────────────────────────── */
typedef struct {
    Canvas *canvas;
    Camera  camera;
    bool    is3DMode;
} Scene;

/* ─────────────────────────────────────────
   UI 상태
───────────────────────────────────────── */
typedef struct {
    PenType  selectedPen;
    ToolMode toolMode;
    Color    selectedColor;
    float    thickness;
    bool     shapeCorrect;
    bool     show3D;
} UIState;

/* ─────────────────────────────────────────
   전역 앱 상태
───────────────────────────────────────── */
typedef struct {
    SDL_Window   *window;
    SDL_GLContext glContext;
    Canvas       *canvas;
    Scene         scene;
    UIState       ui;
    bool          quit;
    float         mouseX, mouseY;
    float         prevMouseX, prevMouseY;
    bool          mouseDown;
} AppState;

#endif /* TYPES_H */
