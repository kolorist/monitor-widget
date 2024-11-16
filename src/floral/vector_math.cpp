#include "vector_math.h"

#include "assert.h"
#include "misc.h"

///////////////////////////////////////////////////////////////////////////////

const vec2i vec2i::zero = vec2i(0, 0);

const vec4f vec4f::zero = vec4f(0.0f, 0.0f, 0.0f, 0.0f);
const vec4f vec4f::one = vec4f(1.0f, 1.0f, 1.0f, 1.0f);

const mat2x2f mat2x2f::zero = mat2x2f();
const mat2x2f mat2x2f::identity = mat2x2f(1.0f);

const mat4x4f mat4x4f::zero = mat4x4f();
const mat4x4f mat4x4f::identity = mat4x4f(1.0f);

///////////////////////////////////////////////////////////////////////////////

vec2f operator-(const vec2f i_v)
{
    return vec2f(-i_v.x, -i_v.y);
}

vec2f operator-(const vec2f& i_a, const vec2f& i_b)
{
    return vec2f(i_a.x - i_b.x, i_a.y - i_b.y);
}

vec2f operator+(const vec2f& i_a, const vec2f& i_b)
{
    return vec2f(i_a.x + i_b.x, i_a.y + i_b.y);
}

vec2f operator*(const vec2f& i_a, const f32 i_scalar)
{
    return vec2f(i_a.x * i_scalar, i_a.y * i_scalar);
}

vec2f operator/(const vec2f& i_a, const f32 i_scalar)
{
    return vec2f(i_a.x / i_scalar, i_a.y / i_scalar);
}

vec2f operator/(const vec2f& i_a, const vec2f& i_b)
{
    return vec2f(i_a.x / i_b.x, i_a.y / i_b.y);
}

vec2f operator*(const f32 i_scalar, const vec2f& i_a)
{
    return vec2f(i_a.x * i_scalar, i_a.y * i_scalar);
}

vec2f operator*(const vec2f& i_a, const vec2f& i_b)
{
    return vec2f(i_a.x * i_b.x, i_a.y * i_b.y);
}

vec2f& operator+=(vec2f& i_a, const vec2f& i_b)
{
    i_a.x += i_b.x;
    i_a.y += i_b.y;
    return i_a;
}

vec2f normalize(const vec2f& i_v)
{
    f32 len = length(i_v);
    return vec2f(i_v.x / len, i_v.y / len);
}

f32 length(const vec2f& i_v)
{
    return mathf_sqrt(i_v.x * i_v.x + i_v.y * i_v.y);
}

f32 sqr_length(const vec2f& i_v)
{
    return i_v.x * i_v.x + i_v.y * i_v.y;
}

vec2i operator+(const vec2i& i_a, const vec2i& i_b)
{
    return vec2i(i_a.x + i_b.x, i_a.y + i_b.y);
}

vec2i operator-(const vec2i& i_a, const vec2i& i_b)
{
    return vec2i(i_a.x - i_b.x, i_a.y - i_b.y);
}

vec2i operator*(const s32 i_scalar, const vec2i& i_a)
{
    return vec2i(i_a.x * i_scalar, i_a.y * i_scalar);
}

bool operator!=(const vec2i& i_a, const vec2i& i_b)
{
    return (i_a.x != i_b.x || i_a.y != i_b.y);
}

vec2i& operator+=(vec2i& i_a, const vec2i& i_b)
{
    i_a.x += i_b.x;
    i_a.y += i_b.y;
    return i_a;
}

vec2f operator*(const vec2i& i_a, const vec2f& i_b)
{
    return vec2f((f32)i_a.x * i_b.x, (f32)i_a.y * i_b.y);
}

bool equal(const vec2i& i_a, const vec2i& i_b)
{
    return (i_a.x == i_b.x && i_a.y == i_b.y);
}

vec3f operator-(const vec3f i_v)
{
    return vec3f(-i_v.x, -i_v.y, -i_v.z);
}

vec3f operator-(const vec3f& i_a, const vec3f& i_b)
{
    return vec3f(i_a.x - i_b.x, i_a.y - i_b.y, i_a.z - i_b.z);
}

vec3f operator+(const vec3f& i_a, const vec3f& i_b)
{
    return vec3f(i_a.x + i_b.x, i_a.y + i_b.y, i_a.z + i_b.z);
}

