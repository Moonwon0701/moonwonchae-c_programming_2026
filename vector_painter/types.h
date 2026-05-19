#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
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
#define MAX_POINTS      4096
#define MAX_STROKES     1024
#define UNDO_STACK_SIZE 64
#define ERASER_RADIUS   30.0f
#define MAX_FILEPATH    256

/* ─────────────────────────────────────────
   열거형
───────────────────────────────────────── */
typedef enum {
    PEN_FIXED    = 0,
    PEN_OPACITY  = 1,
    PEN_SPEED    = 2
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
    Uint8 r, g, b, a;
} Color;

/* ─────────────────────────────────────────
   획(Stroke) 구조체
───────────────────────────────────────── */
typedef struct {
    int     id;
    PenType penType;
    Color   color;
    float   thickness;

    Vec2    points[MAX_POINTS];
    float   pressures[MAX_POINTS];
    float   speeds[MAX_POINTS];
    int     pointCount;
} Stroke;

/* ─────────────────────────────────────────
   Undo/Redo 액션
───────────────────────────────────────── */
typedef struct {
    ActionType type;
    Stroke     stroke;
} Action;

/* ─────────────────────────────────────────
   캔버스 구조체
───────────────────────────────────────── */
typedef struct {
    Stroke *strokes[MAX_STROKES];
    int     strokeCount;
    int     nextId;

    Stroke *currentStroke;
    bool    isDrawing;

    Action  undoStack[UNDO_STACK_SIZE];
    int     undoTop;
    Action  redoStack[UNDO_STACK_SIZE];
    int     redoTop;

    Color   bgColor;
} Canvas;

/* ─────────────────────────────────────────
   UI 상태
───────────────────────────────────────── */
typedef struct {
    PenType  selectedPen;
    ToolMode toolMode;
    Color    selectedColor;
    float    thickness;
    bool     shapeCorrect;
} UIState;

/* ─────────────────────────────────────────
   전역 앱 상태
───────────────────────────────────────── */
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    Canvas       *canvas;
    UIState       ui;
    bool          quit;
    float         mouseX, mouseY;
    float         prevMouseX, prevMouseY;
    bool          mouseDown;
} AppState;

#endif /* TYPES_H */
