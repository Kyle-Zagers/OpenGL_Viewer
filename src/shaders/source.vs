#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int mode; // 0=default, 1=normal, 2=z-buffer view

uniform vec3 lightPosWorld;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;

const int PHONG_MODE = 0;
const int NORMAL_MODE  = 1;
const int ZBUF_MODE    = 2;
const int FLAT_MODE    = 3;
const int GOURAUD_MODE  = 4;

out vec3 eyePos;
out vec3 N;

void main()
{
    // object -> eye space
    mat4 modelView = view * model;
    eyePos  = vec3(modelView * vec4(aPos, 1.0));

    mat3 normalMatrix = transpose(inverse(mat3(modelView)));
    N = normalize(normalMatrix * aNormal);

    // eye -> clip
    gl_Position = projection * vec4(eyePos, 1.0);

    if (mode == NORMAL_MODE) {

        ourColor = N * 0.5 + 0.5;

    } else if (mode == ZBUF_MODE) {

        float d = clamp((eyePos.z) / 100.0, 0.0, 1.0);
        ourColor = vec3(d);

    } else if (mode == FLAT_MODE) {

        ourColor = aColor; // output same color for all vertices of the triangle

    } else if (mode == GOURAUD_MODE) {

        vec3 lightPosEye = vec3(view * vec4(lightPosWorld, 1.0));
        vec3 L = normalize(lightPosEye - eyePos);
        vec3 V = normalize(-eyePos);
        vec3 H = normalize(L + V);

        float lambert = max(dot(N, L), 0.0);
        float spec = 0.0;
        if (lambert > 0.0) {
            spec = pow(max(dot(N, H), 0.0), shininess);
        }

        vec3 ambient  = ambientStrength  * ambientColor;
        vec3 diffuse  = diffuseStrength  * lambert * lightColor;
        vec3 specular = specularStrength * spec    * lightColor;

        vec3 finalColor = (ambient + diffuse) * aColor + specular;

        ourColor = finalColor; // output interpolated by rasterizer

    } else {

        ourColor = aColor;

    }
}

// Reference: https://github.com/ruange/Gouraud-Shading-and-Phong-Shading