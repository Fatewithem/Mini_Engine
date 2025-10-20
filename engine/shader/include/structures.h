struct VulkanMeshInstance
{
    highp float enable_vertex_blending;             // 是否启用骨骼蒙皮
    highp float _padding_enable_vertex_blending_1;
    highp float _padding_enable_vertex_blending_2;
    highp float _padding_enable_vertex_blending_3;
    highp mat4  model_matrix;                       // 实例的模型矩阵
};

struct VulkanMeshVertexJointBinding
{
    highp ivec4 indices;
    highp vec4  weights;
};
