#version 330 core

in float alpha;

out vec4 LFragment;

void main() {
	LFragment = vec4(1.0,1.0,1.0, alpha);
}
