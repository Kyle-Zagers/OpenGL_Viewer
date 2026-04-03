#pragma once

#include "texture.h"
#include "meshHandler.h"
#include "camera.h"

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <istream>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Object {
private:
    unsigned int vertexArrayObject = 0;
    unsigned int vertexBufferObject = 0;
    unsigned int elementBufferObject = 0;

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    float ambientStrength = 0.5f;
    float diffuseStrength = 0.5f;
    float specularStrength = 0.5f;
    float shininess = 32.0f;

    // position(3), texture(2), normal(3)
    std::vector<float> vertexData;
    std::vector<unsigned int> indexData;

    MeshHandler meshHandler = MeshHandler();


    bool loadObj(const std::string& fileName) {
        if (!meshHandler.loadObject(fileName)) {
            return false;
        }
        const std::unordered_map<std::string, MaterialInfo>& materials = meshHandler.getMaterials();

        vertexData.clear();
        vertexData = meshHandler.getVertexData();

        indexData.clear();
        indexData = meshHandler.getIndexData();

        return true;
    }

    void uploadToGpu() {
        glGenVertexArrays(1, &vertexArrayObject);
        glGenBuffers(1, &vertexBufferObject);
        glGenBuffers(1, &elementBufferObject);

        glBindVertexArray(vertexArrayObject);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned int), indexData.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

public:
    Object(std::string filePath, glm::vec3 position, float uniformScale = 1.0f) {
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(uniformScale));
        if (loadObj(filePath)) {
            uploadToGpu();
        }
        else {
            std::cerr << "Failed to load object from file: " << filePath << "\n";
        }
    }

    void changePosition(glm::vec3 newPosition) {
        modelMatrix = glm::translate(modelMatrix, newPosition);
    }

    void applyTransformation(glm::mat4 transform) {
        modelMatrix = transform * modelMatrix;
    }

    glm::mat4 getModelMatrix() const {
        return modelMatrix;
    }

    void draw(unsigned int shaderProgram, Camera& camera) {
        glBindVertexArray(vertexArrayObject);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
        
        glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), ambientStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "diffuseStrength"), diffuseStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), specularStrength);
        glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), shininess);

        glUniform1i(glGetUniformLocation(shaderProgram, "uTex"), 0);


        for (const auto& materialObject : meshHandler.getMaterials()) {
            const MaterialInfo& material = materialObject.second;

            if (material.texture) {
                material.texture->bind(GL_TEXTURE0);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0); // unbind texture if material has no valid texture
                if (material.r >= 0.0f && material.g >= 0.0f && material.b >= 0.0f) {
                    glUniform3f(glGetUniformLocation(shaderProgram, "color"), material.r, material.g, material.b);
                }
                else {
                    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 1.0f, 1.0f);
                }
            }

            for (const auto& group : material.groups) {
                glDrawElements(GL_TRIANGLES, group.second, GL_UNSIGNED_INT, (void*)(group.first * sizeof(unsigned int)));
            }
        }

        glBindVertexArray(0);
    }

    ~Object() {

        if (vertexArrayObject) {
            glDeleteVertexArrays(1, &vertexArrayObject);
        }
        if (vertexBufferObject) {
            glDeleteBuffers(1, &vertexBufferObject);
        }
        if (elementBufferObject) {
            glDeleteBuffers(1, &elementBufferObject);
        }
    }
};

// Reference: https://learnopengl.com/Model-Loading/Model