vec3f operator*(const vec3f& i_a, const f32 i_scalar)
{
    return vec3f(i_a.x * i_scalar, i_a.y * i_scalar, i_a.z * i_scalar);
}

vec3f operator*(const f32 i_scalar, const vec3f& i_a)
{
    return vec3f(i_a.x * i_scalar, i_a.y * i_scalar, i_a.z * i_scalar);
}

vec3f operator*(const vec3f& i_a, const vec3f& i_b)
{
    return vec3f(i_a.x * i_b.x, i_a.y * i_b.y, i_a.z * i_b.z);
}

vec3f operator/(const vec3f& i_a, const f32 i_scalar)
{
    return vec3f(i_a.x / i_scalar, i_a.y / i_scalar, i_a.z / i_scalar);
}

vec3f operator/(const vec3f& i_a, const vec3f& i_b)
{
    return vec3f(i_a.x / i_b.x, i_a.y / i_b.y, i_a.z / i_b.z);
}

vec3f operator+=(vec3f& i_a, const vec3f& i_b)
{
    i_a.x += i_b.x;
    i_a.y += i_b.y;
    i_a.z += i_b.z;
    return i_a;
}

vec3f normalize(const vec3f& i_v)
{
    f32 len = length(i_v);
    return vec3f(i_v.x / len, i_v.y / len, i_v.z / len);
}

f32 length(const vec3f& i_v)
{
    return mathf_sqrt(i_v.x * i_v.x + i_v.y * i_v.y + i_v.z * i_v.z);
}

f32 dot(const vec3f& i_a, const vec3f& i_b)
{
    return (i_a.x * i_b.x + i_a.y * i_b.y + i_a.z * i_b.z);
}

vec3f cross(const vec3f& i_a, const vec3f& i_b)
{
    return vec3f(
        i_a.y * i_b.z - i_a.z * i_b.y,
        i_a.z * i_b.x - i_a.x * i_b.z,
        i_a.x * i_b.y - i_a.y * i_b.x);
}

bool equal(const vec3f& i_a, const vec3f& i_b, const f32 i_epsilon)
{
    f32 disp = length(i_a - i_b);
    return (disp < i_epsilon);
}

vec4f operator*=(vec4f& i_a, const f32 i_scalar)
{
    i_a.x *= i_scalar;
    i_a.y *= i_scalar;
    i_a.z *= i_scalar;
    i_a.w *= i_scalar;
    return i_a;
}

f32 length(const vec4f& i_v)
{
    return mathf_sqrt(i_v.x * i_v.x + i_v.y * i_v.y + i_v.z * i_v.z + i_v.w * i_v.w);
}

vec4f normalize(const vec4f& i_v)
{
    f32 len = length(i_v);
    return vec4f(i_v.x / len, i_v.y / len, i_v.z / len, i_v.w / len);
}

///////////////////////////////////////////////////////////////////////////////

