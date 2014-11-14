#version 130

uniform mat4 transform;

in vec2 position;
in vec4 color;

out vec4 Color;
out vec2 Position;

void main() {
  gl_Position = transform * vec4(position, 0, 1);
  Color = color;
  Position = position;
}
