//
// Created by temdisponivel on 24/05/2018.
//

#include "maths.h"
#include "mem.h"

INLINE void mat4_cpy(const mat4_t *mat, mat4_t *dest){
    dest->data[0].data[0] = mat->data[0].data[0];
    dest->data[0].data[1] = mat->data[0].data[1];
    dest->data[0].data[2] = mat->data[0].data[2];
    dest->data[0].data[3] = mat->data[0].data[3];

    dest->data[1].data[0] = mat->data[1].data[0];
    dest->data[1].data[1] = mat->data[1].data[1];
    dest->data[1].data[2] = mat->data[1].data[2];
    dest->data[1].data[3] = mat->data[1].data[3];

    dest->data[2].data[0] = mat->data[2].data[0];
    dest->data[2].data[1] = mat->data[2].data[1];
    dest->data[2].data[2] = mat->data[2].data[2];
    dest->data[2].data[3] = mat->data[2].data[3];

    dest->data[3].data[0] = mat->data[3].data[0];
    dest->data[3].data[1] = mat->data[3].data[1];
    dest->data[3].data[2] = mat->data[3].data[2];
    dest->data[3].data[3] = mat->data[3].data[3];
}

INLINE void mat4_mul(const mat4_t *mat, mat4_t *dest){
    float a00 = mat->x.x;
    float a01 = mat->x.y;
    float a02 = mat->x.z;
    float a03 = mat->x.w;
    float a10 = mat->y.x;
    float a11 = mat->y.y;
    float a12 = mat->y.z;
    float a13 = mat->y.w;
    float a20 = mat->z.x;
    float a21 = mat->z.y;
    float a22 = mat->z.z;
    float a23 = mat->z.w;
    float a30 = mat->w.x;
    float a31 = mat->w.y;
    float a32 = mat->w.z;
    float a33 = mat->w.w;
    float b00 = mat->x.x;
    float b01 = mat->x.y;
    float b02 = mat->x.z;
    float b03 = mat->x.w;
    float b10 = mat->y.x;
    float b11 = mat->y.y;
    float b12 = mat->y.z;
    float b13 = mat->y.w;
    float b20 = mat->z.x;
    float b21 = mat->z.y;
    float b22 = mat->z.z;
    float b23 = mat->z.w;
    float b30 = mat->w.x;
    float b31 = mat->w.y;
    float b32 = mat->w.z;
    float b33 = mat->w.w;

    dest->x.x = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
    dest->x.y = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
    dest->x.z = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
    dest->x.w = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;
    dest->y.x = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
    dest->y.y = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
    dest->y.z = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
    dest->y.w = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;
    dest->z.x = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
    dest->z.y = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
    dest->z.z = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
    dest->z.w = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;
    dest->w.x = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
    dest->w.y = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
    dest->w.z = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
    dest->w.w = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;
}

INLINE void mat4_to_quat(const mat4_t *mat, quat_t *dest) {
    float trace, r, rinv;

    /* it seems using like m12 instead of m[1][2] causes extra instructions */

    trace = mat->x.x + mat->y.y + mat->z.z;
    if (trace >= 0.0f) {
        r = sqrtf(1.0f + trace);
        rinv = 0.5f / r;

        dest->x = rinv * (mat->y.z - mat->z.y);
        dest->y = rinv * (mat->z.x - mat->x.z);
        dest->z = rinv * (mat->x.y - mat->y.x);
        dest->w = r * 0.5f;
    } else if (mat->x.x >= mat->y.y && mat->x.x >= mat->z.z) {
        r = sqrtf(1.0f - mat->y.y - mat->z.z + mat->x.x);
        rinv = 0.5f / r;

        dest->x = r * 0.5f;
        dest->y = rinv * (mat->x.y + mat->y.x);
        dest->z = rinv * (mat->x.z + mat->z.x);
        dest->w = rinv * (mat->y.z - mat->z.y);
    } else if (mat->y.y >= mat->z.z) {
        r = sqrtf(1.0f - mat->x.x - mat->z.z + mat->y.y);
        rinv = 0.5f / r;

        dest->x = rinv * (mat->x.y + mat->y.x);
        dest->y = r * 0.5f;
        dest->z = rinv * (mat->y.z + mat->z.y);
        dest->w = rinv * (mat->z.x - mat->x.z);
    } else {
        r = sqrtf(1.0f - mat->x.x - mat->y.y + mat->z.z);
        rinv = 0.5f / r;

        dest->x = rinv * (mat->x.z + mat->z.x);
        dest->y = rinv * (mat->y.z + mat->z.y);
        dest->z = r * 0.5f;
        dest->w = rinv * (mat->x.y - mat->y.x);
    }
}

