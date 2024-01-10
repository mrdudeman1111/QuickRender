#pragma once

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

struct PointLight
{
  glm::vec3 Pos;
  glm::vec3 LightColor;
};

struct LightBuffer
{
  uint8_t LightCount;
  PointLight Lights[50];
};

