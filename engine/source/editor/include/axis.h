#pragma once

#include "runtime/function/render/render_entity.h"
#include "runtime/function/render/render_type.h"

namespace Momo
{
    // 渲染三个可视化轴
    class EditorTranslationAxis : public RenderEntity
    {
    public:
        EditorTranslationAxis();
        RenderMeshData m_mesh_data;
    };

    class EditorRotationAxis : public RenderEntity
    {
    public:
        EditorRotationAxis();
        RenderMeshData m_mesh_data;
    };

    class EditorScaleAxis : public RenderEntity
    {
    public:
        EditorScaleAxis();
        RenderMeshData m_mesh_data;
    };
}