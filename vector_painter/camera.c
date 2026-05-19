#include "camera.h"
#include "stroke.h"

/* ─────────────────────────────────────────
   initCamera
   - 카메라 초기 위치, 방향, FOV 설정
───────────────────────────────────────── */
void initCamera(Scene *scene) {
    if (!scene) return;
    scene->camera.position = (Vec3){0.0f, 0.0f, 500.0f};
    scene->camera.target   = (Vec3){0.0f, 0.0f,   0.0f};
    scene->camera.up       = (Vec3){0.0f, 1.0f,   0.0f};
    scene->camera.fov      = 60.0f;
    scene->camera.yaw      = 0.0f;
    scene->camera.pitch    = 0.0f;
    scene->camera.zoom     = 1.0f;
}

/* ─────────────────────────────────────────
   handleCameraInput
   - 마우스 드래그 → 카메라 회전 (yaw/pitch)
   - 스크롤 → 줌 (zoom)
───────────────────────────────────────── */
void handleCameraInput(Scene *scene, float dx, float dy, float zoom) {
    if (!scene) return;
    float sensitivity = 0.3f;
    scene->camera.yaw   += dx * sensitivity;
    scene->camera.pitch += dy * sensitivity;

    /* pitch 범위 제한 */
    if (scene->camera.pitch >  89.0f) scene->camera.pitch =  89.0f;
    if (scene->camera.pitch < -89.0f) scene->camera.pitch = -89.0f;

    scene->camera.zoom *= (1.0f - zoom * 0.1f);
    if (scene->camera.zoom < 0.1f) scene->camera.zoom = 0.1f;
    if (scene->camera.zoom > 10.0f) scene->camera.zoom = 10.0f;

    /* 카메라 위치 재계산 (구면 좌표) */
    float r = 500.0f / scene->camera.zoom;
    float yawR   = scene->camera.yaw   * 3.14159f / 180.0f;
    float pitchR = scene->camera.pitch * 3.14159f / 180.0f;

    scene->camera.position.x = r * cosf(pitchR) * sinf(yawR);
    scene->camera.position.y = r * sinf(pitchR);
    scene->camera.position.z = r * cosf(pitchR) * cosf(yawR);
}

/* ─────────────────────────────────────────
   projectScreenTo3D
   - 화면 좌표(픽셀)를 3D 공간 좌표로 변환
   - 간략화: z=0 평면에 투영 (ray-plane intersection)
───────────────────────────────────────── */
Vec3 projectScreenTo3D(const Camera *camera, float screenX, float screenY) {
    /* NDC 변환 */
    float ndcX = (screenX / WINDOW_W) * 2.0f - 1.0f;
    float ndcY = 1.0f - (screenY / WINDOW_H) * 2.0f;

    /* 간단한 역투영: z=0 평면 위의 점으로 매핑 */
    float halfH = tanf(camera->fov * 0.5f * 3.14159f / 180.0f);
    float halfW = halfH * (float)WINDOW_W / WINDOW_H;

    Vec3 dir = {
        ndcX * halfW,
        ndcY * halfH,
        -1.0f
    };

    /* 카메라 → 월드 변환 (간략화: yaw/pitch 회전 미적용) */
    float t = -camera->position.z / dir.z;
    Vec3 world = {
        camera->position.x + t * dir.x,
        camera->position.y + t * dir.y,
        0.0f
    };
    return world;
}

/* ─────────────────────────────────────────
   addStroke3D
   - 획의 2D 점들을 지정된 3D 평면 위 좌표로 변환하여 배치
───────────────────────────────────────── */
void addStroke3D(Scene *scene, Stroke *stroke, Vec3 planeNormal, Vec3 planeOrigin) {
    if (!scene || !stroke) return;

    stroke->is3D = true;
    for (int i = 0; i < stroke->pointCount; i++) {
        /* 간략화: XY 평면에 배치 (planeNormal = (0,0,1) 가정) */
        stroke->points3D[i].x = stroke->points[i].x + planeOrigin.x;
        stroke->points3D[i].y = stroke->points[i].y + planeOrigin.y;
        stroke->points3D[i].z = planeOrigin.z;
        (void)planeNormal; // 추후 임의 평면 지원 시 사용
    }

    addStrokeToCanvas(scene->canvas, stroke);
}
