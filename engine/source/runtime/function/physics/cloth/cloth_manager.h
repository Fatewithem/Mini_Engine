#pragma once

#include <vector>
#include <memory>
#include "runtime/core/math/vector3.h"

class ClothVertex;

namespace Momo
{
    class ClothManager
    {
    public:
        void initialize(int width, int height);
        void clear();


        void resetCloth();
        void updateCloth(std::vector<ClothVertex>& vertices, float dt, int width, int height);
        void applyConstraint(ClothVertex& v1, ClothVertex& v2, float resetLength);
        void updateVertexBuffer(std::vector<ClothVertex>& vertices, const std::vector<uint16_t>& indices);
        void updateNormals(std::vector<ClothVertex>& vertices, const std::vector<uint16_t>& indices);

        std::vector<Momo::ClothVertex> clothVertices;
        std::vector<uint16_t>    clothIndices;

        std::vector<Vector3> clothInitialPositions;
        double totalTime = 0.0f;
        int lastCycle = -1;

        int width = 32, height = 32;
    
    };
}