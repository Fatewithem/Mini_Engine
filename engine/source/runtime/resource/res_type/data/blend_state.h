#pragma once
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/resource/res_type/data/animation_clip.h"
#include "runtime/resource/res_type/data/animation_skeleton_node_map.h"
#include <string>
#include <vector>

namespace Momo
{

    // 逐骨骼的权重数组（Mask 动画）
    REFLECTION_TYPE(BoneBlendWeight)
    CLASS(BoneBlendWeight, Fields)
    {
        REFLECTION_BODY(BoneBlendWeight);

    public:
        std::vector<float> blend_weight;  // 混合权重
    };

    // 驱动动画
    REFLECTION_TYPE(BlendStateWithClipData)
    CLASS(BlendStateWithClipData, Fields)
    {
        REFLECTION_BODY(BlendStateWithClipData);

    public:
        int                          clip_count;           // 混合动画片段数目
        std::vector<AnimationClip>   blend_clip;           // 动画曲线
        std::vector<AnimSkelMap>     blend_anim_skel_map;  // 动画和目标骨架的映射关系
        std::vector<BoneBlendWeight> blend_weight;         // 每个 clip 的骨骼权重
        std::vector<float>           blend_ratio;          // 每个 clip 的全局混合比例
    };

    // 存储 JSON 格式
    REFLECTION_TYPE(BlendState)
    CLASS(BlendState, Fields)
    {
        REFLECTION_BODY(BlendState);

    public:
        int                      clip_count;                // 动画片段数量
        std::vector<std::string> blend_clip_file_path;      // 每个动画片段的资源路径
        std::vector<float>       blend_clip_file_length;    // 对应动画的时长
        std::vector<std::string> blend_anim_skel_map_path;  // 骨骼映射文件路径
        std::vector<float>       blend_weight;              // 全局权重
        std::vector<std::string> blend_mask_file_path;      // 骨骼 mask 文件路径
        std::vector<float>       blend_ratio;               // clip 的全局混合比例
    };

} // namespace Momo