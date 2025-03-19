#version 300 es
uniform sampler2D shadowMap;

// Light structure
struct LightProperties {
     int type;
     mediump vec4 ambient;
     mediump vec4 diffuse;
     mediump vec4 specular;
     mediump vec4 position;
     mediump vec4 direction;
     mediump float spotCutoff;
     mediump float spotExponent;
};

// Material structure
struct MaterialProperties {
     mediump vec4 ambient;
     mediump vec4 diffuse;
     mediump vec4 specular;
     mediump float shininess;
};

const int MaxLights = 8;
layout (std140) uniform LightBuffer {
     LightProperties Lights[MaxLights];
};

const int MaxMaterials = 8;
layout (std140) uniform MaterialBuffer {
     MaterialProperties Materials[MaxMaterials];
};

// Selected material
uniform int Material;

// Number of lights
uniform int NumLights;
uniform int LightOn[MaxLights];

out mediump vec4 fragColor;

in mediump vec4 Position;
in mediump vec3 Normal;
in mediump vec3 View;
in mediump vec4 LightPosition;

// TODO: Perform shadow depth comparison
mediump float ShadowCalculation(mediump vec4 fragLightPos) {
     // Normalize light position [-1, 1]
     mediump vec3 projCoords = fragLightPos.xyz/fragLightPos.w;

     // Convert to depth range [0, 1]
     projCoords = projCoords*0.5 + 0.5;

     mediump float closestDepth = texture(shadowMap, projCoords.xy).r;
     mediump float curDepth = projCoords.z;

     mediump float bias = 0.005;
     return curDepth - bias > closestDepth ? 1.0f : 0.0f;
}

void main()
{
     mediump vec3 rgb = vec3(0.0f);
     mediump vec3 NormNormal = normalize(Normal);
     mediump vec3 NormView = normalize(View);

     for (int i = 0; i < NumLights; i++) {
          // If light is not off
          if (LightOn[i] != 0) {
               // Ambient component
               if (Lights[i].type != 0) {
                    rgb += vec3(Lights[i].ambient*Materials[Material].ambient);
               }
               // Directional Light
               if (Lights[i].type == 1) {
                    mediump vec3 LightDirection = -normalize(vec3(Lights[i].direction));
                    mediump vec3 HalfVector = normalize(LightDirection + NormView);
                    // Diffuse
                    mediump float diff = max(0.0f, dot(NormNormal, LightDirection));
                    rgb += diff*vec3(Lights[i].diffuse*Materials[Material].diffuse);
                    if (diff > 0.0) {
                         // Specular term
                         mediump float spec = pow(max(0.0f, dot(Normal, HalfVector)), Materials[Material].shininess);
                         rgb += spec*vec3(Lights[i].specular*Materials[Material].specular);
                    }
               }
               // Point light
               if (Lights[i].type == 2) {
                    mediump vec3 LightDirection = normalize(vec3(Lights[i].position - Position));
                    mediump vec3 HalfVector = normalize(LightDirection + NormView);
                    // Diffuse
                    mediump float diff = max(0.0f, dot(NormNormal, LightDirection));
                    rgb += diff*vec3(Lights[i].diffuse*Materials[Material].diffuse);
                    if (diff > 0.0) {
                         // Specular term
                         mediump float spec = pow(max(0.0f, dot(Normal, HalfVector)), Materials[Material].shininess);
                         rgb += spec*vec3(Lights[i].specular*Materials[Material].specular);
                    }
               }
               // Spot light
               if (Lights[i].type == 3) {
                    mediump vec3 LightDirection = normalize(vec3(Lights[i].position - Position));
                    // Determine if inside cone
                    mediump float spotCos = dot(LightDirection, -normalize(vec3(Lights[i].direction)));
                    mediump float coneCos = cos(radians(Lights[i].spotCutoff));
                    if (spotCos >= coneCos) {
                         mediump vec3 HalfVector = normalize(LightDirection + NormView);
                         mediump float attenuation = pow(spotCos, Lights[i].spotExponent);
                         // Diffuse
                         mediump float diff = max(0.0f, dot(NormNormal, LightDirection))*attenuation;
                         rgb += diff*vec3(Lights[i].diffuse*Materials[Material].diffuse);
                         if (diff > 0.0) {
                              // Specular term
                              mediump float spec = pow(max(0.0f, dot(Normal, HalfVector)), Materials[Material].shininess)*attenuation;
                              rgb += spec*vec3(Lights[i].specular*Materials[Material].specular);
                         }
                    }
               }
          }
     }

     // TODO: Determine if fragment (LightPosition) is in shadow
     mediump float shadow = 1.0 - ShadowCalculation(LightPosition);

     // TODO: Apply shadow attenuation to base color
     fragColor = shadow*vec4(min(rgb,vec3(1.0)), Materials[Material].ambient.a);
}
