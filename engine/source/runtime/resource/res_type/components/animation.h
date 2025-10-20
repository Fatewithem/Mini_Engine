#pragma once

#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/resource/res_type/data/blend_state.h"

#include <string>
#include <vector>

namespace Momo
{

    REFLECTION_TYPE(AnimationResultElement)
    CLASS(AnimationResultElement, WhiteListFields)
    {
        REFLECTION_BODY(AnimationResultElement);

    public:
        int        index;      // skeleton 的关节下标
        Matrix4x4_ transform;  // 变换矩阵用于蒙皮
    };

    REFLECTION_TYPE(AnimationResult)
    CLASS(AnimationResult, Fields)
    {
        REFLECTION_BODY(AnimationResult);

    public:
        std::vector<AnimationResultElement> node;  // 整套骨骼变换集合
    };

    REFLECTION_TYPE(AnimationComponentRes)
    CLASS(AnimationComponentRes, Fields)
    {
        REFLECTION_BODY(AnimationComponentRes);

    public:
        std::string skeleton_file_path;  // 骨骼文件的相对位置
        BlendState  blend_state;         // 动画混合配置

        // animation to skeleton map
        float       frame_position; // 0-1

        META(Disable)
        AnimationResult animation_result;  //  反射中禁用
    };

} // namespace Momo