#include "Interface.h"
#include "Memory.h"

#include <fstream>

#include <stdexcept>
#include <string>
#include <vector>
#include <cstring>
#include <vulkan/vulkan_core.h>

namespace Ek
{
  vulkanInterface::vulkanInterface()
  {
    Attachments.resize(1);
  }

  VkResult vulkanInterface::CreateInstance(VkExtent2D inExtent)
  {
    VkResult Err;

    WindowExtent = inExtent;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    Window = glfwCreateWindow(WindowExtent.width, WindowExtent.height, "my game", nullptr, nullptr);

    uint32_t glfwExtCount;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    for(uint32_t i = 0; i < glfwExtCount; i++)
    {
      if(!AddInstExtension(glfwExts[i]))
      {
        throw std::runtime_error("Failed to enable instance extensions for glfw window");
      }
    }

    for(uint32_t i = 0; i < InstanceExtensions.size(); i++)
    {
      std::cout << InstanceExtensions[i] << '\n';
    }

    VkInstanceCreateInfo InstanceCI{};
    InstanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    InstanceCI.enabledLayerCount = InstanceLayers.size();
    InstanceCI.ppEnabledLayerNames = InstanceLayers.data();
    InstanceCI.enabledExtensionCount = InstanceExtensions.size();
    InstanceCI.ppEnabledExtensionNames = InstanceExtensions.data();

    if((Err = vkCreateInstance(&InstanceCI, nullptr, &Instance)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::CreatePhysicalDevice()
  {
    uint32_t PDevCount;
    vkEnumeratePhysicalDevices(Instance, &PDevCount, nullptr);
    std::vector<VkPhysicalDevice> PDevices(PDevCount);
    vkEnumeratePhysicalDevices(Instance, &PDevCount, PDevices.data());

    for(uint32_t i = 0; i < PDevCount; i++)
    {
      VkPhysicalDeviceProperties Props;
      vkGetPhysicalDeviceProperties(PDevices[i], &Props);

      if(Props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      {
        PDevice = PDevices[i];
        break;
      }
    }

    if(PDevice == VK_NULL_HANDLE)
    {
      PDevice = PDevices[0];
    }

    VkPhysicalDeviceMemoryProperties MemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(PDevice, &MemoryProperties);

    HostIndex = -1;
    VRamIndex = -1;

    for(uint32_t i = 0; i < MemoryProperties.memoryTypeCount; i++)
    {
      if(MemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT && VRamIndex == -1)
      {
        VRamIndex = i;
      }
      else if(MemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && HostIndex == -1)
      {
        HostIndex = i;
      }
    }

    uint32_t FamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(PDevice, &FamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> FamilyProperties(FamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(PDevice, &FamilyCount, FamilyProperties.data());

    std::cout << "Family count: " << FamilyCount << '\n';

    GraphicsIndex = -1;
    ComputeIndex = -1;
    TransferIndex = -1;

    for(uint32_t i = 0; i < FamilyCount; i++)
    {
      if(FamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && GraphicsIndex == -1)
      {
        GraphicsIndex = i;
      }
      else if(FamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && ComputeIndex == -1)
      {
        ComputeIndex = i;
      }

      if(FamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && TransferIndex == -1)
      {
        if(GraphicsIndex == i || ComputeIndex == i)
        {
          if(FamilyProperties[i].queueCount >= 2)
          {
            TransferIndex = i;
          }
        }
      }
    }

    if(GraphicsIndex == -1 || ComputeIndex == -1 || TransferIndex == -1)
    {
      std::cout << "\nGraphics: " << GraphicsIndex << "\nCompute: " << ComputeIndex << "\nTransfer: " << TransferIndex << '\n';
      return VK_ERROR_UNKNOWN;
    }

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::CreateDevice()
  {
    VkResult Err;

    std::vector<VkDeviceQueueCreateInfo> Queues;

    if(TransferIndex == GraphicsIndex || TransferIndex == ComputeIndex)
    {
      Queues.resize(2);

      Queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      Queues[0].queueCount = (TransferIndex == GraphicsIndex) ? 2 : 1;
      Queues[0].queueFamilyIndex = GraphicsIndex;

      Queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      Queues[1].queueCount = (TransferIndex == ComputeIndex) ? 2 : 1;
      Queues[1].queueFamilyIndex = ComputeIndex;
    }

    else
    {
      Queues.resize(3);

      Queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      Queues[0].queueCount = 1;
      Queues[0].queueFamilyIndex = GraphicsIndex;

      Queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      Queues[1].queueCount = 1;
      Queues[1].queueFamilyIndex = ComputeIndex;

      Queues[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      Queues[2].queueCount = 1;
      Queues[2].queueFamilyIndex = TransferIndex;
    }

    VkDeviceCreateInfo DevCI{};
    DevCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DevCI.enabledExtensionCount = DeviceExtensions.size();
    DevCI.ppEnabledExtensionNames = DeviceExtensions.data();
    DevCI.queueCreateInfoCount = Queues.size();
    DevCI.pQueueCreateInfos = Queues.data();

    if((Err = vkCreateDevice(PDevice, &DevCI, nullptr, &Device)) != VK_SUCCESS)
    {
      return Err;
    }

    std::cout << "The graphics index: " << GraphicsIndex << "  The Compute Index: " << ComputeIndex << '\n';

    vkGetDeviceQueue(Device, GraphicsIndex, 0, &GraphicsQueue);
    vkGetDeviceQueue(Device, ComputeIndex, 0, &ComputeQueue);
    vkGetDeviceQueue(Device, TransferIndex, (TransferIndex == GraphicsIndex || TransferIndex == ComputeIndex) ? 1 : 0, &TransferQueue);
    // if the Transfer queue family is the same as graphics or compute then we use index 1 instead of 0.

    if(!HostMemory.Init(Device, HostIndex, 128000000))
    {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    HostMemory.MemType = Ek::eHostMemory;
    HostMemory.Map();

    if(!LocalMemory.Init(Device, VRamIndex, 256000000))
    {
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    LocalMemory.MemType = Ek::eLocalMemory;

    if((Err = CreateCommandPool()) != VK_SUCCESS)
    {
      return Err;
    }


    VkBufferCreateInfo TransferCI{};
    TransferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    TransferCI.size = 50000000;
    TransferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    TransferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Err = vkCreateBuffer(Device, &TransferCI, nullptr, &TransferBuffer.Buffer)) != VK_SUCCESS)
    {
      return Err;
    }

    AllocateBuffer(TransferBuffer, Ek::eHostMemory);

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::CreateCommandPool()
  {
    VkResult Err;

    // graphics
    VkCommandPoolCreateInfo GraphicsPoolCI{};
    GraphicsPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    GraphicsPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    GraphicsPoolCI.queueFamilyIndex = GraphicsIndex;

    // Compute
    VkCommandPoolCreateInfo ComputePoolCI{};
    ComputePoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ComputePoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ComputePoolCI.queueFamilyIndex = ComputeIndex;

    // Transfer
    VkCommandPoolCreateInfo TransferPoolCI{};
    TransferPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    TransferPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    TransferPoolCI.queueFamilyIndex = TransferIndex;

    if((Err = vkCreateCommandPool(Device, &GraphicsPoolCI, nullptr, &GraphicsPool)) != VK_SUCCESS)
    {
      return Err;
    }

    if((Err = vkCreateCommandPool(Device, &ComputePoolCI, nullptr, &ComputePool)) != VK_SUCCESS)
    {
      return Err;
    }

    if((Err = vkCreateCommandPool(Device, &TransferPoolCI, nullptr, &TransferPool)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::CreateSwapchain()
  {
    VkResult Err;

    if((Err = glfwCreateWindowSurface(Instance, Window, nullptr, &Surface)) != VK_SUCCESS)
    {
      return Err;
    }

    {
      uint32_t FormatCount;
      vkGetPhysicalDeviceSurfaceFormatsKHR(PDevice, Surface, &FormatCount, nullptr);
      std::vector<VkSurfaceFormatKHR> Formats(FormatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(PDevice, Surface, &FormatCount, Formats.data());

      for(uint32_t i = 0; i < FormatCount; i++)
      {
        if(Formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
          SurfaceFormat = Formats[i];
        }
      }
    }

    VkSwapchainCreateInfoKHR SwapchainCI{};
    SwapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainCI.minImageCount = 3;
    SwapchainCI.surface = Surface;
    SwapchainCI.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    SwapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapchainCI.imageFormat = SurfaceFormat.format;
    SwapchainCI.imageExtent = WindowExtent;
    SwapchainCI.imageColorSpace = SurfaceFormat.colorSpace;
    SwapchainCI.imageArrayLayers = 1;
    SwapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    SwapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    SwapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    SwapchainCI.clipped = VK_FALSE;

    if((Err = vkCreateSwapchainKHR(Device, &SwapchainCI, nullptr, &Swapchain)) != VK_SUCCESS)
    {
      return Err;
    }

    // swapchain image
    {
      uint32_t ImageCount;
      vkGetSwapchainImagesKHR(Device, Swapchain, &ImageCount, nullptr);
      std::vector<VkImage> SwapImages(ImageCount);
      vkGetSwapchainImagesKHR(Device, Swapchain, &ImageCount, SwapImages.data());

      FrameBufferCount = ImageCount;
      FrameBufferImages.resize(ImageCount);

      for(uint32_t i = 0; i < ImageCount; i++)
      {
        FrameBufferImages[i].resize(1);

        FrameBufferImages[i][0].Format = SurfaceFormat.format;
        FrameBufferImages[i][0].Image = SwapImages[i];
        FrameBufferImages[i][0].Layout = VK_IMAGE_LAYOUT_UNDEFINED;
        FrameBufferImages[i][0].Extent = WindowExtent;
        FrameBufferImages[i][0].allocSize = 0;
        FrameBufferImages[i][0].allocOffset = 0;
      }

      // this adds some meta data for the swapchain image, this will be used by the renderpass. (Usually they're also used during framebuffer creation)
      Attachments[0].Format = SurfaceFormat.format;
      Attachments[0].StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      Attachments[0].LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      Attachments[0].InitialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      Attachments[0].FinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      Attachments[0].StencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      Attachments[0].StencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

      Attachments[0].Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      Attachments[0].Aspect = VK_IMAGE_ASPECT_COLOR_BIT;

      // during the first subpass this will be the format
      Attachments[0].SubpassLayouts.push_back(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      Attachments[0].SubpassAttachments.push_back(Ek::eColor);
    }

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::CreateRenderpass(uint32_t sCount, VkPipelineBindPoint* BindPoint)
  {
    VkResult Err;

    SubpassCount = sCount;

    /* define attachment descriptions/references and iterators */
      std::vector<VkAttachmentDescription> Descriptions(0);

      // the *Sizes arrays double as an iterator in the enumeration below used to sort and create the attachment references.

      uint32_t ColorSizes[sCount];
      VkAttachmentReference ColorAtts[sCount][Attachments.size()];

      uint32_t InputSizes[sCount];
      VkAttachmentReference InputAtts[sCount][Attachments.size()];

      uint32_t PreserveSizes[sCount];
      uint32_t PreserveAtts[sCount][Attachments.size()];

      uint32_t ResolveSizes[sCount];
      VkAttachmentReference ResolveAtts[sCount][Attachments.size()];

      // the cool thing about depth stencil attachments is that there can only be one per subpass, so we don't need to keep iterators for it
      VkAttachmentReference DepthAtts[sCount];
    /* define attachment descriptions/references and iterators */

    {
      // in this enumeration, i represents the inde of the attachment, and x represents the index of the subpass
      for(uint32_t i = 0; i < Attachments.size(); i++)
      {
        Descriptions.push_back({});

        Descriptions[i].format = Attachments[i].Format;
        Descriptions[i].initialLayout = Attachments[i].InitialLayout;
        Descriptions[i].finalLayout = Attachments[i].FinalLayout;
        Descriptions[i].stencilStoreOp = Attachments[i].StencilStoreOp;
        Descriptions[i].stencilLoadOp = Attachments[i].StencilLoadOp;
        Descriptions[i].storeOp = Attachments[i].StoreOp;
        Descriptions[i].loadOp = Attachments[i].LoadOp;
        Descriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;

        ColorSizes[i] = 0;
        InputSizes[i] = 0;
        PreserveSizes[i] = 0;
        ResolveSizes[i] = 0;

        for(uint32_t x = 0; x < sCount; x++)
        {
          uint32_t ReferenceIndex = (i*sCount)+x;

          if(!Attachments[i].SubpassLayouts[x])
          {
            std::string strError("Renderpass: Fatal Error! we do not have enough layouts in the SubpassLayouts array element of attachment ");
            strError.append(std::to_string(i));
            strError.append(" in subpass (Index) " + std::to_string(x));

            throw std::runtime_error(strError);
          }

          switch (Attachments[i].SubpassAttachments[x])
          {
            case Ek::eColor:
              ColorAtts[x][ColorSizes[x]].layout = Attachments[i].SubpassLayouts[x];
              ColorAtts[x][ColorSizes[x]].attachment = i;
              ColorSizes[x]++;
              break;

            case Ek::eInput:
              InputAtts[x][InputSizes[x]].layout = Attachments[i].SubpassLayouts[x];
              InputAtts[x][InputSizes[x]].attachment = i;
              InputSizes[x]++;
              break;

            case Ek::ePreserve:
              PreserveAtts[x][PreserveSizes[x]] = i;
              PreserveSizes[x]++;
              break;

            case Ek::eResolve:
              ResolveAtts[x][ResolveSizes[x]].layout = Attachments[i].SubpassLayouts[x];
              ResolveAtts[x][ResolveSizes[x]].attachment = i;
              ResolveSizes[x]++;
              break;

            case Ek::eDepth:
              DepthAtts[x].layout = Attachments[i].SubpassLayouts[x];
              DepthAtts[x].attachment = i;
              break;

            default:
              std::string strError("can't determine attachment ");
              strError.append(std::to_string(i));
              strError.append(" at subpass ");
              strError.append(std::to_string(x));
              strError.append(". Which has Attachment Type of ");
              strError.append(std::to_string(Attachments[i].SubpassAttachments[x]));

              throw std::runtime_error(strError);
          }
        }
      }
    }

    std::vector<VkSubpassDescription> Subpasses(sCount);

    // Check for depth testing
    bool DepthEnabled = false;

    for(uint32_t i = 0; i < Attachments.size(); i++)
    {
      if(Attachments[i].Usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      {
        DepthEnabled = true;
      }
    }

    // fill subpass data
    {
      for(uint32_t i = 0; i < sCount; i++)
      {
        Subpasses[i].pipelineBindPoint = BindPoint[i];

        Subpasses[i].colorAttachmentCount = ColorSizes[i];
        Subpasses[i].pColorAttachments = ColorAtts[i];

        Subpasses[i].inputAttachmentCount = InputSizes[i];
        Subpasses[i].pInputAttachments = InputAtts[i];

        Subpasses[i].preserveAttachmentCount = PreserveSizes[i];
        Subpasses[i].pPreserveAttachments = PreserveAtts[i];

        Subpasses[i].pResolveAttachments = (ResolveSizes[0] == 0) ? nullptr : ResolveAtts[i];

        Subpasses[i].pDepthStencilAttachment = &DepthAtts[i];
      }
    }

    // create renderpass
    {
      VkRenderPassCreateInfo RenderpassCI{};
      RenderpassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      RenderpassCI.subpassCount = Subpasses.size();
      RenderpassCI.pSubpasses = Subpasses.data();
      RenderpassCI.attachmentCount = Descriptions.size();
      RenderpassCI.pAttachments = Descriptions.data();

      std::cout << Attachments.size() << '\n';

      if((Err = vkCreateRenderPass(Device, &RenderpassCI, nullptr, &RenderPass)) != VK_SUCCESS)
      {
        return Err;
      }
    }

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::CreateFrameBuffers()
  {
    VkResult Err;

    // create framebufers/views
    {
      // we need an extra view array for the swapchain images
      FrameBuffers.resize(FrameBufferCount);
      FrameBufferImages.resize(FrameBufferCount);
      FrameBufferViews.resize(FrameBufferCount);

      for(uint32_t i = 0; i < FrameBufferCount; i++)
      {
        FrameBufferImages[i].resize(Attachments.size());
        FrameBufferViews[i].resize(Attachments.size());
      }

      for(uint32_t i = 0; i < FrameBufferCount; i++)
      {
        // x starts at 1 so we can ignore the swapchain image. (it's already created with the swapchain)
        for(uint32_t x = 1; x < Attachments.size(); x++)
        {
          CreateImage(FrameBufferImages[i][x], Attachments[x].Format, WindowExtent, Attachments[x].Usage);

          if(!LocalMemory.AllocateTexture(FrameBufferImages[i][x]))
          {
            throw std::runtime_error("failed to allocate texture for frame buffer\n");
          }
        }

        for(uint32_t x = 0; x < Attachments.size(); x++)
        {
          CreateImageView(FrameBufferViews[i][x], FrameBufferImages[i][x], Attachments[x].Aspect);
        }

        VkFramebufferCreateInfo FrameBufferCI{};
        FrameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FrameBufferCI.width = WindowExtent.width;
        FrameBufferCI.height = WindowExtent.height;
        FrameBufferCI.layers = 1;
        FrameBufferCI.renderPass = RenderPass;
        FrameBufferCI.attachmentCount = FrameBufferViews[i].size();
        FrameBufferCI.pAttachments = FrameBufferViews[i].data();

        if((Err = vkCreateFramebuffer(Device, &FrameBufferCI, nullptr, &FrameBuffers[i])) != VK_SUCCESS)
        {
          return Err;
        }
      }
    }

    // Set the layouts for the attachments
    {
      std::vector<VkImageMemoryBarrier> Barriers;

      for(uint32_t i = 0; i < FrameBufferCount; i++)
      {
        for(uint32_t x = 0; x < Attachments.size(); x++)
        {
          // +1 to ignore the Swapchain image attachment
          Barriers.push_back(FrameBufferImages[i][x].Barrier(Attachments[x].Aspect, Attachments[x].InitialLayout, 0, 0));
        }
      }

      Ek::Wrappers::CommandBuffer cmdMemory = GetCommandBuffer(Ek::eGraphics);

      cmdMemory.BeginCommand();
        PipelineBarrier(cmdMemory, Barriers.size(), Barriers.data(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
      cmdMemory.EndComand();

      cmdMemory.FenceWait();
      cmdMemory.Delete();
    }

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::AddDescriptorBinding(VkDescriptorSetLayoutBinding* pBindings, uint32_t BindingCount)
  {
    VkResult Err;

    for(uint32_t i = 0; i < BindingCount; i++)
    {
      VkDescriptorPoolSize Size{};
      Size.type = pBindings[i].descriptorType;
      Size.descriptorCount = pBindings[i].descriptorCount;

      Sizes.push_back(Size);

      Bindings.push_back(pBindings[i]);
    }

    return VK_SUCCESS;
  }

  VkResult vulkanInterface::CreateDescriptors()
  {
    VkResult Err;

    VkDescriptorPoolCreateInfo PoolCI{};
    PoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolCI.poolSizeCount = Sizes.size();
    PoolCI.pPoolSizes = Sizes.data();
    PoolCI.maxSets = 1;

    if((Err = vkCreateDescriptorPool(Device, &PoolCI, nullptr, &DescPool)) != VK_SUCCESS)
    {
      return Err;
    }

    VkDescriptorSetLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCI.bindingCount = Bindings.size();
    LayoutCI.pBindings = Bindings.data();

    if((Err = vkCreateDescriptorSetLayout(Device, &LayoutCI, nullptr, &ShaderResources.DescriptorLayout)) != VK_SUCCESS)
    {
      return Err;
    }

    VkDescriptorSetAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = DescPool;
    AllocInfo.descriptorSetCount = 1;
    AllocInfo.pSetLayouts = &ShaderResources.DescriptorLayout;

    if((Err = vkAllocateDescriptorSets(Device, &AllocInfo, &ShaderResources.Descriptor)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  void vulkanInterface::Destroy()
  {
    for(uint32_t i = 0; i < FrameBuffers.size(); i++)
    {
      vkDestroyFramebuffer(Device, FrameBuffers[i], nullptr);
    }

    for(uint32_t i = 0; i < FrameBufferViews.size(); i++)
    {
      for(uint32_t x = 0; x < FrameBufferViews[i].size(); x++)
      {
        vkDestroyImageView(Device, FrameBufferViews[i][x], nullptr);
      }
    }

    for(uint32_t i = 0; i < FrameBufferImages.size(); i++)
    {
      // start at 1 to ignore the swapchain image
      for(uint32_t x = 1; x < FrameBufferImages[i].size(); x++)
      {
        FrameBufferImages[i][x].Destroy();
      }
    }

    if(DescPool != VK_NULL_HANDLE)
    {
      vkDestroyDescriptorSetLayout(Device, ShaderResources.DescriptorLayout, nullptr);
      vkDestroyDescriptorPool(Device, DescPool, nullptr);
    }

    vkDestroyRenderPass(Device, RenderPass, nullptr);

    vkDestroySwapchainKHR(Device, Swapchain, nullptr);

    if(AcquireFence != VK_NULL_HANDLE)
    {
      vkDestroyFence(Device, AcquireFence, nullptr);
    }

    vkDestroyCommandPool(Device, GraphicsPool, nullptr);
    vkDestroyCommandPool(Device, ComputePool, nullptr);
    vkDestroyCommandPool(Device, TransferPool, nullptr);

    TransferBuffer.Destroy();
    HostMemory.Destroy();
    LocalMemory.Destroy();

    glfwDestroyWindow(Window);
    vkDestroySurfaceKHR(Instance, Surface, nullptr);

    vkDeviceWaitIdle(Device);

    vkDestroyDevice(Device, nullptr);

    vkDestroyInstance(Instance, nullptr);
  }
}

