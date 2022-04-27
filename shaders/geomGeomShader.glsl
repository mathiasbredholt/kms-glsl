#version 300 es

precision mediump float;

layout(lines) in;
layout(triangle_strip, max_vertices = 6) out;

out vec3 fColor;

uniform mat4 projection;
uniform vec2 viewportSize;
uniform float lineWidth;

vec3 rgb2hsv(vec3 c) {
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
  float u_width = viewportSize[0];
  float u_height = viewportSize[1];


  vec2 p1 = gl_in[0].gl_Position.xy;
  vec2 p2 = gl_in[1].gl_Position.xy;
  vec2 tangent = normalize(p2 - p1);
  vec2 normal = vec2(tangent.y / u_width, -tangent.x / u_height) * lineWidth;

  vec2 quad[4] = vec2[](p1 + normal, p1 - normal, p2 + normal, p2 - normal);

  // Create first triangle
  gl_Position = vec4(quad[0], 0, 1);
  fColor = hsv2rgb(vec3(0.5 + 0.2 * quad[0].y, 1.0, 1.0));
  EmitVertex();
  gl_Position = vec4(quad[1], 0, 1);
  fColor = hsv2rgb(vec3(0.5 + 0.2 * quad[1].y, 1.0, 1.0));
  EmitVertex();
  gl_Position = vec4(quad[2], 0, 1);
  fColor = hsv2rgb(vec3(0.5 + 0.2 * quad[2].y, 1.0, 1.0));
  EmitVertex();
  EndPrimitive();

  // Create second triangle
  gl_Position = vec4(quad[1], 0, 1);
  fColor = hsv2rgb(vec3(0.5 + 0.2 * quad[1].y, 1.0, 1.0));
  EmitVertex();
  gl_Position = vec4(quad[2], 0, 1);
  fColor = hsv2rgb(vec3(0.5 + 0.2 * quad[2].y, 1.0, 1.0));
  EmitVertex();
  gl_Position = vec4(quad[3], 0, 1);
  fColor = hsv2rgb(vec3(0.5 + 0.2 * quad[3].y, 1.0, 1.0));
  EmitVertex();
  EndPrimitive();
}
