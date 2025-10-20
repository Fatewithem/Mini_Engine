#pragma once
#include "runtime/core/meta/reflection/reflection.h"

namespace Momo
{
    REFLECTION_TYPE(MaterialRes)
    CLASS(MaterialRes, Fields)
    {
        REFLECTION_BODY(MaterialRes);

    public:
        std::string m_base_colour_texture_file;         // 基础颜色
        std::string m_metallic_roughness_texture_file;  // 金属度 + 粗糙度
        std::string m_normal_texture_file;              // 法线贴图
        std::string m_occlusion_texture_file;           // 环境光遮蔽
        std::string m_emissive_texture_file;            // 自发光
    };
} // namespace Momo