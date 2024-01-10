#include <iostream>
#include <stdexcept>

#include "Mesh.h"

#include <glm/gtx/transform.hpp>
#include <vulkan/vulkan_core.h>

#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

  const std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributes()
  {
    std::vector<VkVertexInputAttributeDescription> Ret(3);

    Ret[0].binding = 0;
    Ret[0].location = 0;
    Ret[0].offset = offsetof(Vertex, Position);
    Ret[0].format = VK_FORMAT_R32G32B32_SFLOAT;

    Ret[1].binding = 0;
    Ret[1].location = 1;
    Ret[1].offset = offsetof(Vertex, Normal);
    Ret[1].format = VK_FORMAT_R32G32B32_SFLOAT;

    Ret[2].binding = 0;
    Ret[2].location = 2;
    Ret[2].offset = offsetof(Vertex, TexPos);
    Ret[2].format = VK_FORMAT_R32G32_SFLOAT;

    return Ret;
  }

  const std::vector<VkVertexInputBindingDescription> Vertex::GetBinding()
  {
    std::vector<VkVertexInputBindingDescription> Ret(1);

    Ret[0].binding = 0;
    Ret[0].stride = sizeof(Vertex);
    Ret[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return Ret;
  }

namespace Ek
{
  Renderable::Renderable()
  {}

  Renderable::~Renderable()
  {}

  Mesh::Mesh()
  {
    Transform = glm::mat4(1.f);
  }

  Mesh::~Mesh()
  {
    pTemp = nullptr;

    VertexBuffer.Destroy();
    IndexBuffer.Destroy();
    TransitBuffer.Destroy();
    cmdBuffer.Delete();

    Vertices.clear();
    Indices.clear();
    Path.clear();

    if(Albedo.Image != VK_NULL_HANDLE)
    {
      Albedo.Destroy();
      vkDestroyImageView(*pDevice, AlbedoView, nullptr);
      vkDestroySampler(*pDevice, AlbedoSampler, nullptr);
    }

    pDevice = nullptr;
  }

  void Mesh::Draw(Wrappers::CommandBuffer& inBuffer)
  {
    VkDeviceSize Offset = 0;
    vkCmdBindVertexBuffers(inBuffer.Buffer, 0, 1, &VertexBuffer.Buffer, &Offset);
    vkCmdBindIndexBuffer(inBuffer.Buffer, IndexBuffer.Buffer, Offset, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(inBuffer.Buffer, Indices.size(), 1, 0, 0, 0);
  }

  void Mesh::Allocate(Wrappers::CommandBuffer& inCmdBuffer)
  {
    VkResult Err;

    cmdBuffer = inCmdBuffer;

    VkBufferCreateInfo VertexBufferCI{};
    VertexBufferCI.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    VertexBufferCI.size        = sizeof(Vertex) * Vertices.size();
    VertexBufferCI.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VertexBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Err = vkCreateBuffer(*pDevice, &VertexBufferCI, nullptr, &VertexBuffer.Buffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create buffer: " + std::to_string(Err));
    }

    VkBufferCreateInfo IndexBufferCI{};
    IndexBufferCI.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    IndexBufferCI.size        = sizeof(uint32_t) * Indices.size();
    IndexBufferCI.usage       = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    IndexBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Err = vkCreateBuffer(*pDevice, &IndexBufferCI, nullptr, &IndexBuffer.Buffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create buffer: " + std::to_string(Err));
    }

    VkBufferCreateInfo TransitBufferCI{};
    TransitBufferCI.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    TransitBufferCI.size        = (sizeof(Vertex)*Vertices.size())+(sizeof(uint32_t)*Indices.size());
    TransitBufferCI.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    TransitBufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if((Err = vkCreateBuffer(*pDevice, &TransitBufferCI, nullptr, &TransitBuffer.Buffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create buffer: " + std::to_string(Err));
    }

    Alloc->AllocateBuffer(VertexBuffer, eLocalMemory);
    Alloc->AllocateBuffer(IndexBuffer, eLocalMemory);
    Alloc->AllocateBuffer(TransitBuffer, eHostMemory);

    TransitBuffer.Map(&pTemp);
      memcpy(pTemp, Vertices.data(), sizeof(Vertex)*Vertices.size());
      memcpy((Vertex*)pTemp + Vertices.size(), Indices.data(), sizeof(uint32_t)*Indices.size());

      VkBufferCopy VertexCopyInfo{};
      VertexCopyInfo.size = sizeof(Vertex)*Vertices.size();
      VertexCopyInfo.srcOffset = 0;
      VertexCopyInfo.dstOffset = 0;

      VkBufferCopy IndexCopyInfo{};
      IndexCopyInfo.size = sizeof(uint32_t)*Indices.size();
      IndexCopyInfo.srcOffset = VertexCopyInfo.size;
      IndexCopyInfo.dstOffset = 0;

      cmdBuffer.BeginCommand();
        vkCmdCopyBuffer(cmdBuffer.Buffer, TransitBuffer.Buffer, VertexBuffer.Buffer, 1, &VertexCopyInfo);
        vkCmdCopyBuffer(cmdBuffer.Buffer, TransitBuffer.Buffer, IndexBuffer.Buffer, 1, &IndexCopyInfo);
      cmdBuffer.EndComand();

    std::cout << "allocating mesh with size: " << VertexBuffer.allocSize+IndexBuffer.allocSize << '\n';

    cmdBuffer.FenceWait();
  }

  void Mesh::Move(glm::vec3 Direction)
  {
    Transform = glm::translate(Transform, Direction);
  }

  /* 
   things to note here:
    assimp uses:
    Z : up-axis
    Y : forward-axis
    x : right-axis

    assim uses row-major matrices, so we must convert them to column major for use with glm

    texture coordinate origin(0,0) is at the lower left corner for assimp, because it is made for use with opengl.
    but with vulkan our origin is at the top left corner, we must invert the Y coordinate for correct texture wrapping
  */
  void Mesh::Load(EkBackend::AllocateInterface* pAlloc, VkDevice& inDevice, EkBackend::DescriptorSet& Set, std::string inPath)
  {
    Path = inPath;
    SceneSet = &Set;

    Alloc = pAlloc;

    pDevice = &inDevice;

    std::string AbsolutePath = MODELDIR;
    AbsolutePath.append(inPath);

    const aiScene* Scene = Importer.ReadFile(AbsolutePath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if(Scene == nullptr)
    {
      throw std::runtime_error("failed to load mesh at : " + AbsolutePath);
    }

    Vertices.resize(Scene->mMeshes[0]->mNumVertices);

    for(uint32_t i = 0; i < Scene->mMeshes[0]->mNumVertices; i++)
    {
      Vertices[i].Position.y = Scene->mMeshes[0]->mVertices[i].z;
      Vertices[i].Position.x = Scene->mMeshes[0]->mVertices[i].x;
      Vertices[i].Position.z = Scene->mMeshes[0]->mVertices[i].y*-1.f;

      if(Scene->mMeshes[0]->HasTextureCoords(0))
      {
        const aiVector3D* TexCoord = &Scene->mMeshes[0]->mTextureCoords[0][i];
        Vertices[i].TexPos.x = TexCoord->x;
        Vertices[i].TexPos.y = 1.f-TexCoord->y;
      }

      Vertices[i].Normal.x = Scene->mMeshes[0]->mNormals[i].x;
      Vertices[i].Normal.y = Scene->mMeshes[0]->mNormals[i].y;
      Vertices[i].Normal.z = Scene->mMeshes[0]->mNormals[i].z;
    }

    for(uint32_t i = 0; i < Scene->mMeshes[0]->mNumFaces; i++)
    {
      for(uint32_t x = 0; x < Scene->mMeshes[0]->mFaces[i].mNumIndices; x++)
      {
        Indices.push_back(Scene->mMeshes[0]->mFaces[i].mIndices[x]);
      }
    }

    aiString aiAlbedoPath;
    std::string AlbedoPath = MODELDIR;

    if(Scene->mMaterials[Scene->mMeshes[0]->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
      if(Scene->mMaterials[Scene->mMeshes[0]->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &aiAlbedoPath) != AI_SUCCESS)
      {
        throw std::runtime_error("Failed to find material texture from assimp scene while importing mesh");
      }

      AlbedoPath.append(aiAlbedoPath.C_Str());
    }
    else if(Scene->mMaterials[Scene->mMeshes[0]->mMaterialIndex]->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
    {
      if(Scene->mMaterials[Scene->mMeshes[0]->mMaterialIndex]->GetTexture(aiTextureType_BASE_COLOR, 0, &aiAlbedoPath) != AI_SUCCESS)
      {
        throw std::runtime_error("Failed to find material texture from assimp scene while importing mesh");
      }

     AlbedoPath.append(aiAlbedoPath.C_Str());
    }
    else
    {
      Importer.FreeScene();
      return;
    }

    VkResult Err = VK_SUCCESS;

    Err = Alloc->LoadImage(AlbedoPath.c_str(), Albedo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    Err = Alloc->CreateImageView(AlbedoView, Albedo, VK_IMAGE_ASPECT_COLOR_BIT);

    VkSamplerCreateInfo SamplerCI{};
    SamplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    SamplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCI.minLod = 1.f;
    SamplerCI.maxLod = 1.f;
    SamplerCI.minFilter = VK_FILTER_LINEAR;
    SamplerCI.magFilter = VK_FILTER_LINEAR;
    SamplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    SamplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    SamplerCI.anisotropyEnable = VK_FALSE;
    SamplerCI.maxAnisotropy = 1.f;

    if(vkCreateSampler(*pDevice, &SamplerCI, nullptr, &AlbedoSampler) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create sampler");
    }
  }

  void Mesh::SetTextureBinding(uint32_t Binding, uint32_t Location)
  {
    ShaderLocation.first = Binding;
    ShaderLocation.second = Location;

    VkDescriptorImageInfo DescImg{};
    DescImg.imageLayout = Albedo.Layout;
    DescImg.imageView = AlbedoView;
    DescImg.sampler = AlbedoSampler;

    VkWriteDescriptorSet DescWrite{};
    DescWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescWrite.descriptorCount = 1;
    DescWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DescWrite.dstSet = SceneSet->Descriptor;
    DescWrite.pImageInfo = &DescImg;
    DescWrite.dstBinding = Binding;
    DescWrite.dstArrayElement = Location;

    vkUpdateDescriptorSets(*pDevice, 1, &DescWrite, 0, nullptr);
  }
}