mat4x4f operator*(const mat4x4f& i_a, const mat4x4f& i_b)
{
    mat4x4f tMat;
    tMat.m[0][0] = i_a.m[0][0] * i_b.m[0][0] + i_a.m[1][0] * i_b.m[0][1] + i_a.m[2][0] * i_b.m[0][2] + i_a.m[3][0] * i_b.m[0][3];
    tMat.m[0][1] = i_a.m[0][1] * i_b.m[0][0] + i_a.m[1][1] * i_b.m[0][1] + i_a.m[2][1] * i_b.m[0][2] + i_a.m[3][1] * i_b.m[0][3];
    tMat.m[0][2] = i_a.m[0][2] * i_b.m[0][0] + i_a.m[1][2] * i_b.m[0][1] + i_a.m[2][2] * i_b.m[0][2] + i_a.m[3][2] * i_b.m[0][3];
    tMat.m[0][3] = i_a.m[0][3] * i_b.m[0][0] + i_a.m[1][3] * i_b.m[0][1] + i_a.m[2][3] * i_b.m[0][2] + i_a.m[3][3] * i_b.m[0][3];

    tMat.m[1][0] = i_a.m[0][0] * i_b.m[1][0] + i_a.m[1][0] * i_b.m[1][1] + i_a.m[2][0] * i_b.m[1][2] + i_a.m[3][0] * i_b.m[1][3];
    tMat.m[1][1] = i_a.m[0][1] * i_b.m[1][0] + i_a.m[1][1] * i_b.m[1][1] + i_a.m[2][1] * i_b.m[1][2] + i_a.m[3][1] * i_b.m[1][3];
    tMat.m[1][2] = i_a.m[0][2] * i_b.m[1][0] + i_a.m[1][2] * i_b.m[1][1] + i_a.m[2][2] * i_b.m[1][2] + i_a.m[3][2] * i_b.m[1][3];
    tMat.m[1][3] = i_a.m[0][3] * i_b.m[1][0] + i_a.m[1][3] * i_b.m[1][1] + i_a.m[2][3] * i_b.m[1][2] + i_a.m[3][3] * i_b.m[1][3];

    tMat.m[2][0] = i_a.m[0][0] * i_b.m[2][0] + i_a.m[1][0] * i_b.m[2][1] + i_a.m[2][0] * i_b.m[2][2] + i_a.m[3][0] * i_b.m[2][3];
    tMat.m[2][1] = i_a.m[0][1] * i_b.m[2][0] + i_a.m[1][1] * i_b.m[2][1] + i_a.m[2][1] * i_b.m[2][2] + i_a.m[3][1] * i_b.m[2][3];
    tMat.m[2][2] = i_a.m[0][2] * i_b.m[2][0] + i_a.m[1][2] * i_b.m[2][1] + i_a.m[2][2] * i_b.m[2][2] + i_a.m[3][2] * i_b.m[2][3];
    tMat.m[2][3] = i_a.m[0][3] * i_b.m[2][0] + i_a.m[1][3] * i_b.m[2][1] + i_a.m[2][3] * i_b.m[2][2] + i_a.m[3][3] * i_b.m[2][3];

    tMat.m[3][0] = i_a.m[0][0] * i_b.m[3][0] + i_a.m[1][0] * i_b.m[3][1] + i_a.m[2][0] * i_b.m[3][2] + i_a.m[3][0] * i_b.m[3][3];
    tMat.m[3][1] = i_a.m[0][1] * i_b.m[3][0] + i_a.m[1][1] * i_b.m[3][1] + i_a.m[2][1] * i_b.m[3][2] + i_a.m[3][1] * i_b.m[3][3];
    tMat.m[3][2] = i_a.m[0][2] * i_b.m[3][0] + i_a.m[1][2] * i_b.m[3][1] + i_a.m[2][2] * i_b.m[3][2] + i_a.m[3][2] * i_b.m[3][3];
    tMat.m[3][3] = i_a.m[0][3] * i_b.m[3][0] + i_a.m[1][3] * i_b.m[3][1] + i_a.m[2][3] * i_b.m[3][2] + i_a.m[3][3] * i_b.m[3][3];

    return tMat;
}

vec4f operator*(const mat4x4f& i_m, const vec4f& i_v)
{
    f32 x = i_m.m[0][0] * i_v.x + i_m.m[1][0] * i_v.y + i_m.m[2][0] * i_v.z + i_m.m[3][0] * i_v.w;
    f32 y = i_m.m[0][1] * i_v.x + i_m.m[1][1] * i_v.y + i_m.m[2][1] * i_v.z + i_m.m[3][1] * i_v.w;
    f32 z = i_m.m[0][2] * i_v.x + i_m.m[1][2] * i_v.y + i_m.m[2][2] * i_v.z + i_m.m[3][2] * i_v.w;
    f32 w = i_m.m[0][3] * i_v.x + i_m.m[1][3] * i_v.y + i_m.m[2][3] * i_v.z + i_m.m[3][3] * i_v.w;
    return vec4f(x, y, z, w);
}

mat4x4f construct_lookat_dir_rh(const vec3f& i_upDir, const vec3f& i_camPos, const vec3f& i_lookAtDir)
{
    mat4x4f m(1.0f);
    vec3f front = -normalize(i_lookAtDir);          // points out of the screen
    vec3f right = normalize(cross(i_upDir, front)); // points rightward
    vec3f up = cross(front, right);                 // points upward

    m.m[0][0] = right.x;
    m.m[1][0] = right.y;
    m.m[2][0] = right.z;

    m.m[0][1] = up.x;
    m.m[1][1] = up.y;
    m.m[2][1] = up.z;

    m.m[0][2] = front.x;
    m.m[1][2] = front.y;
    m.m[2][2] = front.z;

    m.m[3][0] = -dot(right, i_camPos);
    m.m[3][1] = -dot(up, i_camPos);
    m.m[3][2] = -dot(front, i_camPos);

    return m;
}

