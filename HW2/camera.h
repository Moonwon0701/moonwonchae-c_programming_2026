#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"

void initCamera(Scene *scene);
void handleCameraInput(Scene *scene, float dx, float dy, float zoom);
Vec3 projectScreenTo3D(const Camera *camera, float screenX, float screenY);
void addStroke3D(Scene *scene, Stroke *stroke, Vec3 planeNormal, Vec3 planeOrigin);

#endif /* CAMERA_H */
