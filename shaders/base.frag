#version 130

in vec3 Color;
in vec3 Position;
in vec3 Normal;
out vec4 outputF;

const vec4 ambient = vec4(0.1, 0.1, 0.1, 1);
const vec4 light_color = vec4(1.0, 1.0, 1.0, 1.0);
const vec3 light_position = vec3(0, 0, 0);
const float k_diffuse = 1.0;
const float k_ambient = 0.8;

void main(void) {
  vec3 l_m = normalize(light_position - Position);
  vec4 phong = (ambient * k_ambient) +
               (k_diffuse * (dot(l_m, Normal)) * light_color * vec4(Color, 1));
  outputF = vec4(phong.rgb, 1);
}
