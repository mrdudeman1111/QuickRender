#include "ShaderResources.h"

#include "Wrappers.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Ek
{
  namespace Shaders
  {
    ShaderResource::ShaderResource(ShaderResourceData inData)
    {
      Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      Write.dstSet = *Data.pSet;
      Write.dstBinding = Data.Binding;
      Write.dstArrayElement = Data.Element;
      Write.descriptorType = Data.Type;

      Write.descriptorCount = Data.ImageInfos.size()+Data.BufferInfos.size();
      Write.pImageInfo = Data.ImageInfos.data();
      Write.pBufferInfo = Data.BufferInfos.data();
    }

    ShaderResource::~ShaderResource()
    {}

    void ShaderResource::Update()
    {
      vkUpdateDescriptorSets(*pDevice, 1, &Write, 0, nullptr);
    }
  }
}

