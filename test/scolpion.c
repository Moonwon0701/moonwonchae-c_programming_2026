// creature_cursor.c
// Interactive creature (scorpion-like) that follows the mouse cursor
// Compile: gcc creature_cursor.c -o creature_cursor -lraylib -lm
// Or with pkg-config: gcc creature_cursor.c -o creature_cursor $(pkg-config --libs --cflags raylib) -lm

#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SPINE_SEGMENTS  12
#define MAX_LEGS            8   // pairs
#define TAIL_SEGMENTS       8
#define LERP_SPEED          0.18f
#define SEGMENT_LENGTH      22.0f
#define TAIL_SEG_LENGTH     16.0f

typedef struct {
    Vector2 pos;
    float   angle;
} Segment;

// Spine (body chain)
Segment spine[MAX_SPINE_SEGMENTS];

// Tail chain
Segment tail[TAIL_SEGMENTS];

// Leg target positions (IK)
typedef struct {
    Vector2 tip;        // current foot position
    Vector2 target;     // desired foot position
    float   stepTimer;
    int     stepping;
} Leg;

Leg legs[MAX_LEGS * 2]; // left and right

// ---- math helpers ----
static float Angle(Vector2 a, Vector2 b) {
    return atan2f(b.y - a.y, b.x - a.x);
}

static Vector2 Vec2FromAngle(float angle, float len) {
    return (Vector2){ cosf(angle) * len, sinf(angle) * len };
}

static Vector2 Vec2Add(Vector2 a, Vector2 b) {
    return (Vector2){ a.x + b.x, a.y + b.y };
}

static Vector2 Vec2Sub(Vector2 a, Vector2 b) {
    return (Vector2){ a.x - b.x, a.y - b.y };
}

