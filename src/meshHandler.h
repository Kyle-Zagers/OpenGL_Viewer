#include <vector>
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "texture.h"

#pragma once

struct face {
    unsigned int vertexIndices[3];
    unsigned int textureIndices[3];
    unsigned int normalIndices[3];

    float vertices[9];
    float textures[6];
    float normals[9];
};

struct MaterialInfo {
    Texture* texture = nullptr;
    std::vector<face> faces;
};

class MeshHandler {
private:
    std::vector<float> vertices;
    std::vector<float> vertexTexture;
    std::vector<float> vertexNormal;

    // position(3), texture(2), normal(3)
    std::vector<float> vertexData;
    std::vector<unsigned int> indices;

    std::unordered_map<std::string, MaterialInfo> materials;

    bool parseMtl(std::string filePath) {
        std::ifstream mtlFile(filePath);
        if (!mtlFile.is_open()) {
            std::cerr << "Failed to open MTL file: " << filePath << "\n";
            return false;
        }

        std::string line;
        std::vector<Texture*> textures;
        while (std::getline(mtlFile, line)) {
            if (line.substr(0, 7) == "newmtl ") {

                MaterialInfo newMaterial;
                std::string materialName = line.substr(7);

                std::streampos currentPos = mtlFile.tellg();

                while (std::getline(mtlFile, line) && line.substr(0, 7) != "newmtl ") {

                    if (line.substr(0, 8) == "\tmap_Kd ") {

                        std::string textureFile = "../src/models/" + line.substr(8);
                        Texture* tex = new Texture(GL_TEXTURE_2D, textureFile);

                        if (tex->load()) {
                            newMaterial.texture = tex;
                        }
                        else {
                            delete tex;
                        }

                    }

                    currentPos = mtlFile.tellg();
                }

                materials[materialName] = newMaterial;

                mtlFile.seekg(currentPos);

            }
        }

        return true;
    }

