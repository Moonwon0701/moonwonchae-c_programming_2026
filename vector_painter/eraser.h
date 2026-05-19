#ifndef ERASER_H
#define ERASER_H

#include "types.h"

int  findStrokesNearCursor(const Canvas *canvas, float x, float y,
                           float radius, int *outIds, int maxOut);
void eraseNearCursor(Canvas *canvas, float x, float y, float radius);

#endif
