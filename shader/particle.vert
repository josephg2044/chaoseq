#version 400 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aPhase;

uniform mat4 uView;
uniform mat4 uProj;
uniform float uPointSize;
uniform float uTime;
uniform float uColorSpeed;
uniform int uMonochrome;

out vec3 vColor;

vec3 computeColor(vec3 pos, float phase, float time, float colorSpeed) {
    float radius = length(pos);
    vec3 dir = radius > 1e-5 ? normalize(pos) : vec3(1.0, 0.0, 0.0);
    float hue = phase + time * colorSpeed;
    vec3 wave = dir * 2.0 + vec3(radius * 0.12);
    float r = 0.5 + 0.5 * sin(hue + wave.x);
    float g = 0.5 + 0.5 * sin(hue + wave.y + 2.0943951);
    float b = 0.5 + 0.5 * sin(hue + wave.z + 4.1887902);
    vec3 base = vec3(r, g, b);
    float glow = clamp(radius * 0.02, 0.0, 1.0);
    return mix(base, vec3(1.0), glow * 0.25);
}

void main() {
    vec3 color = computeColor(aPos, aPhase, uTime, uColorSpeed);
    if (uMonochrome != 0) {
        color = vec3(1.0);
    }
    vColor = color;
    vec4 viewPos = uView * vec4(aPos, 1.0);
    float dist = length(viewPos.xyz);
    float attenuation = 15.0 / (dist + 5.0);
    float size = uPointSize * attenuation;
    gl_PointSize = clamp(size, 1.0, 72.0);
    gl_Position = uProj * viewPos;
}
