#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 atexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
//out vec3 Normal;
//out vec3 Pos;

void main()
{
	TexCoords = atexCoord;
	//Normal = aNormal;
	//Pos = (projection * view * model * vec4(aPos, 1.0f)).xyz;
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}