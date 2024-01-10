#include <GLFW/glfw3.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include <vulkan/vulkan_core.h>

#include "Interface.h"
#include "Memory.h"
#include "Mesh.h"
#include "Wrappers.h"

/*
  class Texture:
    VkImageMemorybarrier Barrier();
    void destroy();

  class CommandBuffer:
    void BeginCommandBuffer();
    void EndCommand();
    void FenceWait();

  class Buffer:
    void Destroy();
*/

VkExtent2D RenderExtent{1280, 720};

class Player : Ek::Camera
{
public:
  Player()
  {
    MousePos = {0.f, 0.f};
    bShading = 0;
    tabPressed = false;
  }

  glm::vec2 MousePos;
  float Sensitivity = 0.001f;

  uint32_t bShading;

  bool tabPressed;

  void Destroy()
  {
  }

  void Update(GLFWwindow* inWindow)
  {
    glm::vec3 Move(0.f);
    glm::vec2 Rotate(0.f);

    bool Sprint = false;

    // Movement
    {
      if(glfwGetKey(inWindow, GLFW_KEY_W) == GLFW_PRESS)
      {
        Move.z += 1.f;
      }
      if(glfwGetKey(inWindow, GLFW_KEY_S) == GLFW_PRESS)
      {
        Move.z -= 1.f;
      }
      if(glfwGetKey(inWindow, GLFW_KEY_A) == GLFW_PRESS)
      {
        Move.x -= 1.f;
      }
      if(glfwGetKey(inWindow, GLFW_KEY_D) == GLFW_PRESS)
      {
        Move.x += 1.f;
      }
      if(glfwGetKey(inWindow, GLFW_KEY_Q) == GLFW_PRESS)
      {
        Move.y -= 1.f;
      }
      if(glfwGetKey(inWindow, GLFW_KEY_E) == GLFW_PRESS)
      {
        Move.y += 1.f;
      }
      if(glfwGetKey(inWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      {
        Sprint = true;
      }

      if(!tabPressed)
      {
        if(glfwGetKey(inWindow, GLFW_KEY_TAB) == GLFW_PRESS)
        {
          bShading = (bShading + 1) % 2;
          tabPressed = true;
        }
      }
      else
      {
        if(glfwGetKey(inWindow, GLFW_KEY_TAB) == GLFW_RELEASE)
        {
          tabPressed = false;
        }
      }
    }

    // rotations
    {
      double X;
      double Y;
      glfwGetCursorPos(inWindow, &X, &Y);

      double DeltaX = X-MousePos.x;
      double DeltaY = Y-MousePos.y;

      Rotate.x = DeltaX*Sensitivity;
      Rotate.y = DeltaY*Sensitivity;

      MousePos.x = X;
      MousePos.y = Y;
    }

    if(Sprint)
    {
      Move *= 100.f;
    }
    else
    {
      Move *= 0.1;
    }

    camLook(Rotate);
    camMove(Move);
    camUpdate();
  }
};


int main()
{
  Ek::vulkanInterface Renderer;

  Renderer.AddInstLayer("VK_LAYER_KHRONOS_validation");
  Renderer.AddInstExtension(VK_KHR_SURFACE_EXTENSION_NAME);

  if(Renderer.CreateInstance(RenderExtent) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create instance");
  }

  if(Renderer.CreatePhysicalDevice() != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to pick physical device");
  }

  Renderer.AddDevExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  if(Renderer.CreateDevice() != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create vulkan device");
  }


  Ek::Wrappers::FrameBufferAttachment Depth;

  {
    Depth.Format = VK_FORMAT_D32_SFLOAT;
    Depth.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    Depth.Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;

    Depth.StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    Depth.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    Depth.StencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    Depth.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    Depth.InitialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    Depth.FinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    Depth.SubpassLayouts.push_back(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    Depth.SubpassAttachments.push_back(Ek::eDepth);
  }

  Renderer.AddAttachment(Depth);

  if(Renderer.CreateSwapchain() != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create swapchain");
  }

  VkPipelineBindPoint Subpasses[1] = { VK_PIPELINE_BIND_POINT_GRAPHICS };
  if(Renderer.CreateRenderpass(1, Subpasses) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create renderpass");
  }

  if(Renderer.CreateFrameBuffers() != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create frame buffers");
  }

  // Setup Shader resources
  {
    VkDescriptorSetLayoutBinding Bindings[3]{};

    // Camera
    Bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    Bindings[0].binding = 0;
    Bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Bindings[0].descriptorCount = 1;

    // Binding 1 in vertex shader is not used right now

    Bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    Bindings[1].binding = 2;
    Bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Bindings[1].descriptorCount = 1;

    Bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    Bindings[2].binding = 3;
    Bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Bindings[2].descriptorCount = 2;

    VkPushConstantRange fragConstants{};
    fragConstants.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragConstants.offset = 0;
    fragConstants.size = sizeof(uint32_t) * 2;

    Renderer.AddDescriptorBinding(Bindings, 3);
    Renderer.CreateDescriptors();
  }

  VkPushConstantRange fragConstants{};
  fragConstants.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragConstants.offset = 0;
  fragConstants.size = sizeof(uint32_t) * 2;

  Ek::Material MainMat = Renderer.CreateMaterial();
  MainMat.LoadVertex("Shaders/Vert.spv");
  MainMat.LoadFragment("Shaders/Frag.spv");
  MainMat.AddPushConstant(fragConstants);

  Ek::PipelineInterface* pMainPipe = Renderer.CreatePipeline(MainMat, {0,0}, RenderExtent, true);

  Ek::Mesh* MainMesh = Renderer.CreateMesh("Pawn.dae");
  Ek::Mesh* envMesh = Renderer.CreateMesh("SkySphere.dae");

  uint32_t MainMeshIdx = 0;
  uint32_t envMeshIdx = 1;

  MainMesh->SetTextureBinding(3, MainMeshIdx);
  envMesh->SetTextureBinding(3, envMeshIdx);

  Player User;
  Renderer.CreateCamera((Ek::Camera*)&User, 0);

  Ek::Wrappers::CommandBuffer RenderBuffer = Renderer.GetCommandBuffer(Ek::eGraphics);

  glfwSetInputMode(Renderer.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  while(!glfwWindowShouldClose(Renderer.Window))
  {
    glfwPollEvents();

    User.Update(Renderer.Window);
  
    RenderBuffer.BeginCommand();
      Renderer.BeginRender(RenderBuffer);

        Renderer.BindShaderResources(RenderBuffer, pMainPipe);
        pMainPipe->Bind(RenderBuffer);

          vkCmdPushConstants(RenderBuffer.Buffer, pMainPipe->PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t), &User.bShading);

          vkCmdPushConstants(RenderBuffer.Buffer, pMainPipe->PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &envMeshIdx);
          envMesh->Draw(RenderBuffer);

          vkCmdPushConstants(RenderBuffer.Buffer, pMainPipe->PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &MainMeshIdx);
          MainMesh->Draw(RenderBuffer);

      Renderer.EndRender(RenderBuffer);
    RenderBuffer.EndComand(true);

    Renderer.Present(RenderBuffer);

    RenderBuffer.FenceWait();
  }

  RenderBuffer.Delete();
  delete MainMesh;
  delete envMesh;
  pMainPipe->~PipelineInterface();
  MainMat.Destroy();
  Renderer.Destroy();

  std::cout << "Clean run\n";

  return 0;
}

/*
int main()
{
  VkResult Err;

  Ek::vulkanInterface Renderer;

  if((Err = Renderer.CreateInstance(RenderExtent)) != VK_SUCCESS)     throw std::runtime_error("Failed to create instance with: " + std::to_string(Err));

  if((Err = Renderer.CreatePhysicalDevice()) != VK_SUCCESS)           throw std::runtime_error("Failed to create physical device with: " + std::to_string(Err));

  Renderer.AddDevExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  if((Err = Renderer.CreateDevice()) != VK_SUCCESS)                   throw std::runtime_error("Failed to create device with: " + std::to_string(Err));

  Ek::Wrappers::FrameBufferAttachment DepthAttachment{};

  /
    {
      PosAttachment.Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      PosAttachment.Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
      PosAttachment.Format = PosNormFormat;
      PosAttachment.InitialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      PosAttachment.FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      PosAttachment.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      PosAttachment.StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      PosAttachment.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      PosAttachment.StencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

      PosAttachment.SubpassLayouts.push_back(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      PosAttachment.SubpassAttachments.push_back(Ek::eColor);
    }

    {
      NormAttachment.Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      NormAttachment.Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
      NormAttachment.Format = PosNormFormat;
      NormAttachment.InitialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      NormAttachment.FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      NormAttachment.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      NormAttachment.StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      NormAttachment.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      NormAttachment.StencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

      NormAttachment.SubpassLayouts.push_back(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
      NormAttachment.SubpassAttachments.push_back(Ek::eColor);
    }
  /

  {
    DepthAttachment.Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    DepthAttachment.Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    DepthAttachment.Format = VK_FORMAT_D32_SFLOAT;
    DepthAttachment.InitialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    DepthAttachment.FinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    DepthAttachment.LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthAttachment.StencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthAttachment.StencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;

    DepthAttachment.SubpassLayouts.push_back(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    DepthAttachment.SubpassAttachments.push_back(Ek::eDepth);
  }

  /
    Renderer.AddAttachment(PosAttachment);
    Renderer.AddAttachment(NormAttachment);
  /
  Renderer.AddAttachment(DepthAttachment);

  Renderer.AddInstLayer("VK_LAYER_KHRONOS_validation");

  if((Err = Renderer.CreateSwapchain()) != VK_SUCCESS)                throw std::runtime_error("Failed to create swapchain with: " + std::to_string(Err));

  VkPipelineBindPoint BindPoints[] = { VK_PIPELINE_BIND_POINT_GRAPHICS };
  if((Err = Renderer.CreateRenderpass(1, BindPoints)) != VK_SUCCESS)  throw std::runtime_error("Failed to create renderpass with: " + std::to_string(Err));

  if((Err = Renderer.CreateFrameBuffer()) != VK_SUCCESS)              throw std::runtime_error("Failed to create framebuffer with: " + std::to_string(Err));

  VkDescriptorSetLayoutBinding Bindings[2]{};

  // WVP
  Bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  Bindings[0].binding = 0;
  Bindings[0].descriptorCount = 1;
  Bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

  // CameraPos
  Bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  Bindings[1].binding = 0;
  Bindings[1].descriptorCount = 1;
  Bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

  Renderer.AddDescriptorBinding(Bindings, 2);

  Renderer.CreateDescriptors();

  Ek::Material MainMat = Renderer.CreateMaterial();
  MainMat.LoadVertex("Shaders/Vert.spv");
  MainMat.LoadFragment("Shaders/Frag.spv");

  VkPushConstantRange TxIdConstant{};
  TxIdConstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  TxIdConstant.size = sizeof(uint32_t)*2;

  MainMat.AddPushConstant(TxIdConstant);

  Ek::PipelineInterface* MainPipeline = Renderer.CreatePipeline(MainMat, {0, 0}, RenderExtent, true);

  Ek::Wrappers::CommandBuffer cmdBuffer = Renderer.GetCommandBuffer(Ek::eGraphics);

  Ek::Mesh MainMesh;
  Ek::Mesh envMesh;

  Renderer.CreateMesh("Pawn.dae", MainMesh);
  Renderer.CreateMesh("SkySphere.dae", envMesh);

  MainMesh.SetTextureBinding(2, 0);
  envMesh.SetTextureBinding(2, 1);

  uint32_t MainIdx = 0;
  uint32_t envIdx = 1;

  Player User;
  Renderer.CreateCamera((Ek::Camera*)&User, 0);

  while(!Renderer.ShouldClose())
  {
    glfwPollEvents();

    glfwSetInputMode(Renderer.Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    User.Update(Renderer.Window);

    cmdBuffer.BeginCommand();
      Renderer.BeginRender(cmdBuffer);
        Renderer.BindShaderResources(cmdBuffer, MainPipeline);
        MainPipeline->Bind(cmdBuffer);

          vkCmdPushConstants(cmdBuffer.Buffer, MainPipeline->PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(uint32_t), sizeof(uint32_t), &User.bShading);

          vkCmdPushConstants(cmdBuffer.Buffer, MainPipeline->PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &envIdx);
          envMesh.Draw(cmdBuffer);

          vkCmdPushConstants(cmdBuffer.Buffer, MainPipeline->PipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &MainIdx);
          MainMesh.Draw(cmdBuffer);

      Renderer.EndRender(cmdBuffer);
    cmdBuffer.EndComand(true);

    Renderer.Present(cmdBuffer);

    cmdBuffer.FenceWait();
  }

  User.Destroy();
  cmdBuffer.Delete();
  delete MainPipeline;
  MainMat.Destroy();
  Renderer.Destroy();

  std::cout << "Successfull run\n";
}
*/