mat4x4f construct_lookat_dir_lh(const vec3f& i_upDir, const vec3f& i_camPos, const vec3f& i_lookAtDir)
{
    mat4x4f m(1.0f);
    vec3f front = normalize(i_lookAtDir);
    vec3f right = normalize(cross(i_upDir, front));
    vec3f up = cross(front, right);

    m.m[0][0] = right.x;
    m.m[1][0] = right.y;
    m.m[2][0] = right.z;

    m.m[0][1] = up.x;
    m.m[1][1] = up.y;
    m.m[2][1] = up.z;

    m.m[0][2] = front.x;
    m.m[1][2] = front.y;
    m.m[2][2] = front.z;

    m.m[3][0] = -dot(right, i_camPos);
    m.m[3][1] = -dot(up, i_camPos);
    m.m[3][2] = -dot(front, i_camPos);

    return m;
}

mat4x4f construct_lookat_point_rh(const vec3f& i_upDir, const vec3f& i_camPos, const vec3f& lookAtPoint)
{
    vec3f lookAtDir = lookAtPoint - i_camPos;
    return construct_lookat_dir_rh(i_upDir, i_camPos, lookAtDir);
}

mat4x4f construct_lookat_point_lh(const vec3f& i_upDir, const vec3f& i_camPos, const vec3f& lookAtPoint)
{
    vec3f lookAtDir = lookAtPoint - i_camPos;
    return construct_lookat_dir_lh(i_upDir, i_camPos, lookAtDir);
}

mat4x4f construct_lookat_dir_rh(const camera_view_t* i_desc)
{
    return construct_lookat_dir_rh(i_desc->up_direction, i_desc->position, i_desc->look_at);
}

mat4x4f construct_lookat_dir_lh(const camera_view_t* i_desc)
{
    return construct_lookat_dir_lh(i_desc->up_direction, i_desc->position, i_desc->look_at);
}

mat4x4f construct_lookat_point_rh(const camera_view_t* i_desc)
{
    return construct_lookat_point_rh(i_desc->up_direction, i_desc->position, i_desc->look_at);
}

mat4x4f construct_lookat_point_lh(const camera_view_t* i_desc)
{
    return construct_lookat_point_lh(i_desc->up_direction, i_desc->position, i_desc->look_at);
}

mat4x4f construct_orthographic_rh(const f32 i_left, const f32 i_right, const f32 i_top, const f32 i_bottom, const f32 i_near, const f32 i_far)
{
    mat4x4f m(1.0f);
    m.m[0][0] = 2.0f / (i_right - i_left);
    m.m[1][1] = 2.0f / (i_top - i_bottom);
    m.m[2][2] = -2.0f / (i_far - i_near);
    m.m[3][0] = -(i_right + i_left) / (i_right - i_left);
    m.m[3][1] = -(i_top + i_bottom) / (i_top - i_bottom);
    m.m[3][2] = -(i_far + i_near) / (i_far - i_near);
    return m;
}

mat4x4f construct_orthographic_lh(const f32 i_left, const f32 i_right, const f32 i_top, const f32 i_bottom, const f32 i_near, const f32 i_far)
{
    mat4x4f m(1.0f);
    m.m[0][0] = 2.0f / (i_right - i_left);
    m.m[1][1] = 2.0f / (i_top - i_bottom);
    m.m[2][2] = 2.0f / (i_far - i_near);
    m.m[3][0] = -(i_right + i_left) / (i_right - i_left);
    m.m[3][1] = -(i_top + i_bottom) / (i_top - i_bottom);
    m.m[3][2] = -(i_far + i_near) / (i_far - i_near);
    return m;
}

mat4x4f construct_orthographic_rh(const camera_ortho_t& i_desc)
{
    return construct_orthographic_rh(i_desc.left, i_desc.right, i_desc.top, i_desc.bottom, i_desc.near_plane, i_desc.far_plane);
}

mat4x4f construct_orthographic_lh(const camera_ortho_t& i_desc)
{
    return construct_orthographic_lh(i_desc.left, i_desc.right, i_desc.top, i_desc.bottom, i_desc.near_plane, i_desc.far_plane);
}

