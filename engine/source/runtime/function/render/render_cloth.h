#pragma once

#include "runtime/core/math/vector2.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector4.h"

#include <array>
#include "interface/rhi.h"

namespace Momo
{
    struct ClothMeshVertex
    {
        struct VulkanClothMeshVertexPosition
        {
            Vector3 position;
        };

        struct VulkanClothMeshVertexVarying
        {
            Vector3 normal;
            Vector2 uv;
        };

        static std::array<RHIVertexInputBindingDescription, 2> getBindingDescriptions()
        {
            std::array<RHIVertexInputBindingDescription, 2> bindings{};

            bindings[0].binding   = 0;
            bindings[0].stride    = sizeof(VulkanClothMeshVertexPosition);
            bindings[0].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;

            bindings[1].binding   = 1;
            bindings[1].stride    = sizeof(VulkanClothMeshVertexVarying);
            bindings[1].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;

            return bindings;
        };

        static std::array<RHIVertexInputAttributeDescription, 3> getAttributeDesciptions() 
        {
            std::array<RHIVertexInputAttributeDescription, 3> attributes{};

            // location 0: position
            attributes[0].binding  = 0;
            attributes[0].location = 0;
            attributes[0].format   = RHI_FORMAT_R32G32B32_SFLOAT;
            attributes[0].offset   = offsetof(VulkanClothMeshVertexPosition, position);

            // location 1: normal
            attributes[1].binding  = 1;
            attributes[1].location = 1;
            attributes[1].format   = RHI_FORMAT_R32G32B32_SFLOAT;
            attributes[1].offset   = offsetof(VulkanClothMeshVertexVarying, normal);

            // location 2: uv
            attributes[2].binding  = 1;
            attributes[2].location = 2;
            attributes[2].format   = RHI_FORMAT_R32G32_SFLOAT;
            attributes[2].offset   = offsetof(VulkanClothMeshVertexVarying, uv);

            return attributes;
        }
    };
}