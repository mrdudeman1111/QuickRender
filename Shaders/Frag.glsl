#version 440
#pragma shader_stage(fragment)

layout(set = 0, binding = 1) uniform CameraBuffer
{
  vec3 Position;
} Camera;

layout(set = 0, binding = 2) uniform sampler2D textures[2];

layout(push_constant) uniform PushConstant
{
  int id;
  int bShade;
} Constants;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inCoord;

layout(location = 0) out vec4 outColor;

const vec3 LightPos = vec3(0.f, 0.f, 5.f);
const vec4 LightColor = vec4(0.5f, 1.f, 0.5f, 1.f);

void main()
{
  vec4 Texile = texture(textures[Constants.id], inCoord);

  if(Constants.bShade == 1)
  {
    vec3 CamDir = normalize(Camera.Position-inPos);
    vec3 LightDir = normalize(Camera.Position-LightPos);


    float Ambient = 0.1f;
    vec4 AmbientColor = LightColor*Ambient;

    float Spec = 0.5f;
    vec3 Reflection = reflect(-LightDir, inNorm);
    float SpecIntensity = pow(max(dot(CamDir, Reflection), 0.0), 32);
    vec4 SpecColor = LightColor*SpecIntensity;

    vec3 Halfway = normalize(CamDir+LightDir);
    float Intensity = clamp(dot(Halfway, inNorm), 0.1f, 1.f);
    Intensity = pow(Intensity, 4.f);

    outColor = (SpecColor+AmbientColor+Texile)*Intensity;
  }
  else
  {
    outColor = Texile;
  }
}

