#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include "runtime/core/color/color.h"
#include "runtime/core/math/vector3.h"

#include "runtime/resource/res_type/data/camera_config.h"

namespace Momo
{
    // 天空盒低频部分：模糊、无明显高光
    REFLECTION_TYPE(SkyBoxIrradianceMap)
    CLASS(SkyBoxIrradianceMap, Fields)
    {
        REFLECTION_BODY(SkyBoxIrradianceMap);

    public:
        std::string m_negative_x_map;
        std::string m_positive_x_map;
        std::string m_negative_y_map;
        std::string m_positive_y_map;
        std::string m_negative_z_map;
        std::string m_positive_z_map;
    };

    // 镜面 IBL
    REFLECTION_TYPE(SkyBoxSpecularMap)
    CLASS(SkyBoxSpecularMap, Fields)
    {
        REFLECTION_BODY(SkyBoxSpecularMap);

    public:
        std::string m_negative_x_map;
        std::string m_positive_x_map;
        std::string m_negative_y_map;
        std::string m_positive_y_map;
        std::string m_negative_z_map;
        std::string m_positive_z_map;
    };

    REFLECTION_TYPE(DirectionalLight)
    CLASS(DirectionalLight, Fields)
    {
        REFLECTION_BODY(DirectionalLight);

    public:
        Vector3 m_direction;
        Color   m_color;
    };

    REFLECTION_TYPE(GlobalRenderingRes)
    CLASS(GlobalRenderingRes, Fields)
    {
        REFLECTION_BODY(GlobalRenderingRes);

    public:
        bool                m_enable_fxaa {false};   // FXAA
        SkyBoxIrradianceMap m_skybox_irradiance_map;
        SkyBoxSpecularMap   m_skybox_specular_map;
        std::string         m_brdf_map;              // PBR 的 BRDF 积分查找表（LUT）
        std::string         m_color_grading_map;     // 颜色分级 LUT 贴图

        Color            m_sky_color;
        Color            m_ambient_light;            // 额外环境光颜色
        CameraConfig     m_camera_config;
        DirectionalLight m_directional_light;        // 太阳光
    };
} // namespace Momo