#pragma once
#include "runtime/core/meta/reflection/reflection.h"

#include <string>
#include <vector>
namespace Momo
{

    REFLECTION_TYPE(Vertex)
    CLASS(Vertex, Fields)
    {
        REFLECTION_BODY(Vertex);

    public:
        float px;  // 位置
        float py;
        float pz;
        float nx;  // 法线
        float ny;
        float nz;
        float tx;  // 切线
        float ty;
        float tz;
        float u;   // 纹理坐标
        float v;
    };

    REFLECTION_TYPE(SkeletonBinding)
    CLASS(SkeletonBinding, Fields)
    {
        REFLECTION_BODY(SkeletonBinding);

    public:
        int   index0;
        int   index1;
        int   index2;
        int   index3;
        float weight0;
        float weight1;
        float weight2;
        float weight3;
    };
    
    REFLECTION_TYPE(MeshData)
    CLASS(MeshData, Fields)
    {
        REFLECTION_BODY(MeshData);

    public:
        std::vector<Vertex>          vertex_buffer;
        std::vector<int>             index_buffer;
        std::vector<SkeletonBinding> bind;
    };

} // namespace Momo