#ifndef STROKE_H
#define STROKE_H

#include "types.h"

/* 생성 / 소멸 */
Stroke *createStroke(PenType penType, Color color, float thickness);
void    addPointToStroke(Stroke *stroke, float x, float y, float pressure, float speed);
void    freeStroke(Stroke *stroke);

/* 캔버스 획 관리 */
void addStrokeToCanvas(Canvas *canvas, Stroke *stroke);
bool removeStrokeFromCanvas(Canvas *canvas, int strokeId);

/* 유틸 */
float distancePointToStroke(const Stroke *stroke, float x, float y);

#endif /* STROKE_H */
