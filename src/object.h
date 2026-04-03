#pragma once

#include "texture.h"
#include "tiny_obj_loader.h"
#include "camera.h"

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>

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

    struct textureBatch {
        int materialId = -1;
        unsigned int indexOffset = 0;
        unsigned int indexCount = 0;
    };

    std::vector<textureBatch> textureBatches;
    std::vector<Texture*> textures;

    bool loadObj(const std::string& fileName) {
        tinyobj::ObjReader reader;
        tinyobj::ObjReaderConfig config;
        config.triangulate = true;

        std::string objFile = fileName + ".obj";
        std::string mtlFile = fileName + ".mtl";
        config.mtl_search_path = fileName.substr(0, fileName.find_last_of("/\\") + 1); // set search path to the directory of the obj file

        if (!reader.ParseFromFile(objFile, config)) {
            std::cerr << "Failed to load obj: " << objFile << "\n";
            if (!reader.Error().empty()) std::cerr << reader.Error() << "\n";
            return false;
        }

        const tinyobj::attrib_t& attrib = reader.GetAttrib();
        const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
        const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();

        if (attrib.vertices.empty() || shapes.empty()) {
            return false;
        }

        bool hasAnyTexturedMaterial = false;
        bool hasAnyTexcoords = false;

        textures.assign(materials.size(), nullptr);
        for (size_t i = 0; i < materials.size(); ++i) {
            if (!materials[i].diffuse_texname.empty()) {
            hasAnyTexturedMaterial = true;
                std::string texturePath = config.mtl_search_path + materials[i].diffuse_texname;
                textures[i] = new Texture(GL_TEXTURE_2D, texturePath);
                if (!textures[i]->load()) {
                    std::cerr << "Failed to load texture: " << texturePath << "\n";
                    delete textures[i];
                    textures[i] = nullptr;
                }
            }
        }

        std::map<int, std::vector<unsigned int>> materialToIndices;

        for (const tinyobj::shape_t& shape : shapes) {
            size_t shapeIndexOffset = 0;

            for (size_t faceIndex = 0; faceIndex < shape.mesh.num_face_vertices.size(); faceIndex++) {
                int faceVertexCount = shape.mesh.num_face_vertices[faceIndex];

                // only accept triangles.
                if (faceVertexCount != 3) {
                    shapeIndexOffset += faceVertexCount;
                    continue;
                }

                int materialId = -1;
                if (faceIndex < shape.mesh.material_ids.size()) {
                    materialId = shape.mesh.material_ids[faceIndex];
                }

                for (int corner = 0; corner < 3; ++corner) {
                    tinyobj::index_t meshIndex = shape.mesh.indices[shapeIndexOffset + corner];

                    if (meshIndex.vertex_index < 0) {
                        continue;
                    }

                    int positionIndex = meshIndex.vertex_index;
                    int texcoordIndex = meshIndex.texcoord_index;
                    int normalIndex = meshIndex.normal_index;

                    glm::vec3 position(
                        attrib.vertices[3 * positionIndex + 0],
                        attrib.vertices[3 * positionIndex + 1],
                        attrib.vertices[3 * positionIndex + 2]
                    );

                    glm::vec2 texcoord(0.0f, 0.0f);
                    if (texcoordIndex >= 0 && attrib.texcoords.size() >= (2 * texcoordIndex + 2)) {
                        hasAnyTexcoords = true;
                        texcoord = glm::vec2(
                            attrib.texcoords[2 * texcoordIndex + 0],
                            attrib.texcoords[2 * texcoordIndex + 1]
                        );
                    }

                    glm::vec3 normal(0.0f, 0.0f, 0.0f);
                    if (normalIndex >= 0 && attrib.normals.size() >= (3 * normalIndex + 3)) {
                        normal = glm::vec3(
                            attrib.normals[3 * normalIndex + 0],
                            attrib.normals[3 * normalIndex + 1],
                            attrib.normals[3 * normalIndex + 2]
                        );
                    }
                    else { // create normal from triangle vertices if normal is missing
                        int vertexIndex1 = shape.mesh.indices[shapeIndexOffset + 0].vertex_index;
                        int vertexIndex2 = shape.mesh.indices[shapeIndexOffset + 1].vertex_index;
                        int vertexIndex3 = shape.mesh.indices[shapeIndexOffset + 2].vertex_index;

                        glm::vec3 v1(
                            attrib.vertices[3 * vertexIndex1 + 0],
                            attrib.vertices[3 * vertexIndex1 + 1],
                            attrib.vertices[3 * vertexIndex1 + 2]
                        );
                        glm::vec3 v2(
                            attrib.vertices[3 * vertexIndex2 + 0],
                            attrib.vertices[3 * vertexIndex2 + 1],
                            attrib.vertices[3 * vertexIndex2 + 2]
                        );
                        glm::vec3 v3(
                            attrib.vertices[3 * vertexIndex3 + 0],
                            attrib.vertices[3 * vertexIndex3 + 1],
                            attrib.vertices[3 * vertexIndex3 + 2]
                        );

                        normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));

                    }

                    vertexData.push_back(position.x);
                    vertexData.push_back(position.y);
                    vertexData.push_back(position.z);

                    vertexData.push_back(texcoord.x);
                    vertexData.push_back(texcoord.y);

                    vertexData.push_back(normal.x);
                    vertexData.push_back(normal.y);
                    vertexData.push_back(normal.z);

                    unsigned int newIndex = static_cast<unsigned int>((vertexData.size() / 8) - 1);
                    materialToIndices[materialId].push_back(newIndex);
                }

                shapeIndexOffset += 3;
            }
        }

        indexData.clear();
        textureBatches.clear();

        for (auto& batch : materialToIndices) {
            textureBatch newBatch;
            newBatch.materialId = batch.first;
            newBatch.indexOffset = static_cast<unsigned int>(indexData.size());
            newBatch.indexCount = static_cast<unsigned int>(batch.second.size());
            indexData.insert(indexData.end(), batch.second.begin(), batch.second.end());
            textureBatches.push_back(newBatch);
        }

        if (hasAnyTexturedMaterial && !hasAnyTexcoords) {
            std::cerr << "Model '" << fileName << "' has texture maps in MTL but no usable vt coordinates. "
                      << "Using generated planar UV fallback." << "\n";
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

        for (const auto& batch : textureBatches) {
            if (batch.materialId >= 0 && batch.materialId < textures.size() && textures[batch.materialId] != nullptr) {
                textures[batch.materialId]->bind(GL_TEXTURE0);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0); // unbind texture if material has no valid texture
            }

            glDrawElements(GL_TRIANGLES, batch.indexCount, GL_UNSIGNED_INT, (void*)(batch.indexOffset * sizeof(unsigned int)));
        }

        glBindVertexArray(0);
    }

    ~Object() {
        for (auto& tex : textures) {
            if (tex) {
                delete tex;
            }
        }

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