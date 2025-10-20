#pragma once
#include "runtime/core/math/transform.h"
#include "runtime/core/meta/reflection/reflection.h"

#include <string>
#include <vector>
namespace Momo
{

    REFLECTION_TYPE(RawBone)
    CLASS(RawBone, Fields)
    {
        REFLECTION_BODY(RawBone);

    public:
        std::string name;          // 骨骼名称
        int         index;         // 骨骼唯一 ID
        Transform   binding_pose;  // 绑定姿态，骨骼在建模时的参考位置
        Matrix4x4_  tpose_matrix;  // 骨骼矩阵，用于蒙皮计算
        int         parent_index;  // 父骨骼索引
    };

    REFLECTION_TYPE(SkeletonData)
    CLASS(SkeletonData, Fields)
    {
        REFLECTION_BODY(SkeletonData);

    public:
        std::vector<RawBone> bones_map;       // 存储所有骨骼
        bool                 is_flat = false; // "bone.index" equals index in bones_map
        int                  root_index;      // 根骨骼索引

        // 拓扑排序
        bool in_topological_order = false; // TODO: if not in topological order, we need to topology sort in skeleton
                                           // build process
    };

} // namespace Momo