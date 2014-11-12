#version 130

uniform sampler2D texture;
uniform vec4 color;

in vec2 TexCoord;

out vec4 outColor;

void main() {
  float alpha = texture2D(texture, TexCoord).r;
  if (alpha > 0) {
    outColor = color * alpha;
  } else {
     discard;
  }
}
