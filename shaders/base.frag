#version 130

in vec3 Color;
in vec3 Position;
//out vec4 outputF;

void main(void) {
//  outputF = vec4(Color, 1.0);
  gl_FragColor = vec4(Position / 256, 1.0);
//  gl_FragColor = vec4(1, 1, 1, 1);
}
