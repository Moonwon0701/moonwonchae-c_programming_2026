#ifndef STROKE_H
#define STROKE_H

#include "types.h"

Stroke *createStroke(PenType penType, Color color, float thickness);
void    addPointToStroke(Stroke *stroke, float x, float y, float pressure, float speed);
void    freeStroke(Stroke *stroke);
void    addStrokeToCanvas(Canvas *canvas, Stroke *stroke);
bool    removeStrokeFromCanvas(Canvas *canvas, int strokeId);
float   distancePointToStroke(const Stroke *stroke, float x, float y);

#endif