static float Vec2Len(Vector2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

static Vector2 Vec2Lerp(Vector2 a, Vector2 b, float t) {
    return (Vector2){ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

static Vector2 Vec2Norm(Vector2 v) {
    float len = Vec2Len(v);
    if (len < 0.0001f) return (Vector2){0, 0};
    return (Vector2){ v.x / len, v.y / len };
}

// Follow chain: each segment follows the one in front
static void UpdateChain(Segment *chain, int count, Vector2 head, float segLen) {
    chain[0].pos = head;
    for (int i = 1; i < count; i++) {
        float ang = Angle(chain[i].pos, chain[i-1].pos);
        chain[i].angle = ang;
        Vector2 offset = Vec2FromAngle(ang + (float)M_PI, segLen);
        chain[i].pos = Vec2Add(chain[i-1].pos, offset);
    }
}

// Draw an ellipse body segment
static void DrawBodySegment(Vector2 pos, float angle, float w, float h, Color col) {
    DrawEllipse((int)pos.x, (int)pos.y, (int)w, (int)h, col);
    // outline
    DrawEllipseLines((int)pos.x, (int)pos.y, (int)w, (int)h, ColorAlpha(WHITE, 0.5f));
}

// Draw a leg as a 2-joint IK limb
static void DrawLeg(Vector2 root, Vector2 tip, float reach, Color col) {
    // Simple 2-bone IK: mid-point is offset perpendicular
    float dist = Vec2Len(Vec2Sub(tip, root));
    float halfLen = reach * 0.55f;

    // Mid joint: find point forming triangle
    Vector2 mid;
    if (dist < reach * 1.9f) {
        float a = (halfLen * halfLen - halfLen * halfLen + dist * dist) / (2.0f * dist);
        float h2 = sqrtf(fmaxf(0, halfLen * halfLen - a * a));
        Vector2 dir = Vec2Norm(Vec2Sub(tip, root));
        Vector2 perp = { -dir.y, dir.x };
        mid = Vec2Add(Vec2Add(root, (Vector2){ dir.x * a, dir.y * a }),
                      (Vector2){ perp.x * h2, perp.y * h2 });
    } else {
        mid = Vec2Lerp(root, tip, 0.5f);
    }

    DrawLineEx(root, mid, 1.5f, col);
    DrawLineEx(mid, tip, 1.5f, col);
    // Claw at tip
    Vector2 clawDir = Vec2Norm(Vec2Sub(tip, mid));
    Vector2 perp = { -clawDir.y * 4, clawDir.x * 4 };
    Vector2 clawEnd = Vec2Add(tip, (Vector2){ clawDir.x * 6, clawDir.y * 6 });
    DrawLineEx(tip, Vec2Add(clawEnd, perp), 1.2f, col);
    DrawLineEx(tip, Vec2Sub(clawEnd, perp), 1.2f, col);
}

// Pincers (front claws)
static void DrawPincer(Vector2 base, float angle, float side, Color col) {
    float len = 30.0f;
    float openAng = 0.4f * side;
    Vector2 mid = Vec2Add(base, Vec2FromAngle(angle, len * 0.5f));
    Vector2 upper = Vec2Add(mid, Vec2FromAngle(angle + openAng, len * 0.5f));
    Vector2 lower = Vec2Add(mid, Vec2FromAngle(angle - openAng, len * 0.5f));
    DrawLineEx(base, mid, 2.0f, col);
    DrawLineEx(mid, upper, 2.0f, col);
    DrawLineEx(mid, lower, 2.0f, col);
    // claw tips
    DrawCircleV(upper, 2.5f, WHITE);
    DrawCircleV(lower, 2.5f, WHITE);
}

int main(void) {
    const int W = 900, H = 600;
    InitWindow(W, H, "Creature Cursor - raylib");
    SetTargetFPS(60);
    HideCursor();

    // Initialize spine
    for (int i = 0; i < MAX_SPINE_SEGMENTS; i++) {
        spine[i].pos = (Vector2){ W/2.0f + i * SEGMENT_LENGTH, H/2.0f };
        spine[i].angle = (float)M_PI;
    }

    // Initialize tail
    for (int i = 0; i < TAIL_SEGMENTS; i++) {
        tail[i].pos = spine[MAX_SPINE_SEGMENTS-1].pos;
        tail[i].angle = (float)M_PI;
    }

    // Initialize legs
    for (int i = 0; i < MAX_LEGS * 2; i++) {
        legs[i].tip = spine[2 + (i/2)].pos;
        legs[i].target = legs[i].tip;
        legs[i].stepTimer = 0;
        legs[i].stepping = 0;
    }

    Vector2 mouseSmooth = { W/2.0f, H/2.0f };
    float time = 0;

    while (!WindowShouldClose()) {
        time += GetFrameTime();
        Vector2 mouse = GetMousePosition();

        // Smooth mouse following
        mouseSmooth = Vec2Lerp(mouseSmooth, mouse, LERP_SPEED);

        // Update spine chain
        UpdateChain(spine, MAX_SPINE_SEGMENTS, mouseSmooth, SEGMENT_LENGTH);

        // Update tail from last spine segment
        // Tail curves upward (scorpion-like)
        Vector2 tailBase = spine[MAX_SPINE_SEGMENTS - 1].pos;
        float tailBaseAngle = spine[MAX_SPINE_SEGMENTS - 2].angle + (float)M_PI;
        Vector2 tailHead = Vec2Add(tailBase, Vec2FromAngle(tailBaseAngle, TAIL_SEG_LENGTH));
        UpdateChain(tail, TAIL_SEGMENTS, tailHead, TAIL_SEG_LENGTH);

        // Update legs with simple step logic
        for (int i = 0; i < MAX_LEGS; i++) {
            int spineIdx = 2 + i;
            if (spineIdx >= MAX_SPINE_SEGMENTS) spineIdx = MAX_SPINE_SEGMENTS - 1;

            Vector2 anchor = spine[spineIdx].pos;
            float ang = spine[spineIdx].angle;
            float reach = 36.0f;

            // Left leg (index i*2)
            Vector2 leftTarget = Vec2Add(anchor, Vec2FromAngle(ang + (float)M_PI_2 + 0.3f, reach));
            leftTarget.x += sinf(time * 3.0f + i) * 3.0f;
            // Right leg (index i*2+1)
            Vector2 rightTarget = Vec2Add(anchor, Vec2FromAngle(ang - (float)M_PI_2 - 0.3f, reach));
            rightTarget.x += sinf(time * 3.0f + i + (float)M_PI) * 3.0f;

            // Step if too far from target
            float leftDist = Vec2Len(Vec2Sub(leftTarget, legs[i*2].tip));
            float rightDist = Vec2Len(Vec2Sub(rightTarget, legs[i*2+1].tip));

            if (leftDist > 20.0f) {
                legs[i*2].tip = Vec2Lerp(legs[i*2].tip, leftTarget, 0.25f);
            }
            if (rightDist > 20.0f) {
                legs[i*2+1].tip = Vec2Lerp(legs[i*2+1].tip, rightTarget, 0.25f);
            }
        }

        // ---- Draw ----
        BeginDrawing();
        ClearBackground((Color){15, 15, 20, 255});

        // Draw subtle grid
        for (int x = 0; x < W; x += 40)
            DrawLine(x, 0, x, H, (Color){30, 30, 40, 255});
        for (int y = 0; y < H; y += 40)
            DrawLine(0, y, W, y, (Color){30, 30, 40, 255});

        Color bodyCol = (Color){200, 210, 220, 255};
        Color legCol  = (Color){170, 185, 200, 255};
        Color tailCol = (Color){190, 200, 215, 255};

        // Draw tail
        for (int i = TAIL_SEGMENTS - 1; i >= 1; i--) {
            float t = 1.0f - (float)i / TAIL_SEGMENTS;
            float w = 4.0f * (1.0f - t * 0.7f);
            DrawLineEx(tail[i].pos, tail[i-1].pos, w, tailCol);
            // ribs on tail
            Vector2 perp = Vec2FromAngle(tail[i].angle + (float)M_PI_2, 5.0f * (1.0f - t));
            DrawLineEx(Vec2Add(tail[i].pos, perp), Vec2Sub(tail[i].pos, perp), 1.0f, ColorAlpha(WHITE, 0.4f));
        }
        // Stinger
        {
            Vector2 sting = Vec2Add(tail[0].pos, Vec2FromAngle(tail[0].angle, 18.0f));
            DrawLineEx(tail[0].pos, sting, 2.0f, WHITE);
            DrawCircleV(sting, 3.5f, WHITE);
        }

        // Draw legs
        for (int i = 0; i < MAX_LEGS; i++) {
            int spineIdx = 2 + i;
            if (spineIdx >= MAX_SPINE_SEGMENTS) spineIdx = MAX_SPINE_SEGMENTS - 1;
            Vector2 root = spine[spineIdx].pos;
            float reach = 36.0f;
            DrawLeg(root, legs[i*2].tip,   reach, legCol);
            DrawLeg(root, legs[i*2+1].tip, reach, legCol);
        }

        // Draw body segments (spine)
        for (int i = MAX_SPINE_SEGMENTS - 1; i >= 1; i--) {
            float t = (float)i / MAX_SPINE_SEGMENTS;
            float segW = 4.0f + (1.0f - t) * 8.0f;
            float segH = 3.0f + (1.0f - t) * 5.0f;

            // Body ellipses
            DrawBodySegment(spine[i].pos, spine[i].angle, segW, segH, (Color){40, 45, 55, 255});

            // Spine line
            DrawLineEx(spine[i].pos, spine[i-1].pos, 2.5f * (1.0f - t * 0.5f), bodyCol);

            // Ribs / cross lines
            if (i % 1 == 0) {
                float ribLen = segW * 1.2f;
                Vector2 perp = Vec2FromAngle(spine[i].angle + (float)M_PI_2, ribLen);
                DrawLineEx(Vec2Add(spine[i].pos, perp), Vec2Sub(spine[i].pos, perp), 1.0f, ColorAlpha(WHITE, 0.35f));
            }
        }

        // Draw head
        {
            Vector2 head = spine[0].pos;
            float ang = spine[0].angle;
            // Head circle
            DrawCircleV(head, 9.0f, (Color){50, 55, 65, 255});
            DrawCircleLines((int)head.x, (int)head.y, 9, bodyCol);
            // Eyes
            Vector2 eyeL = Vec2Add(head, Vec2FromAngle(ang + 1.1f, 7.0f));
            Vector2 eyeR = Vec2Add(head, Vec2FromAngle(ang - 1.1f, 7.0f));
            DrawCircleV(eyeL, 2.5f, WHITE);
            DrawCircleV(eyeR, 2.5f, WHITE);
            DrawCircleV(eyeL, 1.0f, BLACK);
            DrawCircleV(eyeR, 1.0f, BLACK);
            // Antennae
            Vector2 ant1 = Vec2Add(head, Vec2FromAngle(ang + 0.3f, 28.0f));
            Vector2 ant2 = Vec2Add(head, Vec2FromAngle(ang - 0.3f, 28.0f));
            DrawLineEx(head, ant1, 1.0f, ColorAlpha(WHITE, 0.6f));
            DrawLineEx(head, ant2, 1.0f, ColorAlpha(WHITE, 0.6f));
            DrawCircleV(ant1, 2.0f, WHITE);
            DrawCircleV(ant2, 2.0f, WHITE);
            // Pincers
            DrawPincer(head, ang + 0.2f,  1.0f, bodyCol);
            DrawPincer(head, ang - 0.2f, -1.0f, bodyCol);
        }

        // Custom cursor dot
        DrawCircleV(mouse, 3.0f, (Color){255, 80, 80, 200});
        DrawCircleLines((int)mouse.x, (int)mouse.y, 8, (Color){255, 80, 80, 120});

        // UI
        DrawText("Creature Cursor | raylib", 10, 10, 16, (Color){80, 90, 100, 255});
        DrawText("Move your mouse!", 10, 30, 14, (Color){60, 70, 80, 255});
        DrawText("ESC to quit", 10, H - 24, 13, (Color){60, 70, 80, 255});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}