mat4x4f construct_perspective_rh(const f32 i_near, const f32 i_far, const f32 i_fovy, const f32 i_aspectRatio)
{
    const f32 tanHalfFovY = mathf_tan(to_radians(i_fovy / 2.0f));

    mat4x4f m;
    m.m[0][0] = 1.0f / (i_aspectRatio * tanHalfFovY);
    m.m[1][1] = 1.0f / tanHalfFovY;
    m.m[2][2] = -(i_far + i_near) / (i_far - i_near);
    m.m[2][3] = -1.0f;
    m.m[3][2] = -2.0f * i_far * i_near / (i_far - i_near);

    return m;
}

mat4x4f construct_infinity_perspective_rh(const f32 i_near, const f32 i_fovy, const f32 i_aspectRatio)
{
    const f32 tanHalfFovY = mathf_tan(to_radians(i_fovy / 2.0f));

    mat4x4f m;
    m.m[0][0] = 1.0f / (i_aspectRatio * tanHalfFovY);
    m.m[1][1] = 1.0f / tanHalfFovY;
    m.m[2][2] = -1;
    m.m[2][3] = -1.0f;
    m.m[3][2] = -2.0f * i_near;

    return m;
}

mat4x4f construct_perspective_lh(const f32 i_near, const f32 i_far, const f32 i_fovy, const f32 i_aspectRatio)
{
    const f32 tanHalfFovY = mathf_tan(to_radians(i_fovy / 2.0f));

    mat4x4f m;
    m.m[0][0] = 1.0f / (i_aspectRatio * tanHalfFovY);
    m.m[1][1] = 1.0f / tanHalfFovY;
    m.m[2][2] = (i_far + i_near) / (i_far - i_near);
    m.m[2][3] = 1.0f;
    m.m[3][2] = -2.0f * i_far * i_near / (i_far - i_near);

    return m;
}

mat4x4f construct_infinity_perspective_lh(const f32 i_near, const f32 i_fovy, const f32 i_aspectRatio)
{
    const f32 tanHalfFovY = mathf_tan(to_radians(i_fovy / 2.0f));

    mat4x4f m;
    m.m[0][0] = 1.0f / (i_aspectRatio * tanHalfFovY);
    m.m[1][1] = 1.0f / tanHalfFovY;
    m.m[2][2] = 1.0f;
    m.m[2][3] = 1.0f;
    m.m[3][2] = -2.0f * i_near;

    return m;
}

mat4x4f construct_perspective_rh(const camera_persp_t* i_desc)
{
    return construct_perspective_rh(i_desc->near_plane, i_desc->far_plane, i_desc->fov, i_desc->aspect_ratio);
}

mat4x4f construct_perspective_lh(const camera_persp_t* i_desc)
{
    return construct_perspective_lh(i_desc->near_plane, i_desc->far_plane, i_desc->fov, i_desc->aspect_ratio);
}

// way of matrix - vector multiplication
// opengl:
// 	a b c d			x			ax + by + cz + dw
// 	e f g h	  mul	y	equal	ex + fy + gz + hw
// 	i j k l			z			ix + jy + kz + lw
// 	m n o p			w			mx + ny + oz + pw
//
// cpu:
// 	a e i m
// 	b f j n
// 	c g k o
// 	d h l p

mat4x4f construct_translation3d(const f32 i_deltaX, const f32 i_deltaY, const f32 i_deltaZ)
{
    mat4x4f tMat = mat4x4f(1.0f);
    tMat.m[3][0] = i_deltaX;
    tMat.m[3][1] = i_deltaY;
    tMat.m[3][2] = i_deltaZ;
    return tMat;
}

mat4x4f construct_translation3d(const vec3f& i_delta)
{
    mat4x4f tMat = mat4x4f(1.0f);
    tMat.m[3][0] = i_delta.x;
    tMat.m[3][1] = i_delta.y;
    tMat.m[3][2] = i_delta.z;
    return tMat;
}

mat4x4f construct_scaling3d(const f32 i_scaleX, const f32 i_scaleY, const f32 i_scaleZ)
{
    mat4x4f tMat;
    tMat.m[0][0] = i_scaleX;
    tMat.m[1][1] = i_scaleY;
    tMat.m[2][2] = i_scaleZ;
    tMat.m[3][3] = 1.0f;
    return tMat;
}

mat4x4f construct_scaling3d(const vec3f& i_scale)
{
    mat4x4f tMat;
    tMat.m[0][0] = i_scale.x;
    tMat.m[1][1] = i_scale.y;
    tMat.m[2][2] = i_scale.z;
    tMat.m[3][3] = 1.0f;
    return tMat;
}

