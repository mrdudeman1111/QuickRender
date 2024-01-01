#include "Interface.h"
#include "Memory.h"
#include "Wrappers.h"

#include <sail-c++/sail-c++.h>
#include <cstdint>
#include <cstring>
#include <vulkan/vulkan_core.h>

namespace Ek
{
/* Instance/Device Layers and Extensions */
  bool vulkanInterface::AddInstLayer(const char* pLayer)
  {
    if(InstLayerProperties.size() == 0)
    {
      uint32_t PropertyCount;
      vkEnumerateInstanceLayerProperties(&PropertyCount, nullptr);
      InstLayerProperties.resize(PropertyCount);
      vkEnumerateInstanceLayerProperties(&PropertyCount, InstLayerProperties.data());
    }

    for(uint32_t i = 0; i < InstLayerProperties.size(); i++)
    {
      if(strcmp(InstLayerProperties[i].layerName, pLayer) == 0)
      {
        InstanceLayers.push_back(pLayer);
        return true;
      }
    }

    return false;
  }

  bool vulkanInterface::AddInstExtension(const char* pExtension)
  {
    if(InstExtensionProperties.size() == 0)
    {
      uint32_t PropertyCount;
      vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, nullptr);
      InstExtensionProperties.resize(PropertyCount);
      vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, InstExtensionProperties.data());
    }

    for(uint32_t i = 0; i < InstExtensionProperties.size(); i++)
    {
      if(strcmp(InstExtensionProperties[i].extensionName, pExtension) == 0)
      {
        InstanceExtensions.push_back(pExtension);
        return true;
      }
    }

    return false;
  }

  bool vulkanInterface::AddDevExtension(const char* pExtension)
  {
    if(DevExtensionProperties.size() == 0)
    {
      uint32_t PropertyCount;
      vkEnumerateDeviceExtensionProperties(PDevice, nullptr, &PropertyCount, nullptr);
      DevExtensionProperties.resize(PropertyCount);
      vkEnumerateDeviceExtensionProperties(PDevice, nullptr, &PropertyCount, DevExtensionProperties.data());
    }

    for(uint32_t i = 0; i < DevExtensionProperties.size(); i++)
    {
      if(strcmp(DevExtensionProperties[i].extensionName, pExtension))
      {
        DeviceExtensions.push_back(pExtension);
        return true;
      }
    }

    return false;
  }
/* Instance/Device Layers and Extensions */

/* Framebuffer/Renderpass */
  const void vulkanInterface::AddAttachment(Ek::Wrappers::FrameBufferAttachment Attachment)
  {
    Attachments.push_back(Attachment);
  }
/* Framebuffer/Renderpass */

/* Allocations */
  void vulkanInterface::AllocateTexture(Ek::Texture& inTexture, Ek::eMemoryType MemType)
  {
    if(MemType == Ek::eLocalMemory)
    {
      LocalMemory.AllocateTexture(inTexture);
    }
    else
    {
      HostMemory.AllocateTexture(inTexture);
    }
  }

  void vulkanInterface::AllocateBuffer(Ek::Buffer& inBuff, Ek::eMemoryType MemType)
  {
    if(MemType == Ek::eLocalMemory)
    {
      LocalMemory.AllocateBuffer(inBuff);
    }
    else
    {
      HostMemory.AllocateBuffer(inBuff);
    }
  }
/* Allocations */

