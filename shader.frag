#version 450

in vec4  fragColor;
in vec2  fragUv;
in float fragTexId;

out vec4 finalColor;

uniform sampler2D tex[32];

void main()
{
    int idx = int(fragTexId);

    if (idx == 0) {
        finalColor = fragColor;
    } else if (idx == 1) {
        finalColor = vec4(1, 1, 1, texture(tex[idx], fragUv).r) * fragColor;
    } else {
        finalColor = texture(tex[idx], fragUv) * fragColor;
    }
}