mat4x4f construct_x_rotation3d(f32 i_rXRad)
{
    mat4x4f tMat;
    tMat.m[0][0] = 1.0f;
    tMat.m[1][1] = mathf_cos(i_rXRad);
    tMat.m[1][2] = -mathf_sin(i_rXRad);
    tMat.m[2][1] = mathf_sin(i_rXRad);
    tMat.m[2][2] = mathf_cos(i_rXRad);
    tMat.m[3][3] = 1.0f;
    return tMat;
}

mat4x4f construct_y_rotation3d(f32 i_rYRad)
{
    mat4x4f tMat;
    tMat.m[0][0] = mathf_cos(i_rYRad);
    tMat.m[0][2] = mathf_sin(i_rYRad);
    tMat.m[1][1] = 1.0f;
    tMat.m[2][0] = -mathf_sin(i_rYRad);
    tMat.m[2][2] = mathf_cos(i_rYRad);
    tMat.m[3][3] = 1.0f;
    return tMat;
}

mat4x4f construct_z_rotation3d(f32 i_rZRad)
{
    mat4x4f tMat;
    tMat.m[0][0] = mathf_cos(i_rZRad);
    tMat.m[0][1] = -mathf_sin(i_rZRad);
    tMat.m[1][0] = mathf_sin(i_rZRad);
    tMat.m[1][1] = mathf_cos(i_rZRad);
    tMat.m[2][2] = 1.0f;
    tMat.m[3][3] = 1.0f;
    return tMat;
}

mat4x4f construct_axis_rotation3d(const vec3f& i_axis, const f32 i_angle)
{
    mat4x4f tMat;
    f32 c = mathf_cos(i_angle);
    f32 s = mathf_sin(i_angle);

    vec3f ax = normalize(i_axis);

    tMat.m[0][0] = c + (1 - c) * ax.x * ax.x;
    tMat.m[0][1] = (1 - c) * ax.x * ax.y + s * ax.z;
    tMat.m[0][2] = (1 - c) * ax.x * ax.z - s * ax.y;
    tMat.m[0][3] = 0;

    tMat.m[1][0] = (1 - c) * ax.y * ax.x - s * ax.z;
    tMat.m[1][1] = c + (1 - c) * ax.y * ax.y;
    tMat.m[1][2] = (1 - c) * ax.y * ax.z + s * ax.x;
    tMat.m[1][3] = 0;

    tMat.m[2][0] = (1 - c) * ax.z * ax.x + s * ax.y;
    tMat.m[2][1] = (1 - c) * ax.z * ax.y - s * ax.x;
    tMat.m[2][2] = c + (1 - c) * ax.z * ax.z;
    tMat.m[2][3] = 0;

    tMat.m[3][3] = 1.0f;
    return tMat;
}

mat4x4f construct_transpose(const mat4x4f& i_m)
{
    mat4x4f tMat;
    tMat.m[0][0] = i_m.m[0][0];
    tMat.m[0][1] = i_m.m[1][0];
    tMat.m[0][2] = i_m.m[2][0];
    tMat.m[0][3] = i_m.m[3][0];
    tMat.m[1][0] = i_m.m[0][1];
    tMat.m[1][1] = i_m.m[1][1];
    tMat.m[1][2] = i_m.m[2][1];
    tMat.m[1][3] = i_m.m[3][1];
    tMat.m[2][0] = i_m.m[0][2];
    tMat.m[2][1] = i_m.m[1][2];
    tMat.m[2][2] = i_m.m[2][2];
    tMat.m[2][3] = i_m.m[3][2];
    tMat.m[3][0] = i_m.m[0][3];
    tMat.m[3][1] = i_m.m[1][3];
    tMat.m[3][2] = i_m.m[2][3];
    tMat.m[3][3] = i_m.m[3][3];
    return tMat;
}

// mat3x3f construct_transpose(const mat3x3f& i_m)
//{
//	mat3x3f tMat;
//	tMat.m[0][0] = i_m[0][0]; tMat.m[0][1] = i_m[1][0]; tMat.m[0][2] = i_m[2][0];
//	tMat.m[1][0] = i_m[0][1]; tMat.m[1][1] = i_m[1][1]; tMat.m[1][2] = i_m[2][1];
//	tMat.m[2][0] = i_m[0][2]; tMat.m[2][1] = i_m[1][2]; tMat.m[2][2] = i_m[2][2];
//	return tMat;
// }

