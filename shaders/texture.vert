#version 130

uniform mat4 view_matrix, projection_matrix;

in vec4 position;
in vec2 texcoord;

out vec2 TexCoord;
out vec3 Position;

void main() {
  gl_Position = projection_matrix * view_matrix * position;
  TexCoord = texcoord;
  Position = position.xyz;
}
