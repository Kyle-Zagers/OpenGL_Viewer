#version 330 core

uniform mat4 view;
uniform vec3 lightPosWorld;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform vec3 ambientColor;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;

in vec3 ourColor;
in vec3 eyePos;
in vec3 N;

out vec4 FragColor;

uniform int mode;

const int PHONG_MODE   = 0;
const int NORMAL_MODE  = 1;
const int ZBUF_MODE    = 2;
const int FLAT_MODE    = 3;
const int GOURAUD_MODE = 4;

void main()
{
    vec3 finalColor = ourColor;

    if (mode == PHONG_MODE) {
        vec3 normal = normalize(N); // interpolated normals must be renormalized

        vec3 lightPosEye = vec3(view * vec4(lightPosWorld, 1.0));
        vec3 L = normalize(lightPosEye - eyePos);
        vec3 V = normalize(-eyePos);
        vec3 H = normalize(L + V);

        float lambert = max(dot(normal, L), 0.0);
        float spec = 0.0;
        if (lambert > 0.0) {
            spec = pow(max(dot(normal, H), 0.0), shininess);
        }

        vec3 ambient  = ambientStrength  * ambientColor;
        vec3 diffuse  = diffuseStrength  * lambert * lightColor;
        vec3 specular = specularStrength * spec    * lightColor;

        finalColor = (ambient + diffuse) * ourColor + specular; // no "vec3" here
    }

    FragColor = vec4(finalColor, 1.0);
}