#version 330 core

#extension GL_ARB_explicit_uniform_location : enable

layout (location=1) uniform mat4 model;
layout (location=2) uniform mat4 view;
layout (location=3) uniform mat4 projection;
layout (location=4) uniform float opacity;

layout (location=1) in vec3 pos;

out float alpha;

void main() {
	alpha = opacity;
	gl_Position = projection * view * model * vec4(pos.xyz, 1.0f);
}