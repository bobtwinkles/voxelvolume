#version 130

uniform mat4 view_matrix, projection_matrix;

in vec4 position;
in vec3 color;

out vec3 Color;
out vec3 Position;

void main() {
  Color = color;
  gl_Position = projection_matrix * view_matrix * position;
  Position = position.xyz;
}
