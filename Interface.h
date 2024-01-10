#pragma once

#include <cwchar>
#include <iostream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/projection.hpp>

#include "Mesh.h"
#include "AssetMan.h"
#include "ShaderResources.h"

namespace Ek
{
  namespace Query
  {
    enum eColorSpaceBits
    {
      eRGBA = 1,
      eABGR = 2,
      eRGB  = 4,
      eBGR  = 8,
      eRG   = 16,
    };

    enum eDataTypeBits
    {
      uNorm   = 1,
      sNorm   = 2,
      uInt    = 4,
      sInt    = 8,
      uScaled = 16,
      sScaled = 32,
      Packed = 64
    };

    typedef uint32_t ColorSpace;
    typedef uint32_t DataType;

    VkFormat GetBestFormat(VkPhysicalDevice& PDevice, VkImageUsageFlags Usage, ColorSpace DesiredColorSpace, DataType DatType);
  }
}

/*
 In this file we define:
 |
 * - Material
 |
 * - PipelineInterface
 |
 * - vulkanInterface
*/
// Note: we have one big descriptor that points to all our resources, aka. an Image aray, MVP buffers, structures that we want to push, etc


namespace Ek
{
  /* Implemenation in MaterialInterface.cpp */
  class Material
  {
    public:
      Material(VkDevice* inDevice);

      VkResult LoadVertex(std::string VertPath);
      VkResult LoadFragment(std::string FragPath);

      void AddBinding(VkDescriptorSetLayoutBinding Binding);
      void AddPushConstant(VkPushConstantRange Range);

      VkDescriptorSetLayout GetDescriptorLayout();

      void Destroy();

      VkShaderModule Vertex;
      VkShaderModule Fragment;

      std::vector<VkPushConstantRange> Constants;

    private:
      VkDevice* pDevice;

      std::vector<VkDescriptorSetLayoutBinding> Bindings;
      VkDescriptorSetLayout Layout;
  };

  /* Implementation in PipelineInterface.cpp */
  class PipelineInterface
  {
    public:
      PipelineInterface();
      ~PipelineInterface();

      VkPipelineLayout PipelineLayout;
      VkPipeline Pipeline;

      Material* pipeMaterial;

      void Bind(Ek::Wrappers::CommandBuffer& cmdBuffer);
      void SetDescriptorLayout(VkDescriptorSetLayout DescLayout);
      VkResult Init(VkDevice& Device, Material& Mat, Ek::Wrappers::FrameBufferAttachment* FrameBufferAttachments, uint32_t FrameBufferAttachmentCount, VkOffset2D Offset, VkExtent2D Resolution, VkRenderPass& RenderPass, uint32_t Subpass, bool bDepthEnable);

    private:
      VkDescriptorSetLayout DescriptorLayout;

      VkDevice* pDevice;
      VkRenderPass* pRenderPass;
  };

  /* Implementation in Camera.cpp */
  class Camera
  {
    friend class vulkanInterface;
    public:
      Camera();

      void Destroy();

      struct 
      {
        glm::mat4 World;
        glm::mat4 View;
        glm::mat4 Projection;
        glm::mat4 NormalMatrix;
        glm::vec3 Position;
      } MVP;

      glm::mat4 CameraMat;

      void camMove(glm::vec3 Dir);
      void camLook(glm::vec2 Rot);

      void camUpdate();

    protected:
      VkResult Init(VkDevice& Device, VkExtent2D CameraSize, VkDescriptorSet& ShaderDescriptor, uint32_t Binding, EkBackend::MemoryBlock& Memory);

      VkDevice* pDevice;

      uint32_t CameraBinding;
      void* BufferMemory;

      Shaders::ShaderResource* wvpResource;
      Shaders::ShaderResource* posResource;

      Ek::Buffer CameraBuffer;

      uint32_t Width;
      uint32_t Height;

      glm::vec3 Position;
      glm::vec2 Rotation;
  };
  /* implemetation in Camera.cpp */


  /* Implemenation Interface in Interface.cpp  Helpers.cpp */
  class vulkanInterface : public EkBackend::AllocateInterface
  {
    private:
      /* Implementation in Interface.cpp */
        VkResult CreateCommandPool();
      /* Implementation in Interface.cpp */

    public:
      /* Allocator */
        /* Implementation in Helpers */
          VkResult LoadImage(const char* Path, Ek::Texture& inTex, VkImageLayout Layout, VkImageUsageFlags Usage);
          VkResult CreateImage(Ek::Texture& inTex, VkFormat Format, VkExtent2D ImageExtent, VkImageUsageFlags Usage);
          VkResult CreateImageView(VkImageView& View, Ek::Texture& Texture, VkImageAspectFlags Aspects);
          void AllocateBuffer(Ek::Buffer& inBuff, Ek::eMemoryType MemType);
          void AllocateTexture(Ek::Texture& inTexture, Ek::eMemoryType MemType);
        /* Implementation in Helpers */
      /* Allocator */


      const bool ShouldClose() { return glfwWindowShouldClose(Window); }

