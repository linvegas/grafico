#version 450

in vec2  vertexPos;
in vec2  vertexUv;
in vec3  vertexColor;
in float vertexTexId;

out vec2  fragUv;
out vec4  fragColor;
out float fragTexId;

uniform mat4x4 projection;

void main()
{
    gl_Position = projection * vec4(vertexPos, 0.0, 1.0);
    fragColor = vec4(vertexColor, 1.0);
    fragUv = vertexUv;
    fragTexId = vertexTexId;
}
