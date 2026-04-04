// template based on material from learnopengl.com
// Adapted from helloTriangle.cpp by Kyle Zagers
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_FAILURE_USERMSG
#include "object.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, bool& rotate, int& mode, Camera& camera);

static Camera* camera = new Camera();

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

bool getShadersSource(std::string& vertexShaderCode, std::string& fragmentShaderCode)
{
    std::ifstream vertexShaderFile("../src/shaders/source.vs");
    std::ifstream fragmentShaderFile("../src/shaders/source.fs");

    if (!vertexShaderFile.is_open() || !fragmentShaderFile.is_open())
    {
        return false;
    }

    vertexShaderCode = std::string((std::istreambuf_iterator<char>(vertexShaderFile)), std::istreambuf_iterator<char>());
    fragmentShaderCode = std::string((std::istreambuf_iterator<char>(fragmentShaderFile)), std::istreambuf_iterator<char>());
    return true;
}

enum viewMode {
    PHONG,
    NORMAL,
    Z,
    FLAT,
    GOURAUD,
    ZPRIME
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "viewGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // // glew: load all OpenGL function pointers
    glewInit();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);


    // build and compile our shader program
    // ------------------------------------
    std::string vertexShaderCode;
    std::string fragmentShaderCode;
    if (!getShadersSource(vertexShaderCode, fragmentShaderCode))
    {
        std::cerr << "Failed to load shader source code." << std::endl;
        return -1;
    }
    
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    {
        const char* vertexShaderSource = vertexShaderCode.c_str();
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    }
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const char* fragmentShaderSource = fragmentShaderCode.c_str();
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    }
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glm::vec3 obj1Pos = glm::vec3(60.0f, 0.0f, 0.0f);
    glm::vec3 obj2Pos = glm::vec3(0.0f, 2.0f, 0.0f);
    glm::vec3 obj3Pos = glm::vec3(30.0f, 20.0f, 0.0f);
    
    std::vector<Object*> objects;
    objects.emplace_back(new Object("../src/models/sponza", obj1Pos, 0.01f));
    objects.emplace_back(new Object("../src/models/soccerball", obj2Pos));
    objects.emplace_back(new Object("../src/models/f-16", obj3Pos));
    auto axis = obj2Pos - obj1Pos;

    glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -obj1Pos);

    // orthonormal basis for rotation axis based on standard axis corresponding to smallest component of axis
    glm::vec3 basis1, basis2;
    axis = glm::normalize(axis);
    if (std::abs(axis.x) <= std::abs(axis.y) && std::abs(axis.x) <= std::abs(axis.z)) {
        basis1 = glm::normalize(glm::cross(axis, glm::vec3(1.0f, 0.0f, 0.0f)));
    } else if (std::abs(axis.y) <= std::abs(axis.x) && std::abs(axis.y) <= std::abs(axis.z)) {
        basis1 = glm::normalize(glm::cross(axis, glm::vec3(0.0f, 1.0f, 0.0f)));
    } else {
        basis1 = glm::normalize(glm::cross(axis, glm::vec3(0.0f, 0.0f, 1.0f)));
    }
    basis2 = glm::normalize(glm::cross(axis, basis1));

    glm::mat4 axisAlign = glm::mat4(
        axis.x, axis.y, axis.z, 0.0f,
        basis1.x, basis1.y, basis1.z, 0.0f,
        basis2.x, basis2.y, basis2.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    float angle = glm::radians(1.0f);
    glm::mat4 rotationAroundX = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cos(angle), -sin(angle), 0.0f,
        0.0f, sin(angle), cos(angle), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    glm::mat4 rotationMatrix = glm::inverse(translateToOrigin) * glm::inverse(axisAlign) * rotationAroundX * axisAlign * translateToOrigin;

    int fbWidth = 0;
    int fbHeight = 0;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    framebuffer_size_callback(window, fbWidth, fbHeight);

    bool rotate = false;
    int mode = PHONG;

    glm::vec3 lightPosWorld = glm::vec3(10.0f, 100.0f, 10.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window, rotate, mode, *camera);

        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "mode"), mode);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPosWorld"), 1, &lightPosWorld[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, &lightColor[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "ambientColor"), 1, &ambientColor[0]);


        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto& obj : objects) {
            if (rotate) {
                obj->applyTransformation(rotationMatrix);
            }

            obj->draw(shaderProgram, *camera);
        }
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteProgram(shaderProgram);

    for (auto& obj : objects) {
        delete obj;
    }

    delete camera;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, bool& rotate, int& mode, Camera& camera)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        mode = Z;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        mode = NORMAL;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        mode = PHONG;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        mode = FLAT;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        mode = GOURAUD;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        mode = ZPRIME;

    // move camera
    float offset = 0.1f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveForward(offset);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveBackward(offset);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveLeft(offset);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveRight(offset);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.moveUp(offset);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.moveDown(offset);
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        rotate = !rotate;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        camera.makeOrthographic();
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        camera.makePerspective();
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        camera.zoomIn();
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        camera.zoomOut();

    // camera rotation
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.lookDown(1.0f);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.lookUp(1.0f);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.lookRight(1.0f);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.lookLeft(1.0f);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    if (camera && height > 0) {
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        camera->setPerspectiveFromFov(60.0f, aspect, 0.1f, 500.0f);
    }
}