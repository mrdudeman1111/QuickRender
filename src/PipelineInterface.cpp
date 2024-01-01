#include "Interface.h"
#include <fstream>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace Ek
{
  PipelineInterface::PipelineInterface()
  {}

  void PipelineInterface::SetDescriptorLayout(VkDescriptorSetLayout DescLayout)
  {
    DescriptorLayout = DescLayout;
  }

  void PipelineInterface::Bind(Wrappers::CommandBuffer& cmdBuffer)
  {
    vkCmdBindPipeline(cmdBuffer.Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline);
  }

  VkResult PipelineInterface::Init(VkDevice& Device, Material& Mat, Wrappers::FrameBufferAttachment* FrameBufferAttachments, uint32_t FrameBufferAttachmentCount, VkOffset2D Offset, VkExtent2D Resolution, VkRenderPass& RenderPass, uint32_t Subpass, bool bDepthEnable)
  {
    VkResult Err;

    pipeMaterial = &Mat;
    pDevice = &Device;

    VkPipelineLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    LayoutCI.setLayoutCount = 1;
    LayoutCI.pSetLayouts = &DescriptorLayout;
    LayoutCI.pushConstantRangeCount = Mat.Constants.size();
    LayoutCI.pPushConstantRanges = Mat.Constants.data();

    if((Err = vkCreatePipelineLayout(Device, &LayoutCI, nullptr, &PipelineLayout)) != VK_SUCCESS)
    {
      return Err;
    }

    // 1. Viewport

    VkViewport Viewport{};
    VkRect2D Scissor{};
    VkPipelineViewportStateCreateInfo ViewportCI{};

    {
      Viewport.x = Offset.x;
      Viewport.y = Offset.y;
      Viewport.width = Resolution.width;
      Viewport.height = Resolution.height;
      Viewport.minDepth = 0.f;
      Viewport.maxDepth = 1.f;

      Scissor.extent = Resolution;
      Scissor.offset = Offset;

      ViewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      ViewportCI.viewportCount = 1;
      ViewportCI.pViewports = &Viewport;
      ViewportCI.scissorCount = 1;
      ViewportCI.pScissors = &Scissor;
    }

    // 2. Vertices

    std::vector<VkVertexInputAttributeDescription> Attributes = Vertex::GetAttributes();
    std::vector<VkVertexInputBindingDescription> Binding = Vertex::GetBinding();
    VkPipelineVertexInputStateCreateInfo InputState{};

    {
      InputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      InputState.vertexBindingDescriptionCount = Binding.size();
      InputState.pVertexBindingDescriptions = Binding.data();
      InputState.vertexAttributeDescriptionCount = Attributes.size();
      InputState.pVertexAttributeDescriptions = Attributes.data();
    }

    // 3. Shaders

    VkPipelineShaderStageCreateInfo Stages[2]{};

    {
      Stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      Stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
      Stages[0].pName = "main";
      Stages[0].module = pipeMaterial->Vertex;

      Stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      Stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      Stages[1].pName = "main";
      Stages[1].module = pipeMaterial->Fragment;
    }

    // 4. Raster

    VkPipelineRasterizationStateCreateInfo Raster{};

    {
      Raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      Raster.cullMode = VK_CULL_MODE_BACK_BIT;
      Raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      Raster.polygonMode = VK_POLYGON_MODE_FILL;
      Raster.lineWidth = 1.f;

      Raster.depthBiasEnable = VK_FALSE;
      Raster.depthClampEnable = VK_FALSE;
      Raster.rasterizerDiscardEnable = VK_FALSE;
    }

    // 5. Assembly

    VkPipelineInputAssemblyStateCreateInfo InputAssembly{};

    {
      InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      InputAssembly.primitiveRestartEnable = VK_FALSE;
    }

    // 6. Color Blend

    std::vector<VkPipelineColorBlendAttachmentState> BlendAttachments(0);
    VkPipelineColorBlendStateCreateInfo BlendState{};

    {
      for(uint32_t i = 0; i < FrameBufferAttachmentCount; i++)
      {
        // we access the value at the Subpass Index to see what purpose this attachment serves during this pipline's operations
        if(FrameBufferAttachments[i].SubpassAttachments[Subpass] != eDepth)
        {
          BlendAttachments.push_back({});

          BlendAttachments[i].blendEnable = VK_FALSE;

          BlendAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;
          BlendAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;

          BlendAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
          BlendAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
          BlendAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
          BlendAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;

          BlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }
      }

      BlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      BlendState.logicOpEnable = VK_FALSE;
      BlendState.logicOp = VK_LOGIC_OP_NO_OP;
      BlendState.attachmentCount = BlendAttachments.size();
      BlendState.pAttachments = BlendAttachments.data();

      BlendState.blendConstants[0] = 1;
      BlendState.blendConstants[1] = 1;
      BlendState.blendConstants[2] = 1;
      BlendState.blendConstants[3] = 1;
    }

    // 7. Depth

    VkPipelineDepthStencilStateCreateInfo DepthState{};

    {
      DepthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      DepthState.stencilTestEnable = VK_TRUE;
      DepthState.minDepthBounds = 0.f;
      DepthState.maxDepthBounds = 1.f;

      DepthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
      DepthState.depthBoundsTestEnable = VK_FALSE;
      DepthState.depthWriteEnable = VK_TRUE;
      DepthState.depthTestEnable = VK_TRUE;
    }

    // 8. Multisampling

    VkPipelineMultisampleStateCreateInfo MultiSampling{};

    {
      MultiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      MultiSampling.alphaToOneEnable = VK_FALSE;
      MultiSampling.sampleShadingEnable = VK_FALSE;
      MultiSampling.alphaToCoverageEnable = VK_FALSE;
      MultiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    // 9. Graphics Pipeline

    VkGraphicsPipelineCreateInfo PipelineCI{};

    {
      PipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      PipelineCI.layout = PipelineLayout;
      PipelineCI.stageCount = 2;
      PipelineCI.pStages = Stages;
      PipelineCI.renderPass = RenderPass;
      PipelineCI.subpass = Subpass;
      PipelineCI.pViewportState = &ViewportCI;
      PipelineCI.pVertexInputState = &InputState;
      PipelineCI.pInputAssemblyState = &InputAssembly;
      PipelineCI.pMultisampleState = &MultiSampling;
      PipelineCI.pColorBlendState = &BlendState;
      PipelineCI.pDepthStencilState = &DepthState;
      PipelineCI.pRasterizationState = &Raster;
    }

    if((Err = vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &PipelineCI, nullptr, &Pipeline)) != VK_SUCCESS)
    {
      return Err;
    }

    return VK_SUCCESS;
  }

  PipelineInterface::~PipelineInterface()
  {
    vkDestroyPipeline(*pDevice, Pipeline, nullptr);
    vkDestroyPipelineLayout(*pDevice, PipelineLayout, nullptr);
  }
}

