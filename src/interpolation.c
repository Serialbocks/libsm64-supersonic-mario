#include "interpolation.h"
#include "decomp/engine/math_util.h"
#include <stdlib.h>

#define ANGLE_90_DEGREES 16384

u32 interpolationInterval;
u32 interpolationFrame;
u32 interpolationOffset;
f32 interpolationFactorA;
f32 interpolationFactorB;

void set_interpolation_interval(u32 interval) {
    interpolationFrame *= interpolationInterval;
    interpolationFrame /= interval;
    interpolationInterval = interval;
    interpolationOffset = 0;
    if (interpolationInterval > 1) {
        interpolationFactorA = 1.0f;
        interpolationFactorB = 0.0f;
    } else {
        interpolationFactorA = 0.0f;
        interpolationFactorB = 1.0f;
    }
}

void reset_interpolation(void) {
    set_interpolation_interval(interpolationInterval);
}

f32 get_interpolation_delta_time(void) {
    return 1.0f / (30.0f * (f32) interpolationInterval);
}

void increment_interpolation_frame(void) {
    interpolationFrame = interpolationFrame + 1;
    if (interpolationInterval > 1) {
        interpolationOffset = interpolationFrame % interpolationInterval;
        interpolationFactorB = (f32) interpolationOffset / (f32) interpolationInterval;
        interpolationFactorA = 1 - interpolationFactorB;
    } else {
        interpolationOffset = 0;
        interpolationFactorA = 0.0f;
        interpolationFactorB = 1.0f;
    }
}

u16 get_interpolation_area_update_counter(void) {
    return interpolationFrame / interpolationInterval;
}

u8 get_interpolation_should_update(void) {
    return interpolationOffset == 0;
}

u8 get_interpolation_gonna_update(void) {
    return interpolationOffset == (interpolationInterval - 1);
}

f32 f32_interpolate(f32 a, f32 b) {
    return a * interpolationFactorA + b * interpolationFactorB;
}

void vec3f_interpolate(Vec3f dest, Vec3f a, Vec3f b) {
    dest[0] = f32_interpolate(a[0], b[0]);
    dest[1] = f32_interpolate(a[1], b[1]);
    dest[2] = f32_interpolate(a[2], b[2]);
}

s16 s16_angle_interpolate(s16 a, s16 b) {
    u32 diff = abs(b - a);
    if(diff >= ANGLE_90_DEGREES)
        return b;
    return atan2s(coss(a) * interpolationFactorA + coss(b) * interpolationFactorB,
                  sins(a) * interpolationFactorA + sins(b) * interpolationFactorB);
}

void vec3s_angle_interpolate(Vec3s dest, Vec3s a, Vec3s b) {
    dest[0] = s16_angle_interpolate(a[0], b[0]);
    dest[1] = s16_angle_interpolate(a[1], b[1]);
    dest[2] = s16_angle_interpolate(a[2], b[2]);
}

void mtxf_interpolate(Mat4 dest, Mat4 a, Mat4 b) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            dest[i][j] = f32_interpolate(a[i][j], b[i][j]);
}
