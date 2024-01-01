#include <fstream>
#include <vulkan/vulkan_core.h>

#include "Interface.h"

#include <sail-c++/sail-c++.h>
#include <sail-manip/convert.h>

namespace Ek
{
  Material::Material(VkDevice* inDevice) : pDevice(inDevice)
  {}

  VkResult Material::LoadVertex(std::string Path)
  {
    VkResult Err;

    std::ifstream File(Path, std::ifstream::binary);

    File.seekg(0, std::ifstream::end);
    uint32_t FileSize = File.tellg();

    uint32_t* ShaderCode = new uint32_t[FileSize];

    File.seekg(0, std::ifstream::beg);
    File.read((char*)ShaderCode, FileSize);

    VkShaderModuleCreateInfo ModuleCI{};
    ModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ModuleCI.codeSize = FileSize;
    ModuleCI.pCode = ShaderCode;

    if((Err = vkCreateShaderModule(*pDevice, &ModuleCI, nullptr, &Vertex)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  VkResult Material::LoadFragment(std::string Path)
  {
    VkResult Err;

    std::ifstream File(Path, std::ifstream::binary);

    File.seekg(0, std::ifstream::end);
    uint32_t FileSize = File.tellg();

    char* ShaderCode = new char[FileSize];

    File.seekg(0, std::ifstream::beg);
    File.read(ShaderCode, FileSize);

    VkShaderModuleCreateInfo ModuleCI{};
    ModuleCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ModuleCI.codeSize = FileSize;
    ModuleCI.pCode = reinterpret_cast<uint32_t*>(ShaderCode);

    if((Err = vkCreateShaderModule(*pDevice, &ModuleCI, nullptr, &Fragment)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  void Material::AddBinding(VkDescriptorSetLayoutBinding Binding)
  {
    Bindings.push_back(Binding);
  }

  void Material::AddPushConstant(VkPushConstantRange Range)
  {
    Constants.push_back(Range);
  }

  VkDescriptorSetLayout* Material::GetDescriptorLayout()
  {
    VkDescriptorSetLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCI.bindingCount = Bindings.size();
    LayoutCI.pBindings = Bindings.data();

    if(vkCreateDescriptorSetLayout(*pDevice, &LayoutCI, nullptr, &Layout) != VK_SUCCESS)
    {
      std::cout << "failed to create descriptor set layout\n";
      return nullptr;
    }

    return &Layout;
  }

  void Material::Destroy()
  {
    vkDestroyShaderModule(*pDevice, Vertex, nullptr);
    vkDestroyShaderModule(*pDevice, Fragment, nullptr);
    pDevice = nullptr;
  }
}

