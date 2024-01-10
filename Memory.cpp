#include <iostream>
#include <cmath>
#include <string>
#include <vulkan/vulkan_core.h>

#include "Memory.h"

using namespace std;

namespace EkBackend
{
  bool MemoryBlock::Init(VkDevice& inDevice, uint32_t inMemoryIndex, uint32_t DesiredSize)
  {
    VkResult Err;

    pDevice = &inDevice;
    MemoryIndex = inMemoryIndex;

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = DesiredSize;
    AllocInfo.memoryTypeIndex = MemoryIndex;

    if((Err = vkAllocateMemory(inDevice, &AllocInfo, nullptr, &Allocation)) != VK_SUCCESS)
    {
      cout << "failed to allocate memory block with " << to_string(Err) << '\n';
      return false;
    }

    Size = DesiredSize;

    return true;
  }

  void MemoryBlock::Destroy()
  {
    if(Header != nullptr)
    {
      MemHeader* Next = Header;
      MemHeader* Current = Header;

      while(Current != nullptr)
      {
        if(Current->Next == nullptr)
        {
          Current->pObject->Destroy();
          break;
        }
        else
        {
          Next = Current->Next;
          Current->pObject->Destroy(); // Destroy function destroys object and it's memheader
          Current = Next;
          break;
        }
      }
    }

    vkFreeMemory(*pDevice, Allocation, nullptr);
  }

  void MemoryBlock::Map()
  {
    VkResult Err;

    if((Err = vkMapMemory(*pDevice, Allocation, 0, Size, 0, &Memory)) != VK_SUCCESS)
    {
      std::cout << "Mapping memory but failed with " << std::to_string(Err) << '\n';
    }

    Mapped = true;
  }

  void* MemoryBlock::GetMemory(uint32_t Offset)
  {
    if(Mapped)
    {
      // pointer arithmetic, we add the offset to the address of the memory to move forward in memory by (offset)bytes
      return ((byte*)Memory)+Offset;
    }
    else
    {
      cout << "Something has attempted to retrieve a memory pointer from Memory block with MemType of " << MemType << " and failed due to the fact that the memory is not mapped\n";
      return nullptr;
    }
  }

  void MemoryBlock::unMap()
  {
    vkUnmapMemory(*pDevice, Allocation);
    Mapped = false;
  }

  void MemoryBlock::Delete(uint32_t AllocId)
  {
    MemHeader* TargetHeader;

    if(Header->ID == AllocId)
    {
      if(Header->Next != nullptr)
      {
        TargetHeader = Header->Next;
        delete Header;
        Header = TargetHeader;
      }
      else
      {
        Header = nullptr;
        delete Header;
      }

      return;
    }

    TargetHeader = Header;

    while(TargetHeader->Next != nullptr)
    {
      if(TargetHeader->Next->ID == AllocId)
      {
        MemHeader* Temp = TargetHeader->Next;

        TargetHeader->Next = Temp->Next;

        delete Temp;

        return;
      }

      TargetHeader = TargetHeader->Next;
    }

    cout << "Tried to delete Allocation with invalid ID : " << AllocId << '\n';

    return;
  }

  bool MemoryBlock::AllocateBuffer(Ek::Buffer& inBuff)
  {
    VkMemoryRequirements MemReq;
    vkGetBufferMemoryRequirements(*pDevice, inBuff.Buffer, &MemReq);

    uint32_t ReqSize = MemReq.size;
    uint32_t ReqAlignment = MemReq.alignment;

    inBuff.pDevice = pDevice;
    inBuff.pAllocator = this;
    inBuff.allocSize = ReqSize;
    inBuff.allocMemory = Allocation;
    inBuff.allocMemoryType = MemType;

    // if the base header isn't used, use it
    if(Header == nullptr)
    {
      Header = new MemHeader;

      Header->Start = 0;
      Header->MemorySize = ReqSize;
      Header->pObject = &inBuff;
      Header->ID = IdIndex;

      inBuff.allocOffset = 0;
      inBuff.AllocationID = IdIndex;

      vkBindBufferMemory(*pDevice, inBuff.Buffer, Allocation, inBuff.allocOffset);

      IdIndex++;

      return true;
    }

    MemHeader* Curr = Header;
    MemHeader* NewHeader = new MemHeader;
    NewHeader->pObject = &inBuff;

    while(Curr)
    {
      uint32_t Offset = Curr->Start + Curr->MemorySize;

      if(Curr->Next)
      {
        uint32_t Free = Curr->Next->Start - Offset;

        if(Free >= ReqSize)
        {
          if(Offset % ReqAlignment > 0)
          {
            Offset = ReqAlignment * ceil((float)Offset/(float)ReqAlignment);
          }

          NewHeader->MemorySize = ReqSize;
          NewHeader->Start= Offset;
          NewHeader->ID = IdIndex;

          // Insert NewHeader between curr and next
          NewHeader->Next = Curr->Next;
          Curr->Next = NewHeader;

          inBuff.allocOffset = Offset;
          inBuff.AllocationID = IdIndex;

          vkBindBufferMemory(*pDevice, inBuff.Buffer, inBuff.allocMemory, inBuff.allocOffset);

          IdIndex++;

          return true;
        }
      }
      else
      {
        uint32_t Free = Size - Offset;

        if(Free >= ReqSize)
        {
          if(Offset % ReqAlignment > 0)
          {
            Offset = ReqAlignment * ceil((float)Offset/(float)ReqAlignment);
          }

          NewHeader->MemorySize = ReqSize;
          NewHeader->Start = Offset;
          NewHeader->ID = IdIndex;

          Curr->Next = NewHeader;

          inBuff.allocOffset = Offset;
          inBuff.AllocationID = IdIndex;

          vkBindBufferMemory(*pDevice, inBuff.Buffer, inBuff.allocMemory, inBuff.allocOffset);

          IdIndex++;

          return true;
        }
      }

      Curr = Curr->Next;
    }

    return false;
  }

