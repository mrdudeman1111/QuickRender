#pragma once

#include <assimp/Importer.hpp>
#include <assimp/config.h>

#include <cwchar>
#include <vector>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Memory.h"

/*
 * defined in this file:
 *  eAttachmentType
 *  eCommandType
 *  CommandBuffer
 *  FrameBufferAttachment
 *  DescriptorSet
*/

namespace Ek
{
  enum eAttachmentType
  {
    eColor = 1,
    eInput = 2,
    ePreserve = 3,
    eResolve = 4,
    eDepth = 5
  };

  enum eCommandType
  {
    eGraphics = 0,
    eCompute = 1,
    eTransfer = 2
  };

  namespace Wrappers
  {
    struct CommandBuffer
    {
      public:
        VkFence Fence;
        VkCommandBuffer Buffer;
        VkSemaphore Semaphore;

        eCommandType cmdType;

        VkResult Allocate(VkDevice& Device, VkQueue& inQueue, VkCommandPool& Pool, eCommandType inType);
        void Delete();

        void BeginCommand(VkSemaphore* pSemaphore = nullptr);
        void EndComand(bool bSignalSem = false);
        void FenceWait();

      private:
        VkDevice* pDevice;
        VkQueue* pQueue;
        VkCommandPool* pPool;

        VkSemaphore* waitSemaphore;
    };

    struct FrameBufferAttachment
    {
      public:
        VkImageUsageFlags Usage;
        VkImageAspectFlags Aspect;

        VkFormat Format;

        VkImageLayout InitialLayout;

        // there should be one element for every Subpass in this vector
        std::vector<VkImageLayout> SubpassLayouts;
        std::vector<eAttachmentType> SubpassAttachments;

        VkImageLayout FinalLayout;

        VkAttachmentLoadOp LoadOp;
        VkAttachmentStoreOp StoreOp;
        VkAttachmentLoadOp StencilLoadOp;
        VkAttachmentStoreOp StencilStoreOp;
    };
  }
}

namespace EkBackend
{
  class DescriptorSet
  {
    public:
      VkPipelineLayout PipeLayout;
      VkDescriptorSetLayout DescriptorLayout;
      VkDescriptorSet Descriptor;
  };
}

