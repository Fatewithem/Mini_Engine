#pragma once

#include "runtime/core/math/transform.h"
#include "runtime/core/meta/reflection/reflection.h"
#include <string>
#include <vector>

namespace Momo
{

    REFLECTION_TYPE(AnimNodeMap)
    CLASS(AnimNodeMap, Fields)
    {
        REFLECTION_BODY(AnimNodeMap);

    public:
        std::vector<std::string> convert;  // 骨骼名字和索引映射
    };

    // 某个节点随时间变化的关键帧数据
    REFLECTION_TYPE(AnimationChannel)
    CLASS(AnimationChannel, Fields)
    {
        REFLECTION_BODY(AnimationChannel);

    public:
        std::string             name;
        std::vector<Vector3>    position_keys;
        std::vector<Quaternion> rotation_keys;
        std::vector<Vector3>    scaling_keys;
    };

    REFLECTION_TYPE(AnimationClip)
    CLASS(AnimationClip, Fields)
    {
        REFLECTION_BODY(AnimationClip);

    public:
        int                           total_frame {0};  // 动画总帧数
        int                           node_count {0};   // 参与动画的骨骼节点数量
        std::vector<AnimationChannel> node_channels;    // 每个骨骼的动画通道
    };

    REFLECTION_TYPE(AnimationAsset)
    CLASS(AnimationAsset, Fields)
    {
        REFLECTION_BODY(AnimationAsset);

    public:
        AnimNodeMap   node_map;            // 骨骼名字
        AnimationClip clip_data;           // 动画曲线
        std::string   skeleton_file_path;  // 对应的骨架资源
    };

} // namespace Momo