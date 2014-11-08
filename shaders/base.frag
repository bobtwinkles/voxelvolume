#version 140

in vec3 Color;
out vec4 outputF;

void main(void) {
  outputF = vec4(Color, 1.0);
}