vec3f apply_vector_transform(const vec3f& i_v, const mat4x4f& i_m)
{
    vec4f tmp(i_v.x, i_v.y, i_v.z, 0.0f);
    tmp = i_m * tmp;
    return vec3f(tmp.x, tmp.y, tmp.z);
}

vec3f apply_point_transform(const vec3f& i_p, const mat4x4f& i_m)
{
    vec4f tmp(i_p.x, i_p.y, i_p.z, 1.0f);
    tmp = i_m * tmp;
    return vec3f(tmp.x, tmp.y, tmp.z);
}

quaternionf operator*(const quaternionf& q1, const quaternionf& q2)
{
    quaternionf retQuat;
    retQuat.v = cross(q1.v, q2.v) + q1.s * q2.v + q2.s * q1.v;
    retQuat.s = q1.s * q2.s - dot(q1.v, q2.v);
    return retQuat;
}

quaternionf construct_quaternion_euler(const f32 i_rx, const f32 i_ry, const f32 i_rz)
{
    // TODO: optimize this!!!
    quaternionf qX, qY, qZ;
    f32 halfThetaX = to_radians(i_rx) / 2.0f;
    f32 halfThetaY = to_radians(i_ry) / 2.0f;
    f32 halfThetaZ = to_radians(i_rz) / 2.0f;
    qX.v = vec3f(-mathf_sin(halfThetaX), 0.0f, 0.0f);
    qX.s = mathf_cos(halfThetaX);
    qY.v = vec3f(0.0f, -mathf_sin(halfThetaY), 0.0f);
    qY.s = mathf_cos(halfThetaY);
    qZ.v = vec3f(0.0f, 0.0f, -mathf_sin(halfThetaZ));
    qZ.s = mathf_cos(halfThetaZ);
    return qX * qY * qZ;
}

quaternionf construct_quaternion_euler(const vec3f& i_r)
{
    return construct_quaternion_euler(i_r.x, i_r.y, i_r.z);
}

quaternionf construct_quaternion_axis(const vec3f& i_axis, const f32 i_r)
{
    quaternionf q;
    // vec3f n = normalize(i_axis);
    f32 halfTheta = to_radians(i_r) / 2.0f;
    q.v = -i_axis * mathf_sin(halfTheta);
    q.s = mathf_cos(halfTheta);
    return q;
}

quaternionf construct_quaternion_axis_rad(const vec3f& i_axis, const f32 i_r)
{
    quaternionf q;
    // vec3f n = normalize(i_axis);
    f32 halfTheta = i_r / 2.0f;
    q.v = -i_axis * mathf_sin(halfTheta);
    q.s = mathf_cos(halfTheta);
    return q;
}

quaternionf construct_quaternion_v2v(const vec3f& i_v0, const vec3f& i_v1)
{
    static constexpr f32 k_epsilon = 0.001f;
    if (equal(i_v0, i_v1, k_epsilon))
    {
        return construct_quaternion_axis_rad(i_v0, 0.0f);
    }
    else if (equal(i_v0, -i_v1, k_epsilon))
    {
        vec3f v(0.0f);
        if (mathf_abs(i_v0.x) < k_epsilon)
        {
            v.x = 1.0f;
        }
        else if (mathf_abs(i_v0.y) < k_epsilon)
        {
            v.y = 1.0f;
        }
        else
        {
            v.z = 1.0f;
        }
        return construct_quaternion_axis_rad(v, MATH_PI);
    }

    vec3f u0 = normalize(i_v0);
    vec3f u1 = normalize(i_v1);
    vec3f v = cross(u0, u1);
    f32 angleRad = mathf_acos(dot(u0, u1));
    return construct_quaternion_axis_rad(v, angleRad);
}

