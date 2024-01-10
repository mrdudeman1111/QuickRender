#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace Ek
{
  namespace Shaders
  {
    struct ShaderResourceData
    {
      public:
        std::vector<VkDescriptorImageInfo> ImageInfos;
        std::vector<VkDescriptorBufferInfo> BufferInfos;

        VkDescriptorSet* pSet;
        VkDescriptorType Type;
        uint32_t Binding;
        uint32_t Element;
    };

    class ShaderResource
    {
      public:
        ShaderResource(ShaderResourceData inData);
        ~ShaderResource();

        void Update();

      private:
        VkDevice* pDevice;

        VkWriteDescriptorSet Write;

        ShaderResourceData Data;
    };
  }
}

namespace EkBackend
{
  namespace Shaders
  {
    class DescriptorSet
    {
      public:
        VkPipelineLayout PipeLayout;
        VkDescriptorSetLayout DescriptorLayout;
        VkDescriptorSet Descriptor;
    };
  }
}

