#version 300 es
out mediump vec4 FragColor;

in mediump vec2 TexCoords;

uniform sampler2D depthMap;

void main()
{
    mediump float depthValue = texture(depthMap, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}