  bool MemoryBlock::AllocateTexture(Ek::Texture& inTex)
  {
    VkMemoryRequirements MemReq;
    vkGetImageMemoryRequirements(*pDevice, inTex.Image, &MemReq);

    uint32_t ReqSize = MemReq.size;
    uint32_t ReqAlignment = MemReq.alignment;

    inTex.allocSize = ReqSize;
    inTex.allocMemory = Allocation;
    inTex.allocMemoryType = MemType;
    inTex.pAllocator = this;
    inTex.pDevice = pDevice;

    // if the base header isn't used, use it
    if(Header == nullptr)
    {
      Header = new MemHeader;

      Header->Start = 0;
      Header->MemorySize = ReqSize;
      Header->pObject = &inTex;
      Header->ID = IdIndex;

      inTex.allocOffset = 0;
      inTex.AllocationID = IdIndex;

      // we assume here that IdIndex is 0, so we increase it to 1 so that we don't have duplicate AllocationIDs
      IdIndex++;

      vkBindImageMemory(*pDevice, inTex.Image, inTex.allocMemory, inTex.allocOffset);

      return true;
    }

    MemHeader* Curr = Header;
    MemHeader* NewHeader = new MemHeader;
    NewHeader->pObject = &inTex;

    while(Curr != nullptr)
    {
      uint32_t Offset = Curr->Start + Curr->MemorySize;

      if(Curr->Next)
      {
        uint32_t Free = Curr->Next->Start - Offset;

        if(Free >= ReqSize)
        {
          if(Offset % ReqAlignment > 0)
          {
            Offset = ReqAlignment * ceil((float)Offset/(float)ReqAlignment);
          }

          NewHeader->MemorySize = ReqSize;
          NewHeader->Start = Offset;
          NewHeader->ID = IdIndex;

          // Insert NewHeader between curr and next
          NewHeader->Next = Curr->Next;
          Curr->Next = NewHeader;

          inTex.allocOffset = Offset;
          inTex.AllocationID = IdIndex;

          IdIndex++;

          vkBindImageMemory(*pDevice, inTex.Image, inTex.allocMemory, inTex.allocOffset);

          return true;
        }
      }
      else
      {
        uint32_t Free = Size - Offset;

        if(Free >= ReqSize)
        {
          if(Offset % ReqAlignment > 0)
          {
            Offset = ReqAlignment * ceil((float)Offset/(float)ReqAlignment);
          }

          NewHeader->Start = Offset;
          NewHeader->MemorySize = ReqSize;
          NewHeader->ID = IdIndex;

          Curr->Next = NewHeader;

          inTex.allocOffset = Offset;
          inTex.AllocationID = IdIndex;

          IdIndex++;

          vkBindImageMemory(*pDevice, inTex.Image, inTex.allocMemory, inTex.allocOffset);

          return true;
        }
      }

      Curr = Curr->Next;
    }

    return false;
  }
}

namespace Ek
{
  void AllocatedObject::Delete()
  {
    pAllocator->Delete(AllocationID);
  }

  void AllocatedObject::Map(void** Pointer)
  {
    *Pointer = pAllocator->GetMemory(allocOffset);
    return;
  }

  void Texture::Destroy()
  {
    vkDestroyImage(*pDevice, Image, nullptr);
    Delete();
  }

  VkImageMemoryBarrier Texture::Barrier(VkImageAspectFlags Aspect, VkImageLayout NewLayout, VkAccessFlags src, VkAccessFlags dst)
  {
    VkImageMemoryBarrier Barrier{};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    Barrier.image = Image;
    Barrier.oldLayout = Layout;
    Barrier.newLayout = NewLayout;
    Barrier.srcAccessMask = src;
    Barrier.dstAccessMask = dst;

    Barrier.subresourceRange.aspectMask = Aspect;
    Barrier.subresourceRange.layerCount = 1;
    Barrier.subresourceRange.levelCount = 1;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.baseArrayLayer = 0;

    Layout = NewLayout;

    return Barrier;
  }

  void Buffer::Destroy()
  {
    vkDestroyBuffer(*pDevice, Buffer, nullptr);
    Delete();
  }
}

