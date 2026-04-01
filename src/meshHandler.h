#include <vector>
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#pragma once

class MeshHandler {
private:
    std::vector<float> triangles;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

public:
    MeshHandler() {}

    void loadObj(std::string fileName, float r = 1.0f, float g = 1.0f, float b = 1.0f, bool randColor = false) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << fileName << std::endl;
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.substr(0, 2) == "v ") {
                std::istringstream s(line.substr(2));
                float x, y, z;
                s >> x >> y >> z;
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                
                if (randColor) {
                    vertices.push_back(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
                    vertices.push_back(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
                    vertices.push_back(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
                }
                else {
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);
                }
            }
            if (line.substr(0, 2) == "f ") {
                unsigned int v1, v2, v3;
                int spaceIndex1 = line.find(' ', 2);
                if (line.find("/") == std::string::npos) {
                    v1 = std::stoi(line.substr(2, spaceIndex1 - 2));
                    int spaceIndex2 = line.find(' ', spaceIndex1 + 1);
                    v2 = std::stoi(line.substr(spaceIndex1 + 1, spaceIndex2 - spaceIndex1 - 1));
                    int spaceIndex3 = line.find(' ', spaceIndex2 + 1);
                    v3 = std::stoi(line.substr(spaceIndex2 + 1, line.length() - spaceIndex2 - 1));
                } else {
                    v1 = std::stoi(line.substr(2, line.find("/") - 1));
                    int spaceIndex2 = line.find(' ', spaceIndex1 + 1);
                    v2 = std::stoi(line.substr(spaceIndex1 + 1, line.find("/", spaceIndex1 + 1) - spaceIndex1 - 1));
                    int spaceIndex3 = line.find(' ', spaceIndex2 + 1);
                    v3 = std::stoi(line.substr(spaceIndex2 + 1, line.find("/", spaceIndex2 + 1) - spaceIndex2 - 1));
                }


                triangles.push_back(vertices[(v1 - 1) * 6]);
                triangles.push_back(vertices[(v1 - 1) * 6 + 1]);
                triangles.push_back(vertices[(v1 - 1) * 6 + 2]);
                triangles.push_back(vertices[(v1 - 1) * 6 + 3]);
                triangles.push_back(vertices[(v1 - 1) * 6 + 4]);
                triangles.push_back(vertices[(v1 - 1) * 6 + 5]);
                triangles.push_back(vertices[(v2 - 1) * 6]);
                triangles.push_back(vertices[(v2 - 1) * 6 + 1]);
                triangles.push_back(vertices[(v2 - 1) * 6 + 2]);
                triangles.push_back(vertices[(v2 - 1) * 6 + 3]);
                triangles.push_back(vertices[(v2 - 1) * 6 + 4]);
                triangles.push_back(vertices[(v2 - 1) * 6 + 5]);
                triangles.push_back(vertices[(v3 - 1) * 6]);
                triangles.push_back(vertices[(v3 - 1) * 6 + 1]);
                triangles.push_back(vertices[(v3 - 1) * 6 + 2]);
                triangles.push_back(vertices[(v3 - 1) * 6 + 3]);
                triangles.push_back(vertices[(v3 - 1) * 6 + 4]);
                triangles.push_back(vertices[(v3 - 1) * 6 + 5]);

                indices.push_back(v1 - 1);
                indices.push_back(v2 - 1);
                indices.push_back(v3 - 1);
            }
        }
    }

    const std::vector<float>& getTriangles() {
        return triangles;
    }

    const std::vector<float>& getVertices() {
        return vertices;
    }

    const std::vector<unsigned int>& getIndices() {
        return indices;
    }
        
};