#include "Interface.h"
#include "Wrappers.h"
#include "ShaderResources.h"

#include <glm/matrix.hpp>

#include <vulkan/vulkan.h>

namespace Ek
{
  Camera::Camera()
  {
    MVP.World = glm::mat4(1.f);

    MVP.View = glm::mat4(1.f);

    MVP.NormalMatrix = glm::mat4(1.f);

    CameraMat = glm::mat4(1.f);

    Rotation = glm::vec3(0.f);

    Position = glm::vec3(0.f);
  }

  VkResult Camera::Init(VkDevice& inDevice, VkExtent2D CameraSize, VkDescriptorSet& inShaderDescriptor, uint32_t Binding, EkBackend::MemoryBlock& Memory)
  {
    VkResult Err;

    Shaders::ShaderResourceData wvpData;
    Shaders::ShaderResourceData posData;

    // Setup references
    {
      pDevice = &inDevice;

      wvpData.pSet = &inShaderDescriptor;
      wvpData.Binding = Binding;
      wvpData.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      wvpData.Element = 0;

      posData.pSet = &inShaderDescriptor;
      posData.Binding = Binding;
      posData.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      posData.Element = 0;

      Width = CameraSize.width;
      Height = CameraSize.height;
    }

    // allocate camera buffer
    {
      VkBufferCreateInfo BufferCI{};
      BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      BufferCI.size = sizeof(MVP);
      BufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if((Err = vkCreateBuffer(inDevice, &BufferCI, nullptr, &CameraBuffer.Buffer)) != VK_SUCCESS)
      {
        return Err;
      }

      Memory.AllocateBuffer(CameraBuffer);
    }

    // Set update structures
    {
      wvpData.BufferInfos.resize(1);

      wvpData.BufferInfos[0].buffer = CameraBuffer.Buffer;
      wvpData.BufferInfos[0].offset = 0; // this is relative to the start of the buffer, not the start of the allocation
      wvpData.BufferInfos[0].range = sizeof(glm::mat4)*4;

      posData.BufferInfos.resize(1);

      posData.BufferInfos[0].buffer = CameraBuffer.Buffer;
      posData.BufferInfos[0].offset = sizeof(glm::mat4)*4;
      posData.BufferInfos[0].range = sizeof(glm::vec3);
    }

    wvpResource = new Shaders::ShaderResource(wvpData);
    posResource = new Shaders::ShaderResource(posData);
   
    CameraMat = glm::mat4(1.f);
    Position = glm::vec3(0.f, 0.f, 5.f);
    Rotation = glm::vec3(0.f, 0.f, 0.f);

    CameraBuffer.Map(&BufferMemory);

    return VK_SUCCESS;
  }

  void Camera::Destroy()
  {
    CameraBuffer.Destroy();
  }

  void Camera::camMove(glm::vec3 Dir)
  {
    glm::vec3 RightDir(CameraMat[0][0], CameraMat[0][1], CameraMat[0][2]);
    glm::vec3 UpDir(CameraMat[1][0], CameraMat[1][1], CameraMat[1][2]);

    glm::vec3 ForwardDir(CameraMat[2][0], CameraMat[2][1], CameraMat[2][2]);
    ForwardDir *= -1.f;

    glm::vec3 Final(0.f);
    Final += (Dir.x*RightDir);
    Final += (Dir.y*UpDir);
    Final += (Dir.z*ForwardDir);

    if(glm::length(Final) > 1.f)
    {
      Final = glm::normalize(Final);
    }

    Position += Final;

    return;
  }

  void Camera::camLook(glm::vec2 Rot)
  {
    Rotation -= Rot;

    Rotation.y = std::clamp(Rotation.y, -1.57f, 1.57f);
  }

  void Camera::camUpdate()
  {
    MVP.World = glm::mat4(1.f);

      CameraMat = glm::translate(glm::mat4(1.f), Position);

      glm::vec3 UpAxis(0.f, 1.f, 0.f);
      glm::vec3 RightAxis(1.f, 0.f, 0.f);

      CameraMat = glm::rotate(CameraMat, Rotation.x, UpAxis);
      CameraMat = glm::rotate(CameraMat, Rotation.y, RightAxis);

    MVP.View = glm::inverse(CameraMat);

    MVP.Projection = glm::perspective(glm::radians(70.f), (float)Width/(float)Height, 0.1f, 1000.f);
    MVP.Projection[1][1] *= -1;

    MVP.NormalMatrix = glm::transpose(glm::inverse(MVP.World));

    MVP.Position = Position;

    memcpy(BufferMemory, &MVP, sizeof(MVP));

    wvpResource->Update();
    posResource->Update();
  }
}

