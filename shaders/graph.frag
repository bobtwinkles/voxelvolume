#version 130

uniform float x_fade_start;
uniform float x_min;

in vec4 Color;
in vec2 Position;
out vec4 outputColor;

void main() {
  outputColor = Color;

  outputColor.a = Color.a * smoothstep(x_min, x_fade_start, Position.x);
}
