#version 300 es

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out mediump vec2 TexCoords;

void main( )
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos,1.0f);
}
