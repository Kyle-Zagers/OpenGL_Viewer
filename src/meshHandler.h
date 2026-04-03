#include <vector>
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "texture.h"

#pragma once

struct MaterialInfo {
    Texture* texture = nullptr;
    float r = -1.0f, g = -1.0f, b = -1.0f;
    std::vector<std::pair<unsigned int, unsigned int>> groups; // offset, count
};

class MeshHandler {
private:
    std::vector<float> vertices;
    std::vector<float> vertexTexture;
    std::vector<float> vertexNormal;

    // position(3), texture(2), normal(3)
    std::vector<float> vertexData;
    std::vector<unsigned int> indexData;

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

                while (std::getline(mtlFile, line) && line.find("newmtl") == std::string::npos) {

                    if (line.find("map_Kd") != std::string::npos) {

                        std::stringstream ss(line);
                        std::string mapKd;
                        std::string textureFileName;
                        ss >> mapKd >> textureFileName;

                        std::string textureFile = "../src/models/" + textureFileName;
                        Texture* tex = new Texture(GL_TEXTURE_2D, textureFile);

                        if (tex->load()) {
                            newMaterial.texture = tex;
                        }
                        else {
                            delete tex;
                        }

                    } else if (line.find("Kd") != std::string::npos) {
                        std::stringstream ss(line);
                        std::string kd;
                        ss >> kd >> newMaterial.r >> newMaterial.g >> newMaterial.b;
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

                // materials[materialName].indexDataOffset = indexData.size();
                materials[materialName].groups.push_back({ indexData.size(), 0 });

                std::streampos currentPos = objFile.tellg();
                
                while (std::getline(objFile, line)) {
                    ss = std::stringstream(line);
                    ss >> lineType;

                    if (lineType != "f" && lineType != "s" && lineType != "g") {
                        objFile.seekg(currentPos);
                        break;
                    }

                    std::vector<unsigned int> vertexIndices;

                    if (lineType == "f") {
                        std::string vertex;

                        unsigned int triangleVertexCount = vertexData.size() / 8; // each vertex has 8 floats (position(3), texture(2), normal(3))

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
                                vertexData.push_back(-1.0f);
                                vertexData.push_back(-1.0f);

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

                                    vertexData.push_back(vertexTexture[textureIndex * 2]);
                                    vertexData.push_back(vertexTexture[textureIndex * 2 + 1]);
                                } else {
                                    vertexData.push_back(-1.0f);
                                    vertexData.push_back(-1.0f);
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

                                    vertexData.push_back(-1.0f);
                                    vertexData.push_back(-1.0f);

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
                                vertexData.push_back(vertexTexture[textureIndex * 2]);
                                vertexData.push_back(vertexTexture[textureIndex * 2 + 1]);
                            }
                            else {
                                vertexData.push_back(-1.0f);
                                vertexData.push_back(-1.0f);
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
                                indexData.push_back(triangleVertexCount);
                                indexData.push_back(triangleVertexCount + i);
                                indexData.push_back(triangleVertexCount + i + 1);
                            }
                            // materials[materialName].indexDataCount += (vertexIndices.size() - 2) * 3;
                            materials[materialName].groups.back().second += (vertexIndices.size() - 2) * 3;
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

    const std::vector<float>& getVertexData() const {
        return vertexData;
    }

    const std::vector<unsigned int>& getIndexData() const {
        return indexData;
    }

    const std::unordered_map<std::string, MaterialInfo>& getMaterials() {
        return materials;
    }
        
};