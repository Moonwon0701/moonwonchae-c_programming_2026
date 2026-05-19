#ifndef SHAPE_H
#define SHAPE_H

#include "types.h"

ShapeType detectShapeType(const Stroke *stroke);
Stroke   *correctToLine(const Stroke *stroke);
Stroke   *correctToCircle(const Stroke *stroke);
Stroke   *correctToRect(const Stroke *stroke);
Stroke   *applyShapeCorrection(const Stroke *stroke);

#endif
