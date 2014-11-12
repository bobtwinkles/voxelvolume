#version 130

uniform mat4 transform;

in vec4 coordinate;

out vec2 TexCoord;

void main() {
  gl_Position = transform * vec4(coordinate.xy, 0, 1);
  TexCoord = coordinate.zw;
}
