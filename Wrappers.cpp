#include "Wrappers.h"

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/projection.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Ek
{
  namespace Wrappers
  {
    VkResult CommandBuffer::Allocate(VkDevice& Device, VkQueue& inQueue, VkCommandPool& Pool, eCommandType inType)
    {
      VkResult Err;

      pDevice = &Device;
      pQueue = &inQueue;
      pPool = &Pool;
      cmdType = inType;

      VkCommandBufferAllocateInfo AllocInfo{};
      AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      AllocInfo.commandPool = Pool;
      AllocInfo.commandBufferCount = 1;

      if((Err = vkAllocateCommandBuffers(Device, &AllocInfo, &Buffer)) != VK_SUCCESS)
      {
        return Err;
      }

      VkFenceCreateInfo FenceCI{};
      FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

      if((Err = vkCreateFence(Device, &FenceCI, nullptr, &Fence)) != VK_SUCCESS)
      {
        return Err;
      }

      VkSemaphoreCreateInfo SemaphoreCI{};
      SemaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      if((Err = vkCreateSemaphore(Device, &SemaphoreCI, nullptr, &Semaphore)) != VK_SUCCESS)
      {
        return Err;
      }

      return VK_SUCCESS;
    }

    void CommandBuffer::Delete()
    {
      vkDestroyFence(*pDevice, Fence, nullptr);
      vkDestroySemaphore(*pDevice, Semaphore, nullptr);
      vkFreeCommandBuffers(*pDevice, *pPool, 1, &Buffer);

      pDevice = nullptr;
      pQueue = nullptr;
      pPool = nullptr;
    }

    void CommandBuffer::BeginCommand(VkSemaphore* pSemaphore)
    {
      VkCommandBufferBeginInfo BeginInfo{};
      BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

      vkBeginCommandBuffer(Buffer, &BeginInfo);

      waitSemaphore = (pSemaphore == nullptr) ? nullptr : pSemaphore;
    }

    void CommandBuffer::EndComand(bool bSignalSem)
    {
      vkEndCommandBuffer(Buffer);

      VkSubmitInfo SubmitInfo{};
      SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      SubmitInfo.commandBufferCount = 1;
      SubmitInfo.pCommandBuffers = &Buffer;

      if(waitSemaphore != nullptr)
      {
        SubmitInfo.waitSemaphoreCount = 1;
        SubmitInfo.pWaitSemaphores = waitSemaphore;
      }
      if(bSignalSem)
      {
        SubmitInfo.signalSemaphoreCount = 1;
        SubmitInfo.pSignalSemaphores = &Semaphore;
      }

      vkQueueSubmit(*pQueue, 1, &SubmitInfo, Fence);
    }

    void CommandBuffer::FenceWait()
    {
      vkWaitForFences(*pDevice, 1, &Fence, VK_TRUE, UINT64_MAX);
      vkResetFences(*pDevice, 1, &Fence);

      vkResetCommandBuffer(Buffer, 0);
    }
  }
}

