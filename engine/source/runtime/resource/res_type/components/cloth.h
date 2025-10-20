#pragma once

#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/math/vector3.h"
#include "runtime/resource/res_type/data/cloth_data.h"

namespace Momo
{
    class Cloth
    {
    public:
        std::vector<ClothVertex> clothVertexs;
        std::vector<uint16_t>    clothIndices;

        std::vector<Vector3> clothInitialPositions;
        double totalTime = 0.0f;
        int lastCycle = -1;

        int width = 32, height = 32;

        // 顶点输入布局实现
    };
}