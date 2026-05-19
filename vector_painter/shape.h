#ifndef SHAPE_H
#define SHAPE_H

#include "types.h"

/* 획이 어떤 도형에 가까운지 판별 */
ShapeType detectShapeType(const Stroke *stroke);

/* 각 도형으로 보정된 새 획 반환 (원본 교체용) */
Stroke *correctToLine(const Stroke *stroke);
Stroke *correctToCircle(const Stroke *stroke);
Stroke *correctToRect(const Stroke *stroke);

/* 자동 판별 후 보정 디스패치 */
Stroke *applyShapeCorrection(const Stroke *stroke);

#endif /* SHAPE_H */
