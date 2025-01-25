#version 460 core

layout (location = 0) out vec4 FragColor;

in vec2 UV;

uniform sampler2D u_InputImage;

void main()
{
	vec4 imageColour = texture(u_InputImage, UV);
	FragColor = vec4(imageColour);
}