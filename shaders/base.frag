#version 130

in vec3 Color;
in vec3 Position;
out vec4 outputF;

void main(void) {
  outputF = vec4(Color, 1.0);
}
