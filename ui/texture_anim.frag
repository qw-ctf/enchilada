#version 440

layout(location=0) in vec2 qt_TexCoord0;
layout(location=0) out vec4 fragColor;

layout(std140, binding=0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    float singleFrameWidth;  // Width of a single frame in texture coordinate space
    float time;
} ubuf;

layout(binding=1) uniform sampler2D source;

void main() {
    float frameIndex = floor(ubuf.time);
    float s = mod(frameIndex * ubuf.singleFrameWidth, 1.0) + (qt_TexCoord0.s * ubuf.singleFrameWidth);
    fragColor = texture(source, fract(vec2(s, qt_TexCoord0.t))) * ubuf.qt_Opacity;
}
