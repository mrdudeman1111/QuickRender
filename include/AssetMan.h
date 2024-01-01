#pragma once

#include <vector>
#include <string>

#include "Memory.h"

namespace Ek
{
  class AssetInterface
  {
    public:
      AssetInterface();
      ~AssetInterface();

      virtual void Unload(Ek::Texture& Texture) = 0;
      virtual void Unload(const char* FilePath) = 0;
  };

  class AssetManager : public AssetInterface
  {
    public:
      void Init(EkBackend::AllocateInterface* Allocator);

      Ek::Texture* RequestTexture(const char* FilePath);

      void Unload(Ek::Texture& Texture);
      void Unload(const char* FilePath);

    private:
      // associative array of Texure/FilePath pairs
      std::vector<std::pair<Ek::Texture, std::string>> Textures;

      EkBackend::AllocateInterface* pAllocator;
  };
}

/*

class TextureInterface
{}

*/