      /* Implementation in Helpers.cpp */
        bool AddInstLayer(const char* pLayer);
        bool AddInstExtension(const char* pExtension);
        bool AddDevExtension(const char* pExtension);
        const void AddAttachment(Ek::Wrappers::FrameBufferAttachment Attachment);
        VkResult AddDescriptorBinding(VkDescriptorSetLayoutBinding* pBindings, uint32_t BindingCount);

        Material CreateMaterial();
        Mesh* CreateMesh(const char* MeshPath);
        PipelineInterface* CreatePipeline(Material& Mat, VkOffset2D PipeOffset, VkExtent2D PipeExtent, bool bDepthEnable);
        void CreateCamera(Camera* pCam, uint32_t Binding);

        Ek::Wrappers::CommandBuffer GetCommandBuffer(Ek::eCommandType cmdType);

        void PipelineBarrier(Ek::Wrappers::CommandBuffer& cmdBuffer, uint32_t ImgCount, VkImageMemoryBarrier* ImgBarriers, VkPipelineStageFlags Src, VkPipelineStageFlags Dst);
        void CopyBufferToImage(Ek::Wrappers::CommandBuffer& cmdBuffer, Ek::Buffer& src, Ek::Texture& dst, VkImageAspectFlags Aspects);
        void CopyBufferToBuffer(Ek::Wrappers::CommandBuffer& cmdBuffer, Ek::Buffer& src, Ek::Buffer& dst);
      /* Implementation in Helpers.cpp */

      /* Implementation in Interface.cpp */
        vulkanInterface();

        VkResult CreateInstance(VkExtent2D inExtent);
        VkResult CreatePhysicalDevice();
        VkResult CreateDevice();
        VkResult CreateSwapchain();
        VkResult CreateRenderpass(uint32_t sCount, VkPipelineBindPoint* BindPoint);
        VkResult CreateFrameBuffers();
        VkResult CreateDescriptors();

        void Destroy();
      /* Implementation in Interface.cpp */

      /* Implementation in Helpers.cpp */
        void BeginRender(Ek::Wrappers::CommandBuffer& cmdBuffer);
        void BindShaderResources(Ek::Wrappers::CommandBuffer& cmdBuffer, PipelineInterface* Pipeline);
        void EndRender(Ek::Wrappers::CommandBuffer& cmdBuffer);
        void Present(Ek::Wrappers::CommandBuffer& cmdBuffer);
      /* Implementation in Helpers.cpp */


    public:
      // Window
        VkExtent2D WindowExtent;

        GLFWwindow* Window;

        VkInstance Instance;
        VkPhysicalDevice PDevice;
    private:

        VkSurfaceKHR Surface;

      // Instance
        std::vector<VkLayerProperties> InstLayerProperties;
        std::vector<VkExtensionProperties> InstExtensionProperties;
        std::vector<const char*> InstanceLayers;
        std::vector<const char*> InstanceExtensions;

      // Physical Device
        uint32_t GraphicsIndex;
        uint32_t ComputeIndex;
        uint32_t TransferIndex;
        uint32_t VRamIndex;
        uint32_t HostIndex;
        std::vector<VkExtensionProperties> DevExtensionProperties;
        std::vector<const char*> DeviceExtensions;

      // Device
        VkQueue GraphicsQueue = VK_NULL_HANDLE;
        VkQueue ComputeQueue = VK_NULL_HANDLE;
        VkQueue TransferQueue = VK_NULL_HANDLE;
        VkDevice Device;

        VkCommandPool ComputePool = VK_NULL_HANDLE;
        VkCommandPool GraphicsPool = VK_NULL_HANDLE;
        VkCommandPool TransferPool = VK_NULL_HANDLE;

      // Swapchain
        VkSurfaceFormatKHR SurfaceFormat;
        VkSwapchainKHR Swapchain = VK_NULL_HANDLE;

        std::vector<Ek::Wrappers::FrameBufferAttachment> Attachments;

        uint32_t FrameBufferCount;

        // Array of size FrameBufferCount, Internal arrays will be of size Attachments.size()
        std::vector<std::vector<Ek::Texture>> FrameBufferImages;
        // Array of size FrameBufferCount, Internal Arrays will be of size Atttachments.size() +1 to account for the swapchain Image view
        std::vector<std::vector<VkImageView>> FrameBufferViews;
        std::vector<VkFramebuffer> FrameBuffers;

      // renderpass stuff
        uint32_t SubpassCount;
        VkRenderPass RenderPass = VK_NULL_HANDLE;

      // Shader resource
        VkDescriptorPool DescPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorPoolSize> Sizes;
        std::vector<VkDescriptorSetLayoutBinding> Bindings;

        EkBackend::DescriptorSet ShaderResources;

      // Memory
        EkBackend::MemoryBlock HostMemory;
        EkBackend::MemoryBlock LocalMemory;
        Ek::Buffer TransferBuffer;

      // Render tools
        uint32_t ImageIndex;
        VkFence AcquireFence = VK_NULL_HANDLE;
  };
}

