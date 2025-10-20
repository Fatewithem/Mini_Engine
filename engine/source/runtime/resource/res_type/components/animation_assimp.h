#pragma once

#include "runtime/resource/res_type/data/blend_state.h"

#include <string>
#include <vector>

namespace Momo
{
    class AnimationAssimpResultElement
    {
    public:
        int        index;      // skeleton 的关节下标
        Matrix4x4_ transform;  // 变换矩阵用于蒙皮
    };

    class AnimationAssimpResult
    {
    public:
        std::vector<AnimationAssimpResultElement> node;  // 整套骨骼变换集合
    };

    class AnimationAssimpComponentRes
    {
    public:
        BlendState  blend_state;         // 动画混合配置

        // animation to skeleton map
        float       frame_position; // 0-1

        AnimationAssimpResult animation_result; 
    };
}

