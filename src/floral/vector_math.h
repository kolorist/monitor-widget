#pragma once

#include "stdaliases.h"

///////////////////////////////////////////////////////////////////////////////
// vector

struct vec2f // NOLINT(readability-identifier-naming)
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
        };

        f32 elems[2];
    };

    vec2f()
        : x(0),
          y(0)
    {
    }

    vec2f(const f32 i_x, const f32 i_y)
        : x(i_x),
          y(i_y)
    {
    }
};

struct vec2i // NOLINT(readability-identifier-naming)
{
    static const vec2i zero; // NOLINT(readability-identifier-naming)
    union
    {
        struct
        {
            s32 x;
            s32 y;
        };

        s32 elems[2];
    };

    vec2i()
        : x(0),
          y(0)
    {
    }

    vec2i(const s32 i_x, const s32 i_y)
        : x(i_x),
          y(i_y)
    {
    }
};

struct vec3f // NOLINT(readability-identifier-naming)
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
        };

        struct
        {
            f32 h;
            f32 s;
            f32 v;
        };

        f32 elems[3];
    };

    vec3f()
        : x(0),
          y(0),
          z(0)
    {
    }

    vec3f(const vec2f& i_value)
        : x(i_value.x),
          y(i_value.y),
          z(0)
    {
    }

    vec3f(const vec2f& i_value, f32 i_z)
        : x(i_value.x),
          y(i_value.y),
          z(i_z)
    {
    }

    vec3f(f32 i_value)
        : x(i_value),
          y(i_value),
          z(i_value)
    {
    }

    vec3f(f32 i_x, f32 i_y, f32 i_z)
        : x(i_x),
          y(i_y),
          z(i_z)
    {
    }
};

struct vec4f // NOLINT(readability-identifier-naming)
{
    static const vec4f zero; // NOLINT(readability-identifier-naming)
    static const vec4f one;  // NOLINT(readability-identifier-naming)

    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };

        f32 elems[4];
    };

    vec4f()
        : x(0),
          y(0),
          z(0),
          w(0)
    {
    }

    vec4f(f32 i_x, f32 i_y, f32 i_z, f32 i_w)
        : x(i_x),
          y(i_y),
          z(i_z),
          w(i_w)
    {
    }

    vec4f(const vec3f& v, const f32 i_w)
        : x(v.x),
          y(v.y),
          z(v.z),
          w(i_w)
    {
    }
};

vec2f operator-(const vec2f i_v);
vec2f operator-(const vec2f& i_a, const vec2f& i_b);
vec2f operator+(const vec2f& i_a, const vec2f& i_b);
vec2f operator*(const vec2f& i_a, const f32 i_scalar);
vec2f operator*(const f32 i_scalar, const vec2f& i_a);
vec2f operator*(const vec2f& i_a, const vec2f& i_b);
vec2f operator/(const vec2f& i_a, const f32 i_scalar);
vec2f operator/(const vec2f& i_a, const vec2f& i_b);
vec2f& operator+=(vec2f& i_a, const vec2f& i_b);
vec2f normalize(const vec2f& i_v);
f32 length(const vec2f& i_v);
f32 sqr_length(const vec2f& i_v);

vec2i operator+(const vec2i& i_a, const vec2i& i_b);
vec2i operator-(const vec2i& i_a, const vec2i& i_b);
vec2i operator*(const s32 i_scalar, const vec2i& i_a);
bool operator!=(const vec2i& i_a, const vec2i& i_b);

vec2i& operator+=(vec2i& i_a, const vec2i& i_b);
vec2f operator*(const vec2i& i_a, const vec2f& i_b);

vec3f operator-(const vec3f i_v);
vec3f operator-(const vec3f& i_a, const vec3f& i_b);
vec3f operator+(const vec3f& i_a, const vec3f& i_b);
vec3f operator*(const vec3f& i_a, const f32 i_scalar);
vec3f operator*(const f32 i_scalar, const vec3f& i_a);
vec3f operator*(const vec3f& i_a, const vec3f& i_b);
vec3f operator/(const vec3f& i_a, const f32 i_scalar);
vec3f operator/(const vec3f& i_a, const vec3f& i_b);
vec3f operator+=(vec3f& i_a, const vec3f& i_b);
vec3f normalize(const vec3f& i_v);
f32 dot(const vec3f& i_a, const vec3f& i_b);
vec3f cross(const vec3f& i_a, const vec3f& i_b);
bool equal(const vec3f& i_a, const vec3f& i_b, const f32 i_epsilon);
f32 length(const vec3f& i_v);

vec4f operator*=(vec4f& i_a, const f32 i_scalar);
f32 length(const vec4f& i_v);
vec4f normalize(const vec4f& i_v);

///////////////////////////////////////////////////////////////////////////////
// column-major
struct mat2x2f // NOLINT(readability-identifier-naming)
{
    static const mat2x2f zero;     // NOLINT(readability-identifier-naming)
    static const mat2x2f identity; // NOLINT(readability-identifier-naming)

    union
    {
        vec2f c[2];
        f32 m[2][2]; // m[column][row]
        f32 elems[4];
    };

    mat2x2f()
        : elems{ 0.0f, 0.0f, 0.0f, 0.0f }
    {
    }

    mat2x2f(f32 i_value)
        : mat2x2f()
    {
        m[0][0] = i_value;
        m[1][1] = i_value;
    }
};

