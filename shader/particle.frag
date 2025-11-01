#version 400 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(coord, coord);
    if (r2 > 1.0) discard;
    float alpha = 1.0 - smoothstep(0.6, 1.0, r2);
    FragColor = vec4(vColor, alpha);
}
