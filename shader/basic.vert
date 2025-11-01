#version 400 core
layout (location = 0) in vec3 aPos;

uniform mat4 uMVP;
uniform vec3 uColor;

out vec3 vColor;

void main() {
    vColor = uColor;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