// column-major
struct mat4x4f // NOLINT(readability-identifier-naming)
{
    static const mat4x4f zero;     // NOLINT(readability-identifier-naming)
    static const mat4x4f identity; // NOLINT(readability-identifier-naming)

    union
    {
        vec4f c[4];
        f32 m[4][4]; // m[column][row]
        f32 elems[16];
    };

    mat4x4f()
        : elems{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
    {
    }

    mat4x4f(f32 i_value)
        : mat4x4f()
    {
        m[0][0] = i_value;
        m[1][1] = i_value;
        m[2][2] = i_value;
        m[3][3] = i_value;
    }
};

mat4x4f operator*(const mat4x4f& i_a, const mat4x4f& i_b);
vec4f operator*(const mat4x4f& i_m, const vec4f& i_v);

///////////////////////////////////////////////////////////////////////////////
// camera

struct camera_view_t
{
    vec3f position;
    vec3f look_at; // maybe direction or target location
    vec3f up_direction;
};

struct camera_ortho_t
{
    f32 left, right, top, bottom;
    f32 near_plane, far_plane;
};

struct camera_persp_t
{
    f32 near_plane, far_plane;
    f32 fov; // degrees
    f32 aspect_ratio;
};

mat4x4f construct_lookat_dir_rh(const vec3f& i_upDir, const vec3f& i_camPos, const vec3f& i_lookAtDir);
mat4x4f construct_lookat_dir_lh(const vec3f& i_upDir, const vec3f& i_camPos, const vec3f& i_lookAtDir);
mat4x4f construct_lookat_point_rh(const vec3f& i_upDir, const vec3f& i_camPos, const vec3f& lookAtPoint);
mat4x4f construct_lookat_point_lh(const vec3f& i_upDir, const vec3f& i_camPos, const vec3f& lookAtPoint);
mat4x4f construct_lookat_dir_rh(const camera_view_t* i_desc);
mat4x4f construct_lookat_dir_lh(const camera_view_t* i_desc);
mat4x4f construct_lookat_point_rh(const camera_view_t* i_desc);
mat4x4f construct_lookat_point_lh(const camera_view_t* i_desc);
mat4x4f construct_orthographic_rh(const f32 i_left, const f32 i_right, const f32 i_top, const f32 i_bottom, const f32 i_near, const f32 i_far);
mat4x4f construct_orthographic_lh(const f32 i_left, const f32 i_right, const f32 i_top, const f32 i_bottom, const f32 i_near, const f32 i_far);
mat4x4f construct_orthographic_rh(const camera_ortho_t& i_desc);
mat4x4f construct_orthographic_lh(const camera_ortho_t& i_desc);
mat4x4f construct_perspective_rh(const f32 i_near, const f32 i_far, const f32 i_fovy, const f32 i_aspectRatio);
mat4x4f construct_infinity_perspective_rh(const f32 i_near, const f32 i_fovy, const f32 i_aspectRatio);
mat4x4f construct_perspective_lh(const f32 i_near, const f32 i_far, const f32 i_fovy, const f32 i_aspectRatio);
mat4x4f construct_infinity_perspective_lh(const f32 i_near, const f32 i_fovy, const f32 i_aspectRatio);
mat4x4f construct_perspective_rh(const camera_persp_t* i_desc);
mat4x4f construct_perspective_lh(const camera_persp_t* i_desc);
mat4x4f construct_translation3d(const f32 i_deltaX, const f32 i_deltaY, const f32 i_deltaZ);
mat4x4f construct_translation3d(const vec3f& i_delta);
mat4x4f construct_scaling3d(const f32 i_scaleX, const f32 i_scaleY, const f32 i_scaleZ);
mat4x4f construct_scaling3d(const vec3f& i_scale);
mat4x4f construct_x_rotation3d(f32 i_rXRad);
mat4x4f construct_y_rotation3d(f32 i_rYRad);
mat4x4f construct_z_rotation3d(f32 i_rZRad);
mat4x4f construct_axis_rotation3d(const vec3f& i_axis, const f32 i_angle);
mat4x4f construct_transpose(const mat4x4f& i_m);
vec3f apply_vector_transform(const vec3f& i_v, const mat4x4f& i_m);
vec3f apply_point_transform(const vec3f& i_p, const mat4x4f& i_m);

///////////////////////////////////////////////////////////////////////////////
// quaternion

struct quaternionf // NOLINT(readability-identifier-naming)
{
    union
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };

        struct
        {
            vec3f v;
            f32 s;
        };
    };

    quaternionf()
        : x(0),
          y(0),
          z(0),
          w(1)
    {
    }
};

quaternionf operator*(const quaternionf& q1, const quaternionf& q2);
quaternionf construct_quaternion_euler(const f32 i_rx, const f32 i_ry, const f32 i_rz);
quaternionf construct_quaternion_euler(const vec3f& i_r);
quaternionf construct_quaternion_axis(const vec3f& i_axis, const f32 i_r);
quaternionf construct_quaternion_axis_rad(const vec3f& i_axis, const f32 i_r);
quaternionf construct_quaternion_v2v(const vec3f& i_v0, const vec3f& i_v1);
mat4x4f to_transform(const quaternionf& i_q);

///////////////////////////////////////////////////////////////////////////////
// utils

// i_hue: in degrees [0..360]
// i_saturation: [0..1]
// i_value: [0..1]
vec3f transform_hsv(const vec3f& i_input, const f32 i_hue, const f32 i_saturation, const f32 i_value);
vec3f hsv_to_rgb(const vec3f& i_input);
vec3f rgb_to_hsv(const vec3f& i_input);
