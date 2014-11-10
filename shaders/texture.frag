#version 130

uniform sampler2D texture;

in vec2 TexCoord;
in vec3 Position;
out vec4 outputF;

void main() {
  outputF = texture2D(texture, TexCoord);
}
