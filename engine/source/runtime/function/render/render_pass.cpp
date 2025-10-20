#include "runtime/function/render/render_pass.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/render/render_resource.h"
#include "runtime/function/render/interface/vulkan/vulkan_rhi.h"

Momo::VisiableNodes Momo::RenderPass::m_visiable_nodes;  // 全局静态变量：本帧可见的渲染对象

namespace Momo
{   
    // 取出全局渲染资源的地址，并存入 m_global_render_resource
    void RenderPass::initialize(const RenderPassInitInfo* init_info)
    {
        m_global_render_resource =
            &(std::static_pointer_cast<RenderResource>(m_render_resource)->m_global_render_resource);
    }
    void RenderPass::draw() {}

    void RenderPass::postInitialize() {}

    // 返回 subpass
    RHIRenderPass* RenderPass::getRenderPass() const { return m_framebuffer.render_pass; }

    // 遍历 framebuffer 中的所有 attachment，取出每个的 RHIImageView
    std::vector<RHIImageView*> RenderPass::getFramebufferImageViews() const
    {
        std::vector<RHIImageView*> image_views;
        for (auto& attach : m_framebuffer.attachments)
        {
            image_views.push_back(attach.view);
        }
        return image_views;
    }

    // 取出所有 RHIDescriptorSetLayout
    std::vector<RHIDescriptorSetLayout*> RenderPass::getDescriptorSetLayouts() const
    {
        std::vector<RHIDescriptorSetLayout*> layouts;
        for (auto& desc : m_descriptor_infos)
        {
            layouts.push_back(desc.layout);
        }
        return layouts;
    }
}