#version 130

uniform mat4 view_matrix, projection_matrix;

in vec4 position;
in vec3 color;
in vec3 normal;

out vec3 Color;
out vec3 Position;
out vec3 Normal;

void main() {
  Color = color;
  vec4 real_position = vec4(position.xyz, 1);
  gl_Position = projection_matrix * view_matrix * position;
  Position = position.xyz;
  Normal = normal;
}
