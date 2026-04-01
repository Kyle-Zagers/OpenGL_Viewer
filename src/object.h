#include "meshHandler.h"
#include "camera.h"
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma once

class Object {
    MeshHandler meshHandler;
    unsigned int VAO, VBO, EBO;
    bool useEBO;
    glm::mat4 model = glm::mat4(1.0f);

    public:
    Object(std::string fileName, glm::vec3 position, float r = 1.0f, float g = 1.0f, float b = 1.0f, bool _useEBO = false, bool randColor = false) {
        useEBO = _useEBO;
        meshHandler.loadObj(fileName, r, g, b, randColor);
        model = glm::translate(model, position);

        if (useEBO) {

            const std::vector<float>& vertices = meshHandler.getVertices();
            const std::vector<unsigned int>& indices = meshHandler.getIndices();

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
            glBindVertexArray(0); // Unbind VAO
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO

        }
        else {

            const std::vector<float>& vertices = meshHandler.getTriangles();

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
            glBindVertexArray(0); // Unbind VAO

        }

    }

    void changePosition(glm::vec3 newPosition) {
        model = glm::translate(model, newPosition);
    }

    void applyTransformation(glm::mat4 transformationMatrix) {
        model = transformationMatrix * model;
    }

    glm::mat4 getModelMatrix() const {
        return model;
    }

    void draw(unsigned int shaderProgram, bool cpuTransform, Camera& camera, bool zBufferView = false) {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        if (cpuTransform) {
            std::vector<float> transformed = useEBO ? meshHandler.getVertices() : meshHandler.getTriangles();
            // glm::mat4 mvp = projection * glm::translate(view, -cameraPos) * model;

            for (size_t i = 0; i < transformed.size(); i += 6) {
                glm::vec4 v(transformed[i], transformed[i + 1], transformed[i + 2], 1.0f);
                glm::vec4 clip = camera.clip(model, v);
                float invW = (clip.w != 0.0f) ? (1.0f / clip.w) : 1.0f;
                transformed[i]     = clip.x * invW;
                transformed[i + 1] = clip.y * invW;
                transformed[i + 2] = clip.z * invW;
            }

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            glUniform1i(glGetUniformLocation(shaderProgram, "zBufferView"), zBufferView ? 1 : 0);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, transformed.size() * sizeof(float), transformed.data(), GL_DYNAMIC_DRAW);
        } else {
            // restore original mesh buffer so GPU path gets object-space vertices
            const std::vector<float>& original = useEBO ? meshHandler.getVertices() : meshHandler.getTriangles();
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, original.size() * sizeof(float), original.data(), GL_STATIC_DRAW);

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
            glUniform1i(glGetUniformLocation(shaderProgram, "zBufferView"), zBufferView ? 1 : 0);
        }

        if (useEBO) {
            glDrawElements(GL_TRIANGLES, meshHandler.getIndices().size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, meshHandler.getTriangles().size() / 6);
        }
        glBindVertexArray(0);
    }

    ~Object() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
};