INLINE void mat4_mul_vec4(const mat4_t *mat, const vec4_t *vec, vec4_t *dest){
    vec4_t res;
    
    res.x = mat->x.x * vec->x + mat->y.x * vec->y + mat->z.x * vec->z + mat->w.x * vec->w;
    res.y = mat->x.y * vec->x + mat->y.y * vec->y + mat->z.y * vec->z + mat->w.y * vec->w;
    res.z = mat->x.z * vec->x + mat->y.z * vec->y + mat->z.z * vec->z + mat->w.z * vec->w;
    res.w = mat->x.w * vec->x + mat->y.w * vec->y + mat->z.w * vec->z + mat->w.w * vec->w;

    vec4_cpy(&res, dest);
}

INLINE void mat4_mul_vec3(const mat4_t *mat, const vec3_t *vec, vec3_t *dest){
    vec3_t res;
    res.x = mat->x.x * vec->x + mat->y.x * vec->y + mat->z.x * vec->z;

    res.y = mat->x.y * vec->x + mat->y.y * vec->y + mat->z.y * vec->z;

    res.z = mat->x.z * vec->x + mat->y.z * vec->y + mat->z.z * vec->z;

    vec3_cpy(&res, dest);
}

INLINE void mat4_transpose(const mat4_t *mat, mat4_t *dest){
    dest->x.x = mat->x.x;
    dest->y.x = mat->x.y;
    dest->x.y = mat->y.x;
    dest->y.y = mat->y.y;
    dest->x.z = mat->z.x;
    dest->y.z = mat->z.y;
    dest->x.w = mat->w.x;
    dest->y.w = mat->w.y;
    dest->z.x = mat->x.z;
    dest->w.x = mat->x.w;
    dest->z.y = mat->y.z;
    dest->w.y = mat->y.w;
    dest->z.z = mat->z.z;
    dest->w.z = mat->z.w;
    dest->z.w = mat->w.z;
    dest->w.w = mat->w.w;
}

INLINE void mat4_scale(const mat4_t *mat, float scalar, mat4_t *dest){
    dest->x.x *= scalar;
    dest->x.y *= scalar;
    dest->x.z *= scalar;
    dest->x.w *= scalar;
    dest->y.x *= scalar;
    dest->y.y *= scalar;
    dest->y.z *= scalar;
    dest->y.w *= scalar;
    dest->z.x *= scalar;
    dest->z.y *= scalar;
    dest->z.z *= scalar;
    dest->z.w *= scalar;
    dest->w.x *= scalar;
    dest->w.y *= scalar;
    dest->w.z *= scalar;
    dest->w.w *= scalar;
}

INLINE float mat4_det(const mat4_t *mat){
    /* [square] det(A) = det(At) */
    float t[6];
    float a = mat->x.x;
    float b = mat->x.y;
    float c = mat->x.z;
    float d = mat->x.w;
    float e = mat->y.x;
    float f = mat->y.y;
    float g = mat->y.z;
    float h = mat->y.w;
    float i = mat->z.x;
    float j = mat->z.y;
    float k = mat->z.z;
    float l = mat->z.w;
    float m = mat->w.x;
    float n = mat->w.y;
    float o = mat->w.z;
    float p = mat->w.w;

    t[0] = k * p - o * l;
    t[1] = j * p - n * l;
    t[2] = j * o - n * k;
    t[3] = i * p - m * l;
    t[4] = i * o - m * k;
    t[5] = i * n - m * j;

    return a * (f * t[0] - g * t[1] + h * t[2])
           - b * (e * t[0] - g * t[3] + h * t[4])
           + c * (e * t[1] - f * t[3] + h * t[5])
           - d * (e * t[2] - f * t[4] + g * t[5]);
}

INLINE void mat4_inv(const mat4_t *mat, mat4_t *dest){
    float t[6];
    float det;
    float a = mat->x.x;
    float b = mat->x.y;
    float c = mat->x.z;
    float d = mat->x.w;
    float e = mat->y.x;
    float f = mat->y.y;
    float g = mat->y.z;
    float h = mat->y.w;
    float i = mat->z.x;
    float j = mat->z.y;
    float k = mat->z.z;
    float l = mat->z.w;
    float m = mat->w.x;
    float n = mat->w.y;
    float o = mat->w.z;
    float p = mat->w.w;

    t[0] = k * p - o * l;
    t[1] = j * p - n * l;
    t[2] = j * o - n * k;
    t[3] = i * p - m * l;
    t[4] = i * o - m * k;
    t[5] = i * n - m * j;

    dest->x.x = f * t[0] - g * t[1] + h * t[2];
    dest->y.x = -(e * t[0] - g * t[3] + h * t[4]);
    dest->z.x = e * t[1] - f * t[3] + h * t[5];
    dest->w.x = -(e * t[2] - f * t[4] + g * t[5]);

    dest->x.y = -(b * t[0] - c * t[1] + d * t[2]);
    dest->y.y = a * t[0] - c * t[3] + d * t[4];
    dest->z.y = -(a * t[1] - b * t[3] + d * t[5]);
    dest->w.y = a * t[2] - b * t[4] + c * t[5];

    t[0] = g * p - o * h;
    t[1] = f * p - n * h;
    t[2] = f * o - n * g;
    t[3] = e * p - m * h;
    t[4] = e * o - m * g;
    t[5] = e * n - m * f;

    dest->x.z = b * t[0] - c * t[1] + d * t[2];
    dest->y.z = -(a * t[0] - c * t[3] + d * t[4]);
    dest->z.z = a * t[1] - b * t[3] + d * t[5];
    dest->w.z = -(a * t[2] - b * t[4] + c * t[5]);

    t[0] = g * l - k * h;
    t[1] = f * l - j * h;
    t[2] = f * k - j * g;
    t[3] = e * l - i * h;
    t[4] = e * k - i * g;
    t[5] = e * j - i * f;

    dest->x.w = -(b * t[0] - c * t[1] + d * t[2]);
    dest->y.w = a * t[0] - c * t[3] + d * t[4];
    dest->z.w = -(a * t[1] - b * t[3] + d * t[5]);
    dest->w.w = a * t[2] - b * t[4] + c * t[5];

    det = 1.0f / (a *   dest->x.x + b * dest->y.x
                  + c * dest->z.x + d * dest->w.x);

    mat4_scale(dest, det, dest);
}

