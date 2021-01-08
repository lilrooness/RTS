#version 330 core

#extension GL_ARB_explicit_uniform_location : enable

#define M_PI 3.1415926535897932384626433832795

layout (location=1) in vec3 pos; // model space vertex coordinate
layout (location=2) in vec3 translation;
layout (location=3) in vec3 normal;
layout (location=4) in float heading;
layout (location=5) in vec4 tint;

layout (location=1) uniform mat4 model;
layout (location=2) uniform mat4 view;
layout (location=3) uniform mat4 projection;
layout (location=4) uniform float xRotation;

out float intensity;
out vec4 tintColor;

mat4 BuildTranslate(vec4 trans) {
	mat4 translationMatrix = mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		trans.x, trans.y, trans.z, 1.0
	);

	return translationMatrix;
}

mat4 BuildRotateX(float t) {
	mat4 rotationMatrix = mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cos(t), -sin(t), 0.0f,
		0.0f, sin(t), cos(t), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	return rotationMatrix;
}

mat4 BuildRotateY(float t) {
	mat4 rotationMatrix = mat4(
		cos(t), 0.0f, sin(t), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-sin(t), 0.0f, cos(t), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	return rotationMatrix;
}

void main() {
	vec3 lightDir = vec3(0.0f, 0.0f, -1.0f);
	mat4 translationMatrix = BuildTranslate(vec4(translation.xyz, 1.0));
	mat4 rotationMatrix = BuildRotateY(heading);

	// Flip the model 180 around the Y axis, 
	// because we assume that by default it's facing the wrong direction (-z)
	mat4 modelCorrectionRotation = BuildRotateY(M_PI);
	rotationMatrix = rotationMatrix * modelCorrectionRotation;

	intensity = dot(rotationMatrix * vec4(normal, 1.0f), vec4(lightDir, 1.0f));
	tintColor = tint;
	gl_Position = projection * view * model * translationMatrix * rotationMatrix * vec4(pos.xyz, 1);
}