/* Producers */
  Material vulkanInterface::CreateMaterial()
  {
    Material Ret(&Device);
    return Ret;
  }

  void vulkanInterface::CreateMesh(const char* MeshPath, Mesh& pMesh)
  {
    pMesh.Load(this, Device, ShaderResources, MeshPath);

    Ek::Wrappers::CommandBuffer cmdBuffer = GetCommandBuffer(Ek::eTransfer);

    pMesh.Allocate(cmdBuffer);
  }

  PipelineInterface* vulkanInterface::CreatePipeline(Material& Mat, VkOffset2D PipeOffset, VkExtent2D PipeExtent, bool bDepthEnabled)
  {
    PipelineInterface* Ret = new PipelineInterface();

    Ret->SetDescriptorLayout(ShaderResources.DescriptorLayout);
    Ret->Init(Device, Mat, Attachments.data(), Attachments.size(), PipeOffset, PipeExtent, RenderPass, 0, bDepthEnabled);

    return Ret;
  }

  void vulkanInterface::CreateCamera(Camera* pCam, uint32_t Binding)
  {
    pCam->Init(Device, WindowExtent, ShaderResources.Descriptor, Binding, HostMemory);
  }

  VkResult vulkanInterface::CreateImage(Ek::Texture& inTex, VkFormat Format, VkExtent2D ImageExtent, VkImageUsageFlags Usage)
  {
    VkResult Err;

    inTex.Format = Format;
    inTex.Layout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageCreateInfo ImageCI{};
    ImageCI.sType  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCI.format = Format;
    ImageCI.extent = VkExtent3D{ImageExtent.width, ImageExtent.height, 1};
    ImageCI.usage = Usage;
    ImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCI.imageType = VK_IMAGE_TYPE_2D;
    ImageCI.mipLevels = 1;
    ImageCI.arrayLayers = 1;
    ImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if((Err = vkCreateImage(Device, &ImageCI, nullptr, &inTex.Image)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::CreateImageView(VkImageView& View, Ek::Texture& Texture, VkImageAspectFlags Aspects)
  {
    VkResult Err;

    VkImageViewCreateInfo ViewCI{};
    ViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ViewCI.format = Texture.Format;
    ViewCI.image = Texture.Image;
    ViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;

    ViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
    ViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
    ViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
    ViewCI.components.a = VK_COMPONENT_SWIZZLE_A;

    ViewCI.subresourceRange.aspectMask = Aspects;
    ViewCI.subresourceRange.layerCount = 1;
    ViewCI.subresourceRange.levelCount = 1;
    ViewCI.subresourceRange.baseMipLevel = 0;
    ViewCI.subresourceRange.baseArrayLayer = 0;

    if((Err = vkCreateImageView(Device, &ViewCI, nullptr, &View)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  Ek::Wrappers::CommandBuffer vulkanInterface::GetCommandBuffer(Ek::eCommandType cmdType)
  {
    Ek::Wrappers::CommandBuffer Ret;

    VkCommandPool* Pool;
    VkQueue* Queue;

    switch(cmdType)
    {
      case Ek::eGraphics:
        Pool = &GraphicsPool;
        Queue = &GraphicsQueue;
        break;

      case Ek::eCompute:
        Pool = &ComputePool;
        Queue = &ComputeQueue;
        break;

      case Ek::eTransfer:
        Pool = &TransferPool;
        Queue = &TransferQueue;
        break;

      default:
        break;
    }

    Ret.Allocate(Device, *Queue, *Pool, cmdType);

    return Ret;
  }
/* Producers */

/* Rendering */
  void vulkanInterface::BeginRender(Ek::Wrappers::CommandBuffer& cmdBuffer)
  {
    if(AcquireFence == VK_NULL_HANDLE)
    {
      VkFenceCreateInfo FenceCI{};
      FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

      vkCreateFence(Device, &FenceCI, nullptr, &AcquireFence);
    }

    vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, VK_NULL_HANDLE, AcquireFence, &ImageIndex);
    VkRect2D Area{};
    Area.extent = WindowExtent;

    Area.offset.x = 0;
    Area.offset.y = 0;

    VkClearValue* Clears = new VkClearValue[Attachments.size()];

    for(uint32_t i = 0; i < Attachments.size(); i++)
    {
      if(Attachments[i].SubpassAttachments[0] == Ek::eDepth)
      {
        // we fill the depth stencil with 1(max depth)
        Clears[i].depthStencil.depth = 1.f;
        Clears[i].depthStencil.stencil = 1;
      }
      else
      {
        for(uint32_t x = 0; x < 4; x++)
        {
          Clears[i].color.int32[x] = 0;
          Clears[i].color.uint32[x] = 0;
          Clears[i].color.float32[x] = 0.f;
        }
      }
    }

    VkRenderPassBeginInfo BeginInfo{};
    BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    BeginInfo.renderPass = RenderPass;
    BeginInfo.renderArea = Area;
    BeginInfo.clearValueCount = Attachments.size();
    BeginInfo.pClearValues = Clears;

    vkWaitForFences(Device, 1, &AcquireFence, VK_TRUE, UINT64_MAX);
    vkResetFences(Device, 1, &AcquireFence);

    BeginInfo.framebuffer = FrameBuffers[ImageIndex];

    vkCmdBeginRenderPass(cmdBuffer.Buffer, &BeginInfo, VK_SUBPASS_CONTENTS_INLINE);
  }

  void vulkanInterface::BindShaderResources(Ek::Wrappers::CommandBuffer& cmdBuffer, PipelineInterface* Pipeline)
  {
    vkCmdBindDescriptorSets(cmdBuffer.Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->PipelineLayout, 0, 1, &ShaderResources.Descriptor, 0, nullptr);
  }

  void vulkanInterface::EndRender(Ek::Wrappers::CommandBuffer& cmdBuffer)
  {
    vkCmdEndRenderPass(cmdBuffer.Buffer);
  }

  void vulkanInterface::Present(Ek::Wrappers::CommandBuffer& cmdBuffer)
  {
    VkPresentInfoKHR PresentInfo{};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = &Swapchain;
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = &cmdBuffer.Semaphore;

    VkQueue* Queue;

    switch(cmdBuffer.cmdType)
    {
      case Ek::eGraphics:
        Queue = &GraphicsQueue;
        break;

      case Ek::eCompute:
        Queue = &ComputeQueue;
        break;

      case Ek::eTransfer:
        Queue = &TransferQueue;
        break;

      default:
        throw std::runtime_error("Failed to present: Tried to present with type-less command buffer\n");
    }

    vkQueuePresentKHR(*Queue, &PresentInfo);
  }
/* Rendering */

/* Mem ops */
  VkResult vulkanInterface::LoadImage(const char* Path, Ek::Texture& inTex, VkImageLayout Layout, VkImageUsageFlags Usage)
  {
    VkResult Err;
    sail::image imgFile(Path);

    // Load Image into GPU memory
    {

      if((Err = CreateImage(inTex, VK_FORMAT_R8G8B8A8_UNORM, VkExtent2D{imgFile.width(), imgFile.height()}, VK_IMAGE_USAGE_TRANSFER_DST_BIT | Usage)) != VK_SUCCESS)
      {
        return Err;
      }

      AllocateTexture(inTex, Ek::eLocalMemory);

      void* TransferMemory;
      TransferBuffer.Map(&TransferMemory);

      memcpy(TransferMemory, imgFile.pixels(), imgFile.pixels_size());
    }

    VkBufferImageCopy CopyInfo{};
    CopyInfo.imageExtent = {imgFile.width(), imgFile.height(), 1};
    CopyInfo.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    CopyInfo.imageSubresource.mipLevel = 0;
    CopyInfo.imageSubresource.layerCount = 1;
    CopyInfo.imageSubresource.baseArrayLayer = 0;

    {
      VkImageMemoryBarrier toDst{};
      VkImageMemoryBarrier graphicsToDesired{};
      VkImageMemoryBarrier transferToDesired{};

      toDst = inTex.Barrier(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 0);
      graphicsToDesired = inTex.Barrier(VK_IMAGE_ASPECT_COLOR_BIT, Layout, 0, VK_ACCESS_SHADER_READ_BIT);

      Ek::Wrappers::CommandBuffer cmdMemOps = GetCommandBuffer(Ek::eTransfer);
      Ek::Wrappers::CommandBuffer cmdGraphics = GetCommandBuffer(Ek::eGraphics);

      cmdMemOps.BeginCommand();
        vkCmdPipelineBarrier(cmdMemOps.Buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toDst);
        vkCmdCopyBufferToImage(cmdMemOps.Buffer, TransferBuffer.Buffer, inTex.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &CopyInfo);
      cmdMemOps.EndComand();

      cmdMemOps.FenceWait();

      cmdGraphics.BeginCommand();
        vkCmdPipelineBarrier(cmdGraphics.Buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &graphicsToDesired);
      cmdGraphics.EndComand();

      cmdMemOps.Delete();
      cmdGraphics.FenceWait();
      cmdGraphics.Delete();
    }

    return VK_SUCCESS;
  }

  void vulkanInterface::PipelineBarrier(Ek::Wrappers::CommandBuffer& cmdBuffer, uint32_t ImgCount, VkImageMemoryBarrier* ImgBarriers, VkPipelineStageFlags Src, VkPipelineStageFlags Dst)
  {
    vkCmdPipelineBarrier(cmdBuffer.Buffer, Src, Dst, 0, 0, nullptr, 0, nullptr, ImgCount, ImgBarriers);
  }

  void vulkanInterface::CopyBufferToImage(Ek::Wrappers::CommandBuffer& cmdBuffer, Ek::Buffer& src, Ek::Texture& dst, VkImageAspectFlags Aspects)
  {
    VkBufferImageCopy CopyInfo{};
    CopyInfo.imageExtent = VkExtent3D{dst.Extent.width, dst.Extent.height, 1};
    CopyInfo.imageOffset = {0, 0};

    CopyInfo.imageSubresource.aspectMask = Aspects;
    CopyInfo.imageSubresource.mipLevel = 0;
    CopyInfo.imageSubresource.layerCount = 0;
    CopyInfo.imageSubresource.baseArrayLayer = 0;

    vkCmdCopyBufferToImage(cmdBuffer.Buffer, src.Buffer, dst.Image, dst.Layout, 1, &CopyInfo);
  }

  void vulkanInterface::CopyBufferToBuffer(Ek::Wrappers::CommandBuffer& cmdBuffer, Ek::Buffer& src, Ek::Buffer& dst)
  {
    VkBufferCopy CopyInfo{};
    CopyInfo.size = src.allocSize;
    CopyInfo.srcOffset = src.allocOffset;
    CopyInfo.dstOffset = dst.allocOffset;

    vkCmdCopyBuffer(cmdBuffer.Buffer, src.Buffer, dst.Buffer, 1, &CopyInfo);
  }
/* Mem ops */

}

