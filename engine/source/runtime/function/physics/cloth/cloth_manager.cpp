#include "runtime/function/physics/cloth/cloth_manager.h"
#include "runtime/resource/res_type/components/cloth.h"

namespace Momo
{
    void ClothManager::initialize(int width, int height) 
    {
        float dx = 0.1f, dy = 0.1f;
        clothVertices.clear();
        clothIndices.clear();
        clothInitialPositions.clear();
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                ClothVertex v;
                v.pos = Vector3(x * dx, -y * dy, 0.0f);
                v.prevPos = v.pos;
                v.normal = Vector3(0, 0, 1);
                v.uv = Vector2(x / float(width - 1), y / float(height - 1));
                v.invMass = 1.0f;
                clothVertices.push_back(v);
                clothInitialPositions.push_back(v.pos);
            }
        }
        // 固定两个角点：左上角和右上角
        // clothVertices[0].invMass = 0.0f;
        // clothVertices[width - 1].invMass = 0.0f;
        // 生成索引
        for (int y = 0; y < height - 1; ++y) {
            for (int x = 0; x < width - 1; ++x) {
                int i0 = y * width + x;
                int i1 = i0 + 1;
                int i2 = i0 + width;
                int i3 = i2 + 1;
                clothIndices.push_back(i0);
                clothIndices.push_back(i2);
                clothIndices.push_back(i1);
                clothIndices.push_back(i1);
                clothIndices.push_back(i2);
                clothIndices.push_back(i3);
            }
        }
    }

    // Reset cloth to initial rest pose
    void ClothManager::resetCloth() 
    {
        if (clothInitialPositions.size() != clothVertices.size()) return;
        for (size_t i = 0; i < clothVertices.size(); ++i) {
            clothVertices[i].pos = clothInitialPositions[i];
            clothVertices[i].prevPos = clothInitialPositions[i];
        }

        // 将最新的顶点数据同步到 GPU
        // updateVertexBuffer(clothVertices, vertexBuffer, vertexBufferMemory);
    }

    // 更新布料顶点（verlet积分）
    void ClothManager::updateCloth(std::vector<ClothVertex>& vertices, float dt, int width, int height) 
    {
        Vector3 gravity(0.0f, 0.0f, -9.8f);
        Vector3 wind(3.0f, 4.0f, 0.0f);
        // int height = 16, width = 16;

        for(size_t i = 0; i < vertices.size(); ++ i) {
            auto& v = vertices[i];

            Vector3 temp = v.pos;
            if (v.invMass == 0.0f) continue;

            Vector3 accel = gravity;

            // 风力
            if(i % 5 == 0) {
                accel += wind * 0.5f;
            }

            // 横向外力
            if(i / 16 > 8) {
                accel += Vector3(sin(i * 0.1f) * 2.0f, 0.0f, 0.0f);
            }

            v.pos = v.pos + (v.pos - v.prevPos) + accel * dt * dt;
            v.prevPos = temp;
        }
        
        const int iterations = 3;
        const float restLength = 0.1f;

        for(int iter = 0; iter < iterations; ++iter) {
            for(int y = 0; y < height; ++y) {
                for(int x = 0; x < width; ++x) {
                    int i0 = y * width + x;

                    if(x < width - 1) {
                        int i1 = i0 + 1;
                        applyConstraint(vertices[i0], vertices[i1], restLength);
                    }

                    if(y < height - 1) {
                        int i1 = i0 + width;
                        applyConstraint(vertices[i0], vertices[i1], restLength);
                    }

                    if(x < width - 1 && y < height - 1) {
                        applyConstraint(vertices[i0], vertices[i0 + width + 1], restLength * 1.414f);
                        applyConstraint(vertices[i0 + 1], vertices[i0 + width], restLength * 1.414f);
                    }
                }
            }
        }

        // --- Collision correction ---
        Vector3 sphereCenter(1.0f, -1.0f, 0.3f);
        float sphereRadius = 0.5f;

        for (auto& v : vertices) {
            // 与地面碰撞（z = 0）
            if (v.pos.z < 0.0f) {
                v.pos.z = 0.0f;
            }

            // 与球体碰撞
            Vector3 delta = v.pos - sphereCenter;
            float distSq = delta.squaredLength();
            float radiusSq = sphereRadius * sphereRadius;

            if (distSq < radiusSq) {
                // 顶点在球体内部
                float dist = std::sqrt(distSq);  // 只在需要时再开方
                Vector3 correctionDir = delta / dist;
                v.pos = sphereCenter + correctionDir * sphereRadius;
            }
        }
    }

    // 约束函数
    void ClothManager::applyConstraint(ClothVertex& v1, ClothVertex& v2, float restLength) 
    {
        Vector3 delta = v2.pos - v1.pos;
        float dist = delta.length();
        if (dist == 0.0f) return;

        Vector3 correction = delta * (1.0f - restLength / dist) * 0.5f;

        if (v1.invMass > 0.0f) v1.pos += correction;
        if (v2.invMass > 0.0f) v2.pos -= correction;
    }

    void ClothManager::updateNormals(std::vector<ClothVertex>& vertices, const std::vector<uint16_t>& indices) 
    {
        for (auto& v : vertices) v.normal = Vector3(0.0f, 0.0f, 0.0f);

        for (size_t i = 0; i < indices.size(); i += 3) {
            int i0 = indices[i];
            int i1 = indices[i + 1];
            int i2 = indices[i + 2];

            Vector3 e1 = vertices[i1].pos - vertices[i0].pos;
            Vector3 e2 = vertices[i2].pos - vertices[i0].pos;
            Vector3 n = e1.crossProduct(e2);
            n.normalise();

            vertices[i0].normal += n;
            vertices[i1].normal += n;
            vertices[i2].normal += n;
        }

        for (auto& v : vertices)
            v.normal.normalise();
    }
}