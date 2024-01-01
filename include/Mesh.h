#pragma once

#include <assimp/Importer.hpp>
#include <string>

#include "Memory.h"
#include "Wrappers.h"

struct Vertex
{
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexPos;

  static const std::vector<VkVertexInputAttributeDescription> GetAttributes();
  static const std::vector<VkVertexInputBindingDescription> GetBinding();
};

namespace Ek
{
  class Renderable
  {
    public:
      Renderable();
      ~Renderable();

      virtual void Draw(Ek::Wrappers::CommandBuffer& inBuffer) = 0;

    protected:
      VkDevice* pDevice;
      glm::mat4 Transform;

      std::vector<Vertex> Vertices;
      std::vector<uint32_t> Indices;

      Ek::Buffer VertexBuffer;
      Ek::Buffer IndexBuffer;
  };

  class Mesh : public Renderable
  {
    public:
      Mesh();
      ~Mesh();

      void Draw(Ek::Wrappers::CommandBuffer& inBuffer);

      void Load(EkBackend::AllocateInterface* pAlloc, VkDevice& inDevice, EkBackend::DescriptorSet& Set, std::string inPath);
      void Allocate(Ek::Wrappers::CommandBuffer& inCmdBuffer);

      void Move(glm::vec3 Direction);

      void SetTextureBinding(uint32_t Binding, uint32_t Location);

      std::string Path;

    private:

      Assimp::Importer Importer;

      // Transit memory
        void* pTemp;
        Ek::Wrappers::CommandBuffer cmdBuffer;

        Ek::Buffer TransitBuffer;

      EkBackend::DescriptorSet* SceneSet;
      std::pair<uint32_t, uint32_t> ShaderLocation;

      VkSampler AlbedoSampler;
      Ek::Texture Albedo;
      VkImageView AlbedoView;

      EkBackend::AllocateInterface* Alloc;
  };
}