    bool parseObj(std::string filePath) {
        std::ifstream objFile(filePath);
        if (!objFile.is_open()) {
            std::cerr << "Failed to open OBJ file: " << filePath << "\n";
            return false;
        }

        std::string line;
        while (std::getline(objFile, line)) {

            std::stringstream ss(line);
            std::string lineType;
            ss >> lineType;

            if (lineType == "v") {
                float x, y, z;
                ss >> x >> y >> z;
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
            else if (lineType == "usemtl") {
                std::string materialName;
                ss >> materialName;
                
                while (std::getline(objFile, line)) {
                    ss = std::stringstream(line);
                    ss >> lineType;

                    if (lineType != "f" && lineType != "s" && lineType != "g") {
                        break;
                    }

                    std::vector<unsigned int> vertexIndices;
                    std::vector<unsigned int> textureIndices;
                    std::vector<unsigned int> normalIndices;

                    if (lineType == "f") {
                        std::string vertex;
                        
                        while (ss >> vertex) {
                            size_t slash = vertex.find('/');
                            std::string vertexIndexStr = (slash == std::string::npos) ? vertex : vertex.substr(0, slash);
                            if (vertexIndexStr.empty()) {
                                continue;
                            }

                            int vertexIndex = std::stoi(vertexIndexStr);
                            int vertexCount = vertices.size() / 3;
                            if (vertexIndex < 0) {
                                vertexIndex = vertexCount + vertexIndex;
                            }
                            else {
                                vertexIndex -= 1;
                            }
                            vertexIndices.push_back(vertexIndex);

                            vertexData.push_back(vertices[vertexIndex * 3]);
                            vertexData.push_back(vertices[vertexIndex * 3 + 1]);
                            vertexData.push_back(vertices[vertexIndex * 3 + 2]);

                            if (slash == std::string::npos) {
                                vertexData.push_back(0.0f);
                                vertexData.push_back(0.0f);

                                vertexData.push_back(0.0f);
                                vertexData.push_back(0.0f);
                                vertexData.push_back(0.0f);

                                continue;
                            }

                            size_t slash2 = vertex.find('/', slash + 1);

                            if (slash2 == std::string::npos) {
                                std::string textureIndexStr = vertex.substr(slash + 1);
                                if (!textureIndexStr.empty()) {
                                    int textureIndex = std::stoi(textureIndexStr);
                                    int textureCount = vertexTexture.size() / 2;
                                    if (textureIndex < 0) {
                                        textureIndex = textureCount + textureIndex;
                                    }
                                    else {
                                        textureIndex -= 1;
                                    }
                                    textureIndices.push_back(textureIndex);

                                    vertexData.push_back(vertexTexture[textureIndex * 2]);
                                    vertexData.push_back(vertexTexture[textureIndex * 2 + 1]);
                                } else {
                                    vertexData.push_back(0.0f);
                                    vertexData.push_back(0.0f);
                                }

                                vertexData.push_back(0.0f);
                                vertexData.push_back(0.0f);
                                vertexData.push_back(0.0f);

                                continue;
                            }

                            if (slash2 == slash + 1) {
                                std::string normalIndexStr = vertex.substr(slash2 + 1);
                                if (!normalIndexStr.empty()) {
                                    int normalIndex = std::stoi(normalIndexStr);
                                    int normalCount = vertexNormal.size() / 3;
                                    if (normalIndex < 0) {
                                        normalIndex = normalCount + normalIndex;
                                    }
                                    else {
                                        normalIndex -= 1;
                                    }
                                    normalIndices.push_back(normalIndex);

                                    vertexData.push_back(0.0f);
                                    vertexData.push_back(0.0f);

                                    vertexData.push_back(vertexNormal[normalIndex * 3]);
                                    vertexData.push_back(vertexNormal[normalIndex * 3 + 1]);
                                    vertexData.push_back(vertexNormal[normalIndex * 3 + 2]);

                                    continue;
                                }
                            }

                            std::string textureIndexStr = vertex.substr(slash + 1, slash2 - slash - 1);
                            if (!textureIndexStr.empty()) {
                                int textureIndex = std::stoi(textureIndexStr);
                                int textureCount = vertexTexture.size() / 2;
                                if (textureIndex < 0) {
                                    textureIndex = textureCount + textureIndex;
                                }
                                else {
                                    textureIndex -= 1;
                                }
                                textureIndices.push_back(textureIndex);
                                vertexData.push_back(vertexTexture[textureIndex * 2]);
                                vertexData.push_back(vertexTexture[textureIndex * 2 + 1]);
                            }
                            else {
                                vertexData.push_back(0.0f);
                                vertexData.push_back(0.0f);
                            }
                            std::string normalIndexStr = vertex.substr(slash2 + 1);
                            if (!normalIndexStr.empty()) {
                                int normalIndex = std::stoi(normalIndexStr);
                                int normalCount = vertexNormal.size() / 3;
                                if (normalIndex < 0) {
                                    normalIndex = normalCount + normalIndex;
                                }
                                else {
                                    normalIndex -= 1;
                                }
                                normalIndices.push_back(normalIndex);
                                vertexData.push_back(vertexNormal[normalIndex * 3]);
                                vertexData.push_back(vertexNormal[normalIndex * 3 + 1]);
                                vertexData.push_back(vertexNormal[normalIndex * 3 + 2]);
                            }
                            else {
                                vertexData.push_back(0.0f);
                                vertexData.push_back(0.0f);
                                vertexData.push_back(0.0f);
                            }

                        }

                        if (vertexIndices.size() >= 3) {
                            for (size_t i = 1; i < vertexIndices.size() - 1; i++) {
                                materials[materialName].faces.push_back({
                                    {vertexIndices[0], vertexIndices[i], vertexIndices[i + 1]},
                                    {textureIndices.size() > 0 ? textureIndices[0] : 0, textureIndices.size() > i ? textureIndices[i] : 0, textureIndices.size() > i + 1 ? textureIndices[i + 1] : 0},
                                    {normalIndices.size() > 0 ? normalIndices[0] : 0, normalIndices.size() > i ? normalIndices[i] : 0, normalIndices.size() > i + 1 ? normalIndices[i + 1] : 0},
                                    {vertices[vertexIndices[0] * 3], vertices[vertexIndices[0] * 3 + 1], vertices[vertexIndices[0] * 3 + 2],
                                     vertices[vertexIndices[i] * 3], vertices[vertexIndices[i] * 3 + 1], vertices[vertexIndices[i] * 3 + 2],
                                     vertices[vertexIndices[i + 1] * 3], vertices[vertexIndices[i + 1] * 3 + 1], vertices[vertexIndices[i + 1] * 3 + 2]},
                                    {textureIndices.size() > 0 ? vertexTexture[textureIndices[0] * 2] : 0.0f, textureIndices.size() > 0 ? vertexTexture[textureIndices[0] * 2 + 1] : 0.0f,
                                     textureIndices.size() > i ? vertexTexture[textureIndices[i] * 2] : 0.0f, textureIndices.size() > i ? vertexTexture[textureIndices[i] * 2 + 1] : 0.0f,
                                     textureIndices.size() > i + 1 ? vertexTexture[textureIndices[i + 1] * 2] : 0.0f, textureIndices.size() > i + 1 ? vertexTexture[textureIndices[i + 1] * 2 + 1] : 0.0f},
                                    {normalIndices.size() > 0 ? vertexNormal[normalIndices[0] * 3] : 0.0f, normalIndices.size() > 0 ? vertexNormal[normalIndices[0] * 3 + 1] : 0.0f, normalIndices.size() > 0 ? vertexNormal[normalIndices[0] * 3 + 2] : 0.0f,
                                     normalIndices.size() > i ? vertexNormal[normalIndices[i] * 3] : 0.0f, normalIndices.size() > i ? vertexNormal[normalIndices[i] * 3 + 1] : 0.0f, normalIndices.size() > i ? vertexNormal[normalIndices[i] * 3 + 2] : 0.0f,
                                     normalIndices.size() > i + 1 ? vertexNormal[normalIndices[i + 1] * 3] : 0.0f, normalIndices.size() > i + 1 ? vertexNormal[normalIndices[i + 1] * 3 + 1] : 0.0f, normalIndices.size() > i + 1 ? vertexNormal[normalIndices[i + 1] * 3 + 2] : 0.0f}
                                });
                            }
                        }

                    }

                }

            }
            else if (lineType == "vn") {
                float x, y, z;
                ss >> x >> y >> z;
                vertexNormal.push_back(x);
                vertexNormal.push_back(y);
                vertexNormal.push_back(z);
            }
            else if (lineType == "vt") {
                float u, v;
                ss >> u >> v;
                vertexTexture.push_back(u);
                vertexTexture.push_back(v);
            }
            else {
                ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
        }
        return true;
    }

public:
    MeshHandler() {}

    bool loadObject(std::string fileName) {
        std::string objPath = fileName + ".obj";
        std::string mtlPath = fileName + ".mtl";

        if (!parseMtl(mtlPath)) {
            std::cerr << "Failed to parse MTL file: " << mtlPath << "\n";
            return false;
        }
        if (!parseObj(objPath)) {
            std::cerr << "Failed to parse OBJ file: " << objPath << "\n";
            return false;
        }

        return true;
    }

    const std::vector<float>& getVertices() {
        return vertices;
    }

    const std::vector<float>& getVertexTexture() {
        return vertexTexture;
    }

    const std::vector<float>& getVertexNormal() {
        return vertexNormal;
    }

    const std::unordered_map<std::string, MaterialInfo>& getMaterials() {
        return materials;
    }
        
};