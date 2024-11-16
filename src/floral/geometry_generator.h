#pragma once

#include "configs.h"
#include "stdaliases.h"
#include "vector_math.h"

///////////////////////////////////////////////////////////////////////////////

// assumptions:
// - vertex position: 2d / 3d vector of f32(s)
// - vertex normal: 2d / 3d vector of f32(s)
// - texture coordinates: 2d / 3d vector of f32(s)

enum class geo_vertex_format_e : s32
{
    position = 1,
    normal = position << 1,
    tangent = normal << 1,
    tex_coord = tangent << 1
};

geo_vertex_format_e operator|(const geo_vertex_format_e i_lhs,
                              const geo_vertex_format_e i_rhs)
{
    return (geo_vertex_format_e)((s32)i_lhs | (s32)i_rhs);
}

struct geo_generate_result_t
{
    u32 verticesGenerated;
    u32 indicesGenerated;
};

struct manifold_geo_generate_result_t
{
    u32 verticesGenerated;
    u32 indicesGenerated;
    u32 manifoldVerticesGenerated;
    u32 manifoldIndicesGenerated;
};

void push_generation_transform(const mat4x4f& i_xform);
void pop_generation_transform();
void reset_generation_transforms_stack();

// transformation stack will affect these functions
geo_generate_result_t
generate_quadtes_plane_3d(const s32 i_startIdx, const size i_vtxStride, const geo_vertex_format_e i_vtxFormat,
                          const f32 i_quadSize, voidptr o_vtxData, s32* o_idxData);
geo_generate_result_t
generate_unit_box_3d(const s32 i_startIdx, const size i_vtxStride, const geo_vertex_format_e i_vtxFormat,
                     voidptr o_vtxData, s32* o_idxData);
#if 0
geo_generate_result_t							generate_unit_icosphere_3d(const u32 i_tesLevel, const s32 i_startIdx, const size i_vtxStride, const geo_vertex_format_e i_vtxFormat, voidptr o_vtxData, s32* o_idxData);
geo_generate_result_t							generate_unit_uvsphere_3d(const u32 i_startIdx, const size i_vtxStride, const geo_vertex_format_e i_vtxFormat, voidptr o_vtxData, s32* o_idxData);

geo_generate_result_t							generate_unit_plane_3d(const s32 i_startIdx, const size i_vtxStride, const geo_vertex_format_e i_vtxFormat, voidptr o_vtxData, s32* o_idxData);

manifold_geo_generate_result_t					generate_manifold_quadtes_unit_plane_3d(
													const s32 i_startIdx, const size i_vtxStride,
													const geo_vertex_format_e i_vtxFormat, const f32 i_quadSize, voidptr o_vtxData, s32* o_idxData,
													const s32 i_mnfStartIdx, const size i_mnfVtxStride, const f32 i_mnfThickness,
													const geo_vertex_format_e i_mnfVtxFormat, voidptr o_mnfVtxData, s32* o_mnfIdxData);

manifold_geo_generate_result_t					generate_manifold_unit_plane_3d(
													const s32 i_startIdx, const size i_vtxStride,
													const geo_vertex_format_e i_vtxFormat, voidptr o_vtxData, s32* o_idxData,
													const s32 i_mnfStartIdx, const size i_mnfVtxStride, const f32 i_mnfThickness,
													const geo_vertex_format_e i_mnfVtxFormat, voidptr o_mnfVtxData, s32* o_mnfIdxData);

manifold_geo_generate_result_t					generate_manifold_icosphere_3d(
													const s32 i_startIdx, const size i_vtxStride,
													const geo_vertex_format_e i_vtxFormat, voidptr o_vtxData, s32* o_idxData,
													const s32 i_mnfStartIdx, const size i_mnfVtxStride, const f32 i_mnfThickness,
													const geo_vertex_format_e i_mnfVtxFormat, voidptr o_mnfVtxData, s32* o_mnfIdxData);
#endif