// transfomation
mat4x4f to_transform(const quaternionf& i_q)
{
    mat4x4f m;
    m.c[0] = vec4f(1.0f - 2.0f * (i_q.y * i_q.y + i_q.z * i_q.z), 2.0f * (i_q.x * i_q.y - i_q.z * i_q.w), 2.0f * (i_q.x * i_q.z + i_q.y * i_q.w), 0.0f);
    m.c[1] = vec4f(2.0f * (i_q.x * i_q.y + i_q.z * i_q.w), 1.0f - 2.0f * (i_q.x * i_q.x + i_q.z * i_q.z), 2.0f * (i_q.y * i_q.z - i_q.x * i_q.w), 0.0f);
    m.c[2] = vec4f(2.0f * (i_q.x * i_q.z - i_q.y * i_q.w), 2.0f * (i_q.y * i_q.z + i_q.x * i_q.w), 1.0f - 2.0f * (i_q.x * i_q.x + i_q.y * i_q.y), 0.0f);
    m.c[3] = vec4f(0.0f, 0.0f, 0.0f, 1.0f);
    // TODO: wtf??? no transpose please!!!
    m = construct_transpose(m);
    return m;
}
// ----------------------------------------------------------------------------

vec3f transform_hsv(const vec3f& i_input, const f32 i_hue, const f32 i_saturation, const f32 i_value)
{
    f32 vsu = i_value * i_saturation * mathf_cos(i_hue * MATH_PI / 180);
    f32 vsw = i_value * i_saturation * mathf_sin(i_hue * MATH_PI / 180);

    vec3f ret;
    ret.r = (.299f * i_value + .701f * vsu + .168f * vsw) * i_input.r +
            (.587f * i_value - .587f * vsu + .330f * vsw) * i_input.g +
            (.114f * i_value - .114f * vsu - .497f * vsw) * i_input.b;
    ret.g = (.299f * i_value - .299f * vsu - .328f * vsw) * i_input.r +
            (.587f * i_value + .413f * vsu + .035f * vsw) * i_input.g +
            (.114f * i_value - .114f * vsu + .292f * vsw) * i_input.b;
    ret.b = (.299f * i_value - .300f * vsu + 1.25f * vsw) * i_input.r +
            (.587f * i_value - .588f * vsu - 1.05f * vsw) * i_input.g +
            (.114f * i_value + .886f * vsu - .203f * vsw) * i_input.b;
    return ret;
}

vec3f hsv_to_rgb(const vec3f& i_input)
{
    f32 hh, p, q, t, ff; // NOLINT(cppcoreguidelines-init-variables)
    u32 i;               // NOLINT(cppcoreguidelines-init-variables)
    vec3f out;

    if (i_input.s <= 0.0f)
    {
        out.r = i_input.v;
        out.g = i_input.v;
        out.b = i_input.v;
        return out;
    }
    hh = i_input.h;
    if (hh >= 360.0f)
    {
        hh = 0.0f;
    }
    hh /= 60.0f;
    i = (u32)hh;
    ff = hh - (f32)i;
    p = i_input.v * (1.0f - i_input.s);
    q = i_input.v * (1.0f - (i_input.s * ff));
    t = i_input.v * (1.0f - (i_input.s * (1.0f - ff)));

    switch (i)
    {
    case 0:
        out.r = i_input.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = i_input.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = i_input.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = i_input.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = i_input.v;
        break;
    case 5:
    default:
        out.r = i_input.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

vec3f rgb_to_hsv(const vec3f& i_input)
{
    vec3f out;
    f32 min, max, delta; // NOLINT(cppcoreguidelines-init-variables)

    min = i_input.r < i_input.g ? i_input.r : i_input.g;
    min = min < i_input.b ? min : i_input.b;

    max = i_input.r > i_input.g ? i_input.r : i_input.g;
    max = max > i_input.b ? max : i_input.b;

    out.v = max; // v
    delta = max - min;
    if (delta < 0.00001f)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if (max > 0.0f)
    {                          // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max); // s
    }
    else
    {
        // if max is 0, then r = g = b = 0
        // s = 0, h is undefined
        out.s = 0.0f;
        out.h = 0.0f; // its now undefined
        FLORAL_ASSERT_MSG(false, "Undefined hue value");
        return out;
    }
    if (i_input.r >= max) // > is bogus, just keeps compilor happy
    {
        out.h = (i_input.g - i_input.b) / delta; // between yellow & magenta
    }
    else if (i_input.g >= max)
    {
        out.h = 2.0f + (i_input.b - i_input.r) / delta; // between cyan & yellow
    }
    else
    {
        out.h = 4.0f + (i_input.r - i_input.g) / delta; // between magenta & cyan
    }
    out.h *= 60.0f; // degrees

    if (out.h < 0.0f)
    {
        out.h += 360.0f;
    }

    return out;
}
