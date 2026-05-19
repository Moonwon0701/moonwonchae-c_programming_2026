#ifndef RENDER_H
#define RENDER_H

#include "types.h"

/* 단일 획 렌더링 (penType 자동 디스패치) */
void renderStroke(const Stroke *stroke);
void renderStrokeFixed(const Stroke *stroke);
void renderStrokeOpacity(const Stroke *stroke);
void renderStrokeSpeed(const Stroke *stroke);

/* 전체 캔버스 렌더링 */
void renderAllStrokes(const Canvas *canvas);

/* 배경 */
void renderBackground(Color bgColor);

/* 지우개 커서 원 표시 */
void renderEraserCursor(float x, float y, float radius);

/* 3D 렌더링 */
void renderScene3D(const Scene *scene);

#endif /* RENDER_H */
