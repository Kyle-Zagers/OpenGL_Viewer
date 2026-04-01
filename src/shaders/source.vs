// const char *vertexShaderSource = "#version 330 core\n"
    // "layout (location = 0) in vec3 aPos;\n"
    // "void main()\n"
    // "{\n"
    // "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    // "}\0";

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool zBufferView;

void main()
{
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    gl_Position = projection * viewPos;
    ourColor = zBufferView ? vec3(viewPos.z / 100.0) : aColor;
}