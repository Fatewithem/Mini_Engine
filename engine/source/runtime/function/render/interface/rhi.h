#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>

#include <memory>
#include <vector>
#include <functional>

#include "rhi_struct.h"
namespace Momo
{
    class WindowSystem;

    struct RHIInitInfo
    {
        std::shared_ptr<WindowSystem> window_system;
    };
    
    class RHI
    {
    public:
        virtual ~RHI() = 0;

        // 初始化
        virtual void initialize(RHIInitInfo initialize_info) = 0;
        virtual void prepareContext() = 0;

        virtual bool isPointLightShadowEnabled() = 0;
        // allocate and create
        // 分配 CommandBuffer
        virtual bool allocateCommandBuffers(const RHICommandBufferAllocateInfo* pAllocateInfo, RHICommandBuffer* &pCommandBuffers) = 0;
        // 分配 DescriptorSets
        virtual bool allocateDescriptorSets(const RHIDescriptorSetAllocateInfo* pAllocateInfo, RHIDescriptorSet* &pDescriptorSets) = 0;
        // 创建 SwapChain
        virtual void createSwapchain() = 0;
        // 尺寸改变时重建 SwapChain
        virtual void recreateSwapchain() = 0;
        // 创建 ImageViews
        virtual void createSwapchainImageViews() = 0;
        // 创建 Framebuffer
        virtual void createFramebufferImageAndView() = 0;
        // 创建采样器 Sampler
        virtual RHISampler* getOrCreateDefaultSampler(RHIDefaultSamplerType type) = 0;
        // 创建 Mipmap采样器
        virtual RHISampler* getOrCreateMipmapSampler(uint32_t width, uint32_t height) = 0;
        // 通过 gsv 创建 Shader
        virtual RHIShader* createShaderModule(const std::vector<unsigned char>& shader_code) = 0;
        // 创建 Buffer 句柄
        virtual void createBuffer(RHIDeviceSize size, RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer* &buffer, RHIDeviceMemory* &buffer_memory) = 0;
        // 创建 Buffer 句柄并初始化（从 CPU 可见的内存中 copy）
        virtual void createBufferAndInitialize(RHIBufferUsageFlags usage, RHIMemoryPropertyFlags properties, RHIBuffer*& buffer, RHIDeviceMemory*& buffer_memory, RHIDeviceSize size, void* data = nullptr, int datasize = 0) = 0;
        // 创建 Buffer 句柄（自动选对内存类型，并简化映射/释放操作）
        virtual bool createBufferVMA(VmaAllocator allocator,
            const RHIBufferCreateInfo* pBufferCreateInfo,
            const VmaAllocationCreateInfo* pAllocationCreateInfo,
            RHIBuffer* &pBuffer,
            VmaAllocation* pAllocation,
            VmaAllocationInfo* pAllocationInfo) = 0;
        virtual bool createBufferWithAlignmentVMA(
            VmaAllocator allocator,
            const RHIBufferCreateInfo* pBufferCreateInfo,
            const VmaAllocationCreateInfo* pAllocationCreateInfo,
            RHIDeviceSize minAlignment,
            RHIBuffer* &pBuffer,
            VmaAllocation* pAllocation,
            VmaAllocationInfo* pAllocationInfo) = 0;
        // 复制 Buffer
        virtual void copyBuffer(RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, RHIDeviceSize srcOffset, RHIDeviceSize dstOffset, RHIDeviceSize size) = 0;
        // 创建一个Image（包括 color 以及 depth）
        virtual void createImage(uint32_t image_width, uint32_t image_height, RHIFormat format, RHIImageTiling image_tiling, RHIImageUsageFlags image_usage_flags, RHIMemoryPropertyFlags memory_property_flags,
            RHIImage* &image, RHIDeviceMemory* &memory, RHIImageCreateFlags image_create_flags, uint32_t array_layers, uint32_t miplevels) = 0;
        // 创建 ImageView（用在 SwapChain 中）
        virtual void createImageView(RHIImage* image, RHIFormat format, RHIImageAspectFlags image_aspect_flags, RHIImageViewType view_type, uint32_t layout_count, uint32_t miplevels,
            RHIImageView* &image_view) = 0;
        // 创建特殊类型的 Image
        virtual void createGlobalImage(RHIImage* &image, RHIImageView* &image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, void* texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels = 0) = 0;
        // 需要设置 Array 的参数
        virtual void createCubeMap(RHIImage* &image, RHIImageView* &image_view, VmaAllocation& image_allocation, uint32_t texture_image_width, uint32_t texture_image_height, std::array<void*, 6> texture_image_pixels, RHIFormat texture_image_format, uint32_t miplevels) = 0;
        // 创建 commandPool
        virtual void createCommandPool() = 0;
        virtual bool createCommandPool(const RHICommandPoolCreateInfo* pCreateInfo, RHICommandPool*& pCommandPool) = 0;
        // 创建 DescriptorPool
        virtual bool createDescriptorPool(const RHIDescriptorPoolCreateInfo* pCreateInfo, RHIDescriptorPool* &pDescriptorPool) = 0;
        // 创建 DescriptorLayout（Set？）
        virtual bool createDescriptorSetLayout(const RHIDescriptorSetLayoutCreateInfo* pCreateInfo, RHIDescriptorSetLayout* &pSetLayout) = 0;
        // 创建 Fence
        virtual bool createFence(const RHIFenceCreateInfo* pCreateInfo, RHIFence* &pFence) = 0;
        // 创建 Framebuffer
        virtual bool createFramebuffer(const RHIFramebufferCreateInfo* pCreateInfo, RHIFramebuffer* &pFramebuffer) = 0;
        // 创建 GraphicsPipeline
        virtual bool createGraphicsPipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIGraphicsPipelineCreateInfo* pCreateInfos, RHIPipeline* &pPipelines) = 0;
        // 创建 ComputePipeline
        virtual bool createComputePipelines(RHIPipelineCache* pipelineCache, uint32_t createInfoCount, const RHIComputePipelineCreateInfo* pCreateInfos, RHIPipeline* &pPipelines) = 0;
        // 绑定 Pipeline 中对应使用的 DescriptorSetLayout
        virtual bool createPipelineLayout(const RHIPipelineLayoutCreateInfo* pCreateInfo, RHIPipelineLayout* &pPipelineLayout) = 0;
        // 创建 RenderPass
        virtual bool createRenderPass(const RHIRenderPassCreateInfo* pCreateInfo, RHIRenderPass* &pRenderPass) = 0;
        // 创建 Sampler
        virtual bool createSampler(const RHISamplerCreateInfo* pCreateInfo, RHISampler* &pSampler) = 0;
        // 创建 Semaphore
        virtual bool createSemaphore(const RHISemaphoreCreateInfo* pCreateInfo, RHISemaphore* &pSemaphore) = 0;

        // command and command write
        // 等待 Fence
        virtual bool waitForFencesPFN(uint32_t fenceCount, RHIFence* const* pFence, RHIBool32 waitAll, uint64_t timeout) = 0;
        // 重置 Fence
        virtual bool resetFencesPFN(uint32_t fenceCount, RHIFence* const* pFences) = 0;
        // 重置 CommandPool
        virtual bool resetCommandPoolPFN(RHICommandPool* commandPool, RHICommandPoolResetFlags flags) = 0;
        // 开始一个 CommandBuffer（分配 buffer 等操作需要 GPU 的 command 来执行）
        virtual bool beginCommandBufferPFN(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) = 0;
        // 释放 CommandBuffer
        virtual bool endCommandBufferPFN(RHICommandBuffer* commandBuffer) = 0;
        // RenderPass
        virtual void cmdBeginRenderPassPFN(RHICommandBuffer* commandBuffer, const RHIRenderPassBeginInfo* pRenderPassBegin, RHISubpassContents contents) = 0;
        virtual void cmdNextSubpassPFN(RHICommandBuffer* commandBuffer, RHISubpassContents contents) = 0;
        virtual void cmdEndRenderPassPFN(RHICommandBuffer* commandBuffer) = 0;
        // 绑定 Pipeline
        virtual void cmdBindPipelinePFN(RHICommandBuffer* commandBuffer, RHIPipelineBindPoint pipelineBindPoint, RHIPipeline* pipeline) = 0;
        // 动态设置参数
        virtual void cmdSetViewportPFN(RHICommandBuffer* commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const RHIViewport* pViewports) = 0;
        virtual void cmdSetScissorPFN(RHICommandBuffer* commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const RHIRect2D* pScissors) = 0;
        // 绑定 Vertex
        virtual void cmdBindVertexBuffersPFN(
            RHICommandBuffer* commandBuffer,
            uint32_t firstBinding,
            uint32_t bindingCount,
            RHIBuffer* const* pBuffers,
            const RHIDeviceSize* pOffsets) = 0;
        // 绑定 Index
        virtual void cmdBindIndexBufferPFN(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset, RHIIndexType indexType) = 0;
        // 绑定 Pipeline 和 DescriptorSets
        virtual void cmdBindDescriptorSetsPFN(
            RHICommandBuffer* commandBuffer,
            RHIPipelineBindPoint pipelineBindPoint,
            RHIPipelineLayout* layout,
            uint32_t firstSet,
            uint32_t descriptorSetCount,
            const RHIDescriptorSet* const* pDescriptorSets,
            uint32_t dynamicOffsetCount,
            const uint32_t* pDynamicOffsets) = 0;
        // 绘制函数
        virtual void cmdDrawIndexedPFN(RHICommandBuffer* commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;
        // 清除 Attachment
        virtual void cmdClearAttachmentsPFN(RHICommandBuffer* commandBuffer, uint32_t attachmentCount, const RHIClearAttachment* pAttachments, uint32_t rectCount, const RHIClearRect* pRects) = 0;
        
        virtual bool beginCommandBuffer(RHICommandBuffer* commandBuffer, const RHICommandBufferBeginInfo* pBeginInfo) = 0;
        // 复制 Image 到 Buffer 中
        virtual void cmdCopyImageToBuffer(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageLayout srcImageLayout, RHIBuffer* dstBuffer, uint32_t regionCount, const RHIBufferImageCopy* pRegions) = 0;
        virtual void cmdCopyImageToImage(RHICommandBuffer* commandBuffer, RHIImage* srcImage, RHIImageAspectFlagBits srcFlag, RHIImage* dstImage, RHIImageAspectFlagBits dstFlag, uint32_t width, uint32_t height) = 0;
        virtual void cmdCopyBuffer(RHICommandBuffer* commandBuffer, RHIBuffer* srcBuffer, RHIBuffer* dstBuffer, uint32_t regionCount, RHIBufferCopy* pRegions) = 0;
        virtual void cmdDraw(RHICommandBuffer* commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
        // 调用 Compute Shader
        virtual void cmdDispatch(RHICommandBuffer* commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
        virtual void cmdDispatchIndirect(RHICommandBuffer* commandBuffer, RHIBuffer* buffer, RHIDeviceSize offset) = 0;
        // 设置 Barrier
        virtual void cmdPipelineBarrier(RHICommandBuffer* commandBuffer, RHIPipelineStageFlags srcStageMask, RHIPipelineStageFlags dstStageMask, RHIDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const RHIMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const RHIBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const RHIImageMemoryBarrier* pImageMemoryBarriers) = 0;
        virtual bool endCommandBuffer(RHICommandBuffer* commandBuffer) = 0;
        // CPU 侧更新 DesciptorSets 内容
        virtual void updateDescriptorSets(uint32_t descriptorWriteCount, const RHIWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const RHICopyDescriptorSet* pDescriptorCopies) = 0;
        virtual bool queueSubmit(RHIQueue* queue, uint32_t submitCount, const RHISubmitInfo* pSubmits, RHIFence* fence) = 0;
        virtual bool queueWaitIdle(RHIQueue* queue) = 0;
        virtual void resetCommandPool() = 0;
        virtual void waitForFences() = 0;

        // query
        virtual void getPhysicalDeviceProperties(RHIPhysicalDeviceProperties* pProperties) = 0;
        virtual RHICommandBuffer* getCurrentCommandBuffer() const = 0;
        virtual RHICommandBuffer* const* getCommandBufferList() const = 0;
        virtual RHICommandPool* getCommandPoor() const = 0;
        virtual RHIDescriptorPool* getDescriptorPoor() const = 0;
        virtual RHIFence* const* getFenceList() const = 0;
        virtual QueueFamilyIndices getQueueFamilyIndices() const = 0;
        virtual RHIQueue* getGraphicsQueue() const = 0;
        virtual RHIQueue* getComputeQueue() const = 0;
        virtual RHISwapChainDesc getSwapchainInfo() = 0;
        virtual RHIDepthImageDesc getDepthImageInfo() const = 0;
        virtual uint8_t getMaxFramesInFlight() const = 0;
        virtual uint8_t getCurrentFrameIndex() const = 0;
        virtual void setCurrentFrameIndex(uint8_t index) = 0;

        // command write
        virtual RHICommandBuffer* beginSingleTimeCommands() = 0;
        virtual void            endSingleTimeCommands(RHICommandBuffer* command_buffer) = 0;
        virtual bool prepareBeforePass(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;
        virtual void submitRendering(std::function<void()> passUpdateAfterRecreateSwapchain) = 0;
        virtual void pushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color) = 0;
        virtual void popEvent(RHICommandBuffer* commond_buffer) = 0;

        // destory
        virtual void clear() = 0;
        virtual void clearSwapchain() = 0;
        virtual void destroyDefaultSampler(RHIDefaultSamplerType type) = 0;
        virtual void destroyMipmappedSampler() = 0;
        virtual void destroyShaderModule(RHIShader* shader) = 0;
        virtual void destroySemaphore(RHISemaphore* semaphore) = 0;
        virtual void destroySampler(RHISampler* sampler) = 0;
        virtual void destroyInstance(RHIInstance* instance) = 0;
        virtual void destroyImageView(RHIImageView* imageView) = 0;
        virtual void destroyImage(RHIImage* image) = 0;
        virtual void destroyFramebuffer(RHIFramebuffer* framebuffer) = 0;
        virtual void destroyFence(RHIFence* fence) = 0;
        virtual void destroyDevice() = 0;
        virtual void destroyCommandPool(RHICommandPool* commandPool) = 0;
        virtual void destroyBuffer(RHIBuffer* &buffer) = 0;
        virtual void freeCommandBuffers(RHICommandPool* commandPool, uint32_t commandBufferCount, RHICommandBuffer* pCommandBuffers) = 0;

        // memory
        virtual void freeMemory(RHIDeviceMemory* &memory) = 0;
        virtual bool mapMemory(RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size, RHIMemoryMapFlags flags, void** ppData) = 0;
        virtual void unmapMemory(RHIDeviceMemory* memory) = 0;
        // GPU → CPU 的刷新
        virtual void invalidateMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) = 0;
        // CPU → GPU 的刷新
        virtual void flushMappedMemoryRanges(void* pNext, RHIDeviceMemory* memory, RHIDeviceSize offset, RHIDeviceSize size) = 0;

        //semaphores
        virtual RHISemaphore* &getTextureCopySemaphore(uint32_t index) = 0;

    private:
    };

    inline RHI::~RHI() = default;
} // namespace Momo
