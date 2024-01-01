#include <cstring>
#include <iostream>
#include <cinttypes>
#include <iterator>
#include <vulkan/vulkan_core.h>

#include "AssetMan.h"
#include "Interface.h"
#include "Memory.h"

namespace Ek
{
  AssetInterface::AssetInterface()
  {}

  AssetInterface::~AssetInterface()
  {}

  void AssetManager::Init(EkBackend::AllocateInterface* Allocator)
  {
    pAllocator = Allocator;
  }

  Ek::Texture* AssetManager::RequestTexture(const char* FilePath)
  {
    uint32_t i;

    for(i = 0; i < Textures.size(); i++)
    {
      if(strcmp(Textures[i].second.c_str(), FilePath) == 0)
      {
        return &Textures[i].first;
      }
    }

    Ek::Texture NewTex;

    // textures should have color attachment optimal, because they are used as color storage/textures for objects
    // it is assumed that this will be sampled in a shader to be used as a texture
    pAllocator->LoadImage(FilePath, NewTex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT);

    Textures.push_back({NewTex, FilePath});

    return &Textures[i].first;
  }

  void AssetManager::Unload(const char* FilePath)
  {
    for(uint32_t i = 0; i < Textures.size(); i++)
    {
      if(Textures[i].second.compare(FilePath))
      {
        Textures[i].first.Destroy();
        Textures.erase(Textures.begin()+i);
      }
    }
  }

  void AssetManager::Unload(Ek::Texture& Texture)
  {
    for(uint32_t i = 0; i < Textures.size(); i++)
    {
      if(Textures[i].first.Image == Texture.Image)
      {
        Textures[i].first.Destroy();
        Textures.erase(Textures.begin()+i);
      }
    }
  }
}

