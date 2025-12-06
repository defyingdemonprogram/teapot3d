#ifndef VEC_H_
#define VEC_H_

#include <math.h>

typedef struct {
    float x, y;
} Vector2;

static inline Vector2 make_vector2(float x, float y) {
    return (Vector2) {.x = x, .y = y};
}

typedef struct {
    float x, y, z;
} Vector3;

static inline Vector3 make_vector3(float x, float y, float z) {
    return (Vector3) {.x = x, .y = y, .z = z};
}

static inline float vector3_dot(Vector3 a, Vector3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static inline Vector3 rotate_y(Vector3 p, float delta_angle) {
    float angle = atan2f(p.z, p.x) + delta_angle;
    float mag = sqrtf(p.x * p.x + p.z * p.z);
    return make_vector3(cosf(angle) * mag, p.y, sinf(angle) * mag);
}

static inline Vector2 project_3d_to_2d(Vector3 v) {
    if (v.z < 0) v.z = -v.z;
    if (v.z < 1e-6) v.z += 1e-6;
    return make_vector2(v.x / v.z, v.y / v.z);
}

#endif // VEC_H_
