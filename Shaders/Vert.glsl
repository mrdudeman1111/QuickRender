#version 440
#pragma shader_stage(vertex)

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inCoord;

layout(set = 0, binding = 0) uniform CameraBuffer
{
  mat4 World;
  mat4 View;
  mat4 Projection;
  mat4 Normal;
} Camera;

layout(set = 0, Binding = 1) uniform Meshes
{
  mat4 Meshes[50];
} MeshPositions;

layout(push_constant) uniform ConstantsBuffer
{
  int PosIdx;
} Constants;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNorm;
layout(location = 2) out vec2 outCoord;

void main()
{
  gl_Position = Camera.Projection * Camera.View * Camera.World * vec4(inPos, 1.f);

  outPos = (Camera.World * vec4(inPos, 1.f)).xyz;
  outNorm = (Camera.World * vec4(inNorm, 0.f)).xyz;
  outCoord = inCoord;
}

