#version 130

in vec3 Color;
in vec3 Position;
in vec3 Normal;
out vec4 outputF;

void main(void) {
  outputF = vec4(Color, 1.0);
  outputF *= 0.01;
  outputF += vec4(abs(Normal), 1);
}
