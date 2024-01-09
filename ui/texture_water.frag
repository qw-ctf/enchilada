#version 440

layout(location=0) in vec2 qt_TexCoord0;

layout(location=0) out vec4 fragColor;

layout(std140, binding=0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    float time;
} ubuf;

layout(binding=1) uniform sampler2D source;

void main() {
    float s = qt_TexCoord0.s + sin((qt_TexCoord0.t + ubuf.time) * 1.5) * 0.125;
    float t = qt_TexCoord0.t + sin((qt_TexCoord0.s + ubuf.time) * 1.5) * 0.125;

    fragColor = texture(source, fract(vec2(s, t))) * ubuf.qt_Opacity;
}