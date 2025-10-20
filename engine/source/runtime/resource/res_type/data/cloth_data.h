#pragma once
#include "runtime/core/meta/reflection/reflection.h"
#include "runtime/core/math/math.h"
#include "runtime/core/math/vector3.h"
#include "runtime/core/math/vector2.h"

namespace Momo
{
    class ClothVertex
    {
    public:
        Vector3 pos;
        Vector3 prevPos;
        Vector3 normal;
        Vector2 uv;

        float invMass;
    };
};