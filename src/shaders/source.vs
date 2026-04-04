#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 UVcoord;
out vec3 ourColor;
out vec3 ourSpecular;
flat out vec3 flatColor;
flat out vec3 flatSpecular;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int mode;

uniform vec3 lightPosWorld;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;

const int PHONG_MODE = 0;
const int NORMAL_MODE  = 1;
const int Z_MODE    = 2;
const int FLAT_MODE    = 3;
const int GOURAUD_MODE  = 4;
const int ZPRIME_MODE  = 5;

out vec3 eyePos;
out vec3 N;

void main()
{
    UVcoord = aTexCoord;
    ourSpecular = vec3(0.0);

    // object to eye space
    mat4 modelView = view * model;
    eyePos  = vec3(modelView * vec4(aPos, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(modelView)));
    N = normalize(normalMatrix * aNormal);

    // eye to clip
    gl_Position = projection * vec4(eyePos, 1.0);

    if (mode == NORMAL_MODE) {

        ourColor = aNormal * 0.5 + 0.5;

    } else if (mode == Z_MODE) {

        float near = 0.1;
        float far = 100.0;

        float depth = eyePos.z;

        // 0 to 1
        float z = (depth - near) / (far - near);

        ourColor = vec3(1 - z);

    } else if (mode == ZPRIME_MODE) {

        ourColor = vec3(0.0);

    } else if (mode == GOURAUD_MODE || mode == FLAT_MODE) {

        vec3 lightPosEye = vec3(view * vec4(lightPosWorld, 1.0));
        vec3 L = normalize(lightPosEye - eyePos);
        vec3 V = normalize(-eyePos);
        vec3 H = normalize(L + V);

        vec3 ambient = ambientStrength * ambientColor;
        vec3 diffuse = diffuseStrength * max(dot(N, L), 0.0) * lightColor;
        vec3 specular = specularStrength * pow(max(dot(N, H), 0.0), shininess) * lightColor;

        ourColor = ambient + diffuse;
        ourSpecular = specular;

        flatColor = ambient + diffuse;
        flatSpecular = specular;

    } else {

        ourColor = vec3(1.0);

    }
}

// Reference: https://github.com/ruange/Gouraud-Shading-and-Phong-Shading