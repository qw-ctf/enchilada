#version 440

layout(location=0) in vec2 qt_TexCoord0;

layout(location=0) out vec4 fragColor;

layout(std140, binding=0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    float time;
} ubuf;

layout(binding=1) uniform sampler2D skyTexture;
layout(binding=2) uniform sampler2D cloudTexture;

void main() {
    float sky_s = qt_TexCoord0.s / 2.0 + ubuf.time;
    float sky_t = qt_TexCoord0.t / 2.0 + ubuf.time;

    float cloud_s = qt_TexCoord0.s / 2.0 + ubuf.time * 2.0;
    float cloud_t = qt_TexCoord0.t / 2.0 + ubuf.time * 2.0;

    vec4 skyColor = texture(skyTexture, fract(vec2(sky_s, sky_t)));
    vec4 cloudColor = texture(cloudTexture, fract(vec2(cloud_s, cloud_t)));

    fragColor = mix(skyColor, cloudColor, cloudColor.a) * ubuf.qt_Opacity;
}