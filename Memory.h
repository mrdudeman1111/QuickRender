#pragma once

#include <vector>
#include <queue>

#include <vulkan/vulkan.h>

/* Forward declarations */
namespace EkBackend
{
  class MemHeader;
  class MemoryBlock;
  class AllocateInterface;
}

namespace Ek
{
  class AllocatedObject;
  class Texture;
  class Buffer;
}


/* Full declarations */
namespace Ek
{
  enum eMemoryType
  {
    eHostMemory = 0,
    eLocalMemory = 1
  };

  class AllocatedObject
  {
    friend EkBackend::MemoryBlock;

    public:
      uint32_t allocSize;
      uint32_t allocOffset;
      eMemoryType allocMemoryType;

      virtual void Destroy() = 0;
      void Map(void** Pointer);

    protected:
      VkDevice* pDevice;
      VkDeviceMemory allocMemory;

      void Delete();

    private:
      EkBackend::MemoryBlock* pAllocator;
      uint32_t AllocationID;
  };

  class Texture : public AllocatedObject
  {
    public:
      VkImageMemoryBarrier Barrier(VkImageAspectFlags Aspect, VkImageLayout NewLayout, VkAccessFlags src, VkAccessFlags dst);
      void Destroy();

      // Handles
        VkImage Image;

      // Image data
        VkExtent2D Extent;
        VkFormat Format;
        VkImageLayout Layout;
  };

  class Buffer : public AllocatedObject
  {
    public:
      void Destroy();

      // Handles
        VkBuffer Buffer;
  };
}

namespace EkBackend
{
  struct MemHeader
  {
    uint32_t MemorySize;
    uint32_t Start;

    uint32_t ID = -1;

    Ek::AllocatedObject* pObject;

    MemHeader* Next = nullptr;
  };

  class MemoryBlock
  {
    public:
      bool Init(VkDevice& inDevice, uint32_t inMemoryIndex, uint32_t DesiredSize);
      void Destroy();

      void Map();
      void* GetMemory(uint32_t Offset);
      void unMap();

      bool AllocateBuffer(Ek::Buffer& inBuffer);
      bool AllocateTexture(Ek::Texture& inTexture);
      void Delete(uint32_t AllocId);

      uint32_t AssessFrag();

      Ek::eMemoryType MemType;
      bool Mapped;

    private:
      VkDevice* pDevice;
      uint32_t MemoryIndex;
      VkDeviceMemory Allocation;
      uint32_t Size;

      void* Memory;

      uint32_t IdIndex = 1;

      MemHeader* Header = nullptr;
  };

  class AllocateInterface
  {
    public:
      virtual VkResult LoadImage(const char* Path, Ek::Texture& inTex, VkImageLayout Layout, VkImageUsageFlags Usage) = 0;
      virtual VkResult CreateImage(Ek::Texture& inTex, VkFormat Format, VkExtent2D ImageExtent, VkImageUsageFlags Usage) = 0;
      virtual VkResult CreateImageView(VkImageView& View, Ek::Texture& Texture, VkImageAspectFlags Aspects) = 0;
      virtual void AllocateBuffer(Ek::Buffer& inBuffer, Ek::eMemoryType MemType) = 0;
      virtual void AllocateTexture(Ek::Texture& inTexture, Ek::eMemoryType MemType) = 0;
  };
}