INLINE void mat4_frustum(float left, float right, float bottom, float top, float near, float far, mat4_t *dest){
    float rl, tb, fn, nv;
    
    memset(dest, 0, sizeof(mat4_t));

    rl = 1.0f / (right  - left);
    tb = 1.0f / (top    - bottom);
    fn =-1.0f / (far - near);
    nv = 2.0f * near;

    dest->x.x = nv * rl;
    dest->y.y = nv * tb;
    dest->z.x = (right  + left)    * rl;
    dest->z.y = (top    + bottom)  * tb;
    dest->z.z = (far + near) * fn;
    dest->z.w =-1.0f;
    dest->w.z = far * nv * fn;
}

INLINE void mat4_ortho(float left, float right, float bottom, float top, float near, float far, mat4_t *dest){
    float rl, tb, fn;
    
    memset(dest, 0, sizeof(mat4_t));
    
    rl = 1.0f / (right  - left);
    tb = 1.0f / (top    - bottom);
    fn =-1.0f / (far - near);

    dest->x.x = 2.0f * rl;
    dest->y.y = 2.0f * tb;
    dest->z.z = 2.0f * fn;
    dest->w.x =-(right  + left)    * rl;
    dest->w.y =-(top    + bottom)  * tb;
    dest->w.z = (far + near) * fn;
    dest->w.w = 1.0f;
}

INLINE void mat4_perspective(float field_of_view, float aspect, float near, float far, mat4_t *dest){
    float f, fn;

    memset(dest, 0, sizeof(mat4_t));

    f  = 1.0f / tanf(field_of_view * 0.5f);
    fn = 1.0f / (near - far);

    dest->x.x = f / aspect;
    dest->y.y = f;
    dest->z.z = (near + far) * fn;
    dest->z.w =-1.0f;
    dest->w.z = 2.0f * near * far * fn;
}

INLINE void mat4_look_at(const vec3_t *pos, const vec3_t *center, const vec3_t *up, mat4_t *dest){
    vec3_t f, u, s;

    vec3_sub(center, pos, &f);
    vec3_normalize(&f, &f);

    vec3_cross(&f, up, &s);
    vec3_normalize(&s, &s);

    vec3_cross(&s, &f, &u);

    dest->x.x = s.x;
    dest->x.y = u.x;
    dest->x.z = -f.x;
    dest->y.x = s.y;
    dest->y.y = u.y;
    dest->y.z = -f.y;
    dest->z.x = s.z;
    dest->z.y = u.z;
    dest->z.z = -f.z;   
    dest->w.x = -vec3_dot(&s, pos);
    dest->w.y = -vec3_dot(&u, pos);
    dest->w.z = vec3_dot(&f, pos);
    dest->x.w = dest->y.w = dest->z.w = 0.0f;
    dest->w.w = 1.0f;
}

INLINE void mat4_look(const vec3_t *pos, const vec3_t *dir, const vec3_t *up, mat4_t *dest){
    vec3_t target;
    vec3_add(pos, dir, &target);
    mat4_look_at(pos, &target, up, dest);
}

INLINE void mat4_decompose_perspective(const mat4_t *proj, float *near, float *far, float *top, float *bottom, float *left, float *right){
    float m00, m11, m20, m21, m22, m32, n, f;
    float n_m11, n_m00;

    m00 = proj->x.x;
    m11 = proj->y.y;
    m20 = proj->z.x;
    m21 = proj->z.y;
    m22 = proj->z.z;
    m32 = proj->w.z;

    n = m32 / (m22 - 1.0f);
    f = m32 / (m22 + 1.0f);

    n_m11 = n / m11;
    n_m00 = n / m00;

    *near = n;
    *far  = f;
    *bottom  = n_m11 * (m21 - 1.0f);
    *top     = n_m11 * (m21 + 1.0f);
    *left    = n_m00 * (m20 - 1.0f);
    *right   = n_m00 * (m20 + 1.0f);
}
