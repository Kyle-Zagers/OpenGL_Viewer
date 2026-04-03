#pragma once

#include "texture.h"
#include "meshHandler.h"
#include "camera.h"

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <istream>
#include <unordered_map>

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

    // position(3), color(3), normal(3)
    std::vector<float> vertexData;
    std::vector<unsigned int> indexData;

    struct DrawBatch {
        Texture* texture = nullptr;
        unsigned int indexOffset = 0;
        unsigned int indexCount = 0;
    };

    std::vector<DrawBatch> drawBatches;

    MeshHandler meshHandler = MeshHandler();


    bool loadObj(const std::string& fileName) {
        vertexData.clear();
        indexData.clear();
        drawBatches.clear();

        if (!meshHandler.loadObject(fileName)) {
            return false;
        }

        const std::unordered_map<std::string, MaterialInfo>& materials = meshHandler.getMaterials();

        for (const auto& materialObject : materials) {
            const MaterialInfo& material = materialObject.second;

            DrawBatch batch;
            batch.texture = material.texture;
            batch.indexOffset = static_cast<unsigned int>(indexData.size());

            for (const face& f : material.faces) {
                for (int corner = 0; corner < 3; ++corner) {
                    vertexData.push_back(f.vertices[corner * 3 + 0]);
                    vertexData.push_back(f.vertices[corner * 3 + 1]);
                    vertexData.push_back(f.vertices[corner * 3 + 2]);

                    vertexData.push_back(f.textures[corner * 2 + 0]);
                    vertexData.push_back(f.textures[corner * 2 + 1]);

                    vertexData.push_back(f.normals[corner * 3 + 0]);
                    vertexData.push_back(f.normals[corner * 3 + 1]);
                    vertexData.push_back(f.normals[corner * 3 + 2]);

                    indexData.push_back(static_cast<unsigned int>((vertexData.size() / 8) - 1));
                }
            }

            batch.indexCount = static_cast<unsigned int>(indexData.size()) - batch.indexOffset;
            if (batch.indexCount > 0) {
                drawBatches.push_back(batch);
            }
        }

        return !vertexData.empty() && !indexData.empty();
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

        for (const DrawBatch& batch : drawBatches) {
            if (batch.texture) {
                batch.texture->bind(GL_TEXTURE0);
            } else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            glDrawElements(
                GL_TRIANGLES,
                static_cast<GLsizei>(batch.indexCount),
                GL_UNSIGNED_INT,
                (void*)(batch.indexOffset * sizeof(unsigned int))
            );
        }

        glBindVertexArray(0);
    }

    ~Object() {
        // for (auto& tex : textures) {
        //     if (tex) {
        //         delete tex;
        //     }
        // }

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