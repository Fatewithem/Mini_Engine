#pragma once

#include "runtime/function/render/render_common.h"
#include "runtime/function/render/render_pass_base.h"
#include "runtime/function/render/render_resource.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace Momo
{
    class VulkanRHI;

    enum
    {
        _main_camera_pass_gbuffer_a                     = 0,
        _main_camera_pass_gbuffer_b                     = 1,
        _main_camera_pass_gbuffer_c                     = 2,
        _main_camera_pass_backup_buffer_odd             = 3,
        _main_camera_pass_backup_buffer_even            = 4,
        _main_camera_pass_post_process_buffer_odd       = 5,
        _main_camera_pass_post_process_buffer_even      = 6,
        _main_camera_pass_depth                         = 7,
        _main_camera_pass_swap_chain_image              = 8,
        _main_camera_pass_custom_attachment_count       = 5,
        _main_camera_pass_post_process_attachment_count = 2,
        _main_camera_pass_attachment_count              = 9,
    };

    enum
    {
        _main_camera_subpass_basepass = 0,
        _main_camera_subpass_deferred_lighting,
        _main_camera_subpass_forward_lighting,
        _main_camera_subpass_tone_mapping,
        _main_camera_subpass_color_grading,
        _main_camera_subpass_fxaa,
        _main_camera_subpass_ui,
        _main_camera_subpass_combine_ui,
        _main_camera_subpass_count
    };

    struct VisiableNodes
    {
        std::vector<RenderMeshNode>*              p_directional_light_visible_mesh_nodes {nullptr};  // 平行光下可见的 mesh
        std::vector<RenderMeshNode>*              p_point_lights_visible_mesh_nodes {nullptr};       // 点光源下可见的 mesh
        std::vector<RenderMeshNode>*              p_main_camera_visible_mesh_nodes {nullptr};        // 主相机可见的 mesh
        RenderAxisNode*                           p_axis_node {nullptr};                             // 世界坐标轴
    };

    class RenderPass : public RenderPassBase
    {
    public:
        // FrameBuffer 中的 Attachment（Color / Depth）
        struct FrameBufferAttachment
        {
            RHIImage*        image;  // 显存中的纹理
            RHIDeviceMemory* mem;    // 显存分配对象
            RHIImageView*    view;   // 图像格式
            RHIFormat       format;  // 像素格式
        };

        struct Framebuffer
        {
            int           width;
            int           height;
            RHIFramebuffer* framebuffer;
            RHIRenderPass*  render_pass;  // 绑定一个 RenderPass

            std::vector<FrameBufferAttachment> attachments;  // 绑定内部的 Attachment
        };

        struct Descriptor
        {
            RHIDescriptorSetLayout* layout;          // 设置 DescriptorSetLayout
            RHIDescriptorSet*       descriptor_set;  // 设置 DescriptorSet
        };

        struct RenderPipelineBase
        {
            RHIPipelineLayout* layout;    // 资源绑定布局
            RHIPipeline*       pipeline;  // 包含 shader、光栅化状态、深度/模板测试等
        };

        GlobalRenderResource*      m_global_render_resource {nullptr};

        std::vector<Descriptor>         m_descriptor_infos;
        std::vector<RenderPipelineBase> m_render_pipelines;
        Framebuffer                     m_framebuffer;

        void initialize(const RenderPassInitInfo* init_info) override;
        void postInitialize() override;

        virtual void draw();

        virtual RHIRenderPass*                       getRenderPass() const;
        virtual std::vector<RHIImageView*>           getFramebufferImageViews() const;
        virtual std::vector<RHIDescriptorSetLayout*> getDescriptorSetLayouts() const;

        static VisiableNodes m_visiable_nodes;

    private:
    };
} // namespace Momo
