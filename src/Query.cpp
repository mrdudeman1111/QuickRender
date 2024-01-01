#include "Interface.h"
#include <vulkan/vulkan_core.h>

namespace Ek
{
  namespace Query
  {
    VkFormat GetBestFormat(VkPhysicalDevice& PDev, VkImageUsageFlags Usage, ColorSpace DesiredColorSpace, DataType DatType)
    {
      std::vector<VkFormat> Desired(0);

      switch(DesiredColorSpace)
      {
        case eRGBA:
          if(DatType & uNorm)
          {
            Desired.push_back(VK_FORMAT_R8G8B8A8_UNORM);
            Desired.push_back(VK_FORMAT_R16G16B16A16_UNORM);
          }
          else if(DatType & sNorm)
          {
            Desired.push_back(VK_FORMAT_R8G8B8A8_SNORM);
            Desired.push_back(VK_FORMAT_R16G16B16A16_SNORM);
          }
          else if(DatType & sInt)
          {
            Desired.push_back(VK_FORMAT_R8G8B8A8_SINT);
            Desired.push_back(VK_FORMAT_R16G16B16A16_SINT);
          }
          else if(DatType & uInt)
          {
            Desired.push_back(VK_FORMAT_R8G8B8A8_UINT);
            Desired.push_back(VK_FORMAT_R16G16B16A16_UINT);
          }
          else if(DatType & sScaled)
          {
            Desired.push_back(VK_FORMAT_R8G8B8A8_SSCALED);
            Desired.push_back(VK_FORMAT_R16G16B16A16_SSCALED);
          }
          else if(DatType & uScaled)
          {
            Desired.push_back(VK_FORMAT_R8G8B8A8_USCALED);
            Desired.push_back(VK_FORMAT_R16G16B16A16_USCALED);
          }
          else if(DatType & Packed)
          {
            Desired.push_back(VK_FORMAT_R5G5B5A1_UNORM_PACK16);
            Desired.push_back(VK_FORMAT_R4G4B4A4_UNORM_PACK16);
          }

          break;

        case eABGR:
          if(DatType & uNorm)
          {
            Desired.push_back(VK_FORMAT_A8B8G8R8_UNORM_PACK32);
            Desired.push_back(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
          }
          else if(DatType & sNorm)
          {
            Desired.push_back(VK_FORMAT_A8B8G8R8_SNORM_PACK32);
            Desired.push_back(VK_FORMAT_A2B10G10R10_SNORM_PACK32);
          }
          else if(DatType & sInt)
          {
            Desired.push_back(VK_FORMAT_A8B8G8R8_SINT_PACK32);
            Desired.push_back(VK_FORMAT_A2B10G10R10_SINT_PACK32);
          }
          else if(DatType & uInt)
          {
            Desired.push_back(VK_FORMAT_A8B8G8R8_UINT_PACK32);
            Desired.push_back(VK_FORMAT_A2B10G10R10_UINT_PACK32);
          }
          else if(DatType & sScaled)
          {
            Desired.push_back(VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
            Desired.push_back(VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
          }
          else if(DatType & uScaled)
          {
            Desired.push_back(VK_FORMAT_A8B8G8R8_USCALED_PACK32);
            Desired.push_back(VK_FORMAT_A2B10G10R10_USCALED_PACK32);
          }
          else if(DatType & Packed)
          {
            Desired.push_back(VK_FORMAT_R5G5B5A1_UNORM_PACK16);
            Desired.push_back(VK_FORMAT_R4G4B4A4_UNORM_PACK16);
          }

          break;

        case eRGB:
          if(DatType & uNorm)
          {
            Desired.push_back(VK_FORMAT_R8G8B8_UNORM);
            Desired.push_back(VK_FORMAT_R16G16B16_UNORM);
          }
          else if(DatType & sNorm)
          {
            Desired.push_back(VK_FORMAT_R8G8B8_SNORM);
            Desired.push_back(VK_FORMAT_R16G16B16_SNORM);
          }
          else if(DatType & sInt)
          {
            Desired.push_back(VK_FORMAT_R8G8B8_SINT);
            Desired.push_back(VK_FORMAT_R16G16B16_SINT);
          }
          else if(DatType & uInt)
          {
            Desired.push_back(VK_FORMAT_R8G8B8_UINT);
            Desired.push_back(VK_FORMAT_R16G16B16_UINT);
          }
          else if(DatType & sScaled)
          {
            Desired.push_back(VK_FORMAT_R8G8B8_SSCALED);
            Desired.push_back(VK_FORMAT_R16G16B16_SSCALED);
          }
          else if(DatType & uScaled)
          {
            Desired.push_back(VK_FORMAT_R8G8B8_USCALED);
            Desired.push_back(VK_FORMAT_R16G16B16_USCALED);
          }
          else if(DatType & Packed)
          {}

          break;

        case eBGR:
          if(DatType & uNorm)
          {
            Desired.push_back(VK_FORMAT_B8G8R8_UNORM);
          }
          else if(DatType & sNorm)
          {
            Desired.push_back(VK_FORMAT_B8G8R8_SNORM);
          }
          else if(DatType & sInt)
          {
            Desired.push_back(VK_FORMAT_B8G8R8_SINT);
          }
          else if(DatType & uInt)
          {
            Desired.push_back(VK_FORMAT_B8G8R8_UINT);
          }
          else if(DatType & sScaled)
          {
            Desired.push_back(VK_FORMAT_B8G8R8_SSCALED);
          }
          else if(DatType & uScaled)
          {
            Desired.push_back(VK_FORMAT_B8G8R8_USCALED);
          }
          else if(DatType & Packed)
          {}

          break;

        case eRG:
          if(DatType & uNorm)
          {
            Desired.push_back(VK_FORMAT_R8G8_UNORM);
          }
          else if(DatType & sNorm)
          {
            Desired.push_back(VK_FORMAT_R8G8_SNORM);
          }
          else if(DatType & sInt)
          {
            Desired.push_back(VK_FORMAT_R8G8_SINT);
          }
          else if(DatType & uInt)
          {
            Desired.push_back(VK_FORMAT_R8G8_UINT);
          }
          else if(DatType & sScaled)
          {
            Desired.push_back(VK_FORMAT_R8G8_SSCALED);
          }
          else if(DatType & uScaled)
          {
            Desired.push_back(VK_FORMAT_R8G8_USCALED);
          }
          else if(DatType & Packed)
          {
            Desired.push_back(VK_FORMAT_R4G4_UNORM_PACK8);
          }

          break;
        default:
          break;
      }

      for(uint32_t i = 0; i < Desired.size(); i++)
      {
        VkImageFormatProperties FormProps;
        if(vkGetPhysicalDeviceImageFormatProperties(PDev, Desired[i], VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, Usage, 0, &FormProps) == VK_SUCCESS)
        {
          return Desired[i];
        }
      }

      return VK_FORMAT_UNDEFINED;

      /*
      if(DesiredColorSpace & eRGBA)
      {
        // RGBA 16 4-4-4-4
          FormProps.push_back({VK_FORMAT_R4G4B4A4_UNORM_PACK16, {}});

        // RGBA 16 5-5-5-1
          FormProps.push_back({VK_FORMAT_R5G5B5A1_UNORM_PACK16, {}});

        // RGBA 32 8-8-8-8
          FormProps.push_back({VK_FORMAT_R8G8B8A8_SNORM, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8A8_UNORM, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8A8_SINT, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8A8_UINT, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8A8_SSCALED, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8A8_USCALED, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8A8_SRGB, {}});

        // RGBA 64 16-16-16-16
          FormProps.push_back({VK_FORMAT_R16G16B16A16_SNORM, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16A16_UNORM, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16A16_SINT, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16A16_UINT, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16A16_SSCALED, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16A16_USCALED, {}});
      }

      if(DesiredColorSpace & eABGR)
      {
        // ABGR 32 8-8-8-8
          FormProps.push_back({VK_FORMAT_A8B8G8R8_SNORM_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A8B8G8R8_UNORM_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A8B8G8R8_SINT_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A8B8G8R8_UINT_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A8B8G8R8_SSCALED_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A8B8G8R8_USCALED_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A8B8G8R8_SRGB_PACK32, {}});

        // AGBR 32 2-10-10-10
          FormProps.push_back({VK_FORMAT_A2B10G10R10_SNORM_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A2B10G10R10_UNORM_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A2B10G10R10_SINT_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A2B10G10R10_UINT_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A2B10G10R10_SSCALED_PACK32, {}});
          FormProps.push_back({VK_FORMAT_A2B10G10R10_USCALED_PACK32, {}});
      }

      if(DesiredColorSpace & eRGB)
      {
        // RGB 8
          FormProps.push_back({VK_FORMAT_R8G8B8_SNORM, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8_UNORM, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8_SINT, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8_UINT, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8_SSCALED, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8_USCALED, {}});
          FormProps.push_back({VK_FORMAT_R8G8B8_SRGB, {}});

          FormProps.push_back({VK_FORMAT_R16G16B16_SNORM, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16_UNORM, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16_SINT, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16_UINT, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16_SSCALED, {}});
          FormProps.push_back({VK_FORMAT_R16G16B16_USCALED, {}});

          FormProps.push_back({VK_FORMAT_R5G6B5_UNORM_PACK16, {}});
      }

      if(DesiredColorSpace & eBGR)
      {
        // BGR 24
          FormProps.push_back({VK_FORMAT_B8G8R8_SNORM, {}});
          FormProps.push_back({VK_FORMAT_B8G8R8_UNORM, {}});
          FormProps.push_back({VK_FORMAT_B8G8R8_SINT, {}});
          FormProps.push_back({VK_FORMAT_B8G8R8_UINT, {}});
          FormProps.push_back({VK_FORMAT_B8G8R8_SSCALED, {}});
          FormProps.push_back({VK_FORMAT_B8G8R8_USCALED, {}});
          FormProps.push_back({VK_FORMAT_B8G8R8_SRGB, {}});

        // BGR 16
          FormProps.push_back({VK_FORMAT_B5G6R5_UNORM_PACK16, {}});
      }

      if(DesiredColorSpace & eRG)
      {
        // RG 16
          FormProps.push_back({VK_FORMAT_R8G8_SNORM, {}});
          FormProps.push_back({VK_FORMAT_R8G8_UNORM, {}});
          FormProps.push_back({VK_FORMAT_R8G8_SINT, {}});
          FormProps.push_back({VK_FORMAT_R8G8_UINT, {}});
          FormProps.push_back({VK_FORMAT_R8G8_SSCALED, {}});
          FormProps.push_back({VK_FORMAT_R8G8_USCALED, {}});
          FormProps.push_back({VK_FORMAT_R8G8_SRGB, {}});

        //RG 32
          FormProps.push_back({VK_FORMAT_R16G16_SNORM, {}});
          FormProps.push_back({VK_FORMAT_R16G16_UNORM, {}});
          FormProps.push_back({VK_FORMAT_R16G16_SINT, {}});
          FormProps.push_back({VK_FORMAT_R16G16_UINT, {}});
          FormProps.push_back({VK_FORMAT_R16G16_SSCALED, {}});
          FormProps.push_back({VK_FORMAT_R16G16_USCALED, {}});

        // RG 64
          FormProps.push_back({VK_FORMAT_R32G32_SINT, {}});
          FormProps.push_back({VK_FORMAT_R32G32_UINT, {}});
          FormProps.push_back({VK_FORMAT_R32G32_SFLOAT, {}});
      }
      */
    }
  }
}

