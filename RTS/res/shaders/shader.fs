#version 330 core

in float intensity;
in vec4 tintColor;

out vec4 LFragment;

void main() {
	vec4 ambientColor = vec4(0.1, 0.1, 0.1, 1.0);
	LFragment = (tintColor * intensity) + ambientColor;
}
