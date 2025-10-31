#version 330 core

out vec4 FragColor;
in vec2 v_TexCoord;
uniform sampler2D u_Texture;

uniform vec3 u_ObjectColor;

void main()
{
    FragColor = vec4(v_TexCoord.x, v_TexCoord.y, 0.0, 1.0);
}