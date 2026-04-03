#include "stb_image.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

class Texture {
    std::string filePath;
    GLenum textureTarget;
    GLuint textureObj = 0;

    public:
    Texture(GLenum textureTarget, const std::string& fileName) {
        this->textureTarget = textureTarget;
        this->filePath = fileName;
    }

    bool load() {
        stbi_set_flip_vertically_on_load(true);
        int width = 0, height = 0, bitsPerPixel = 0;
        unsigned char* imageData = stbi_load(filePath.c_str(), &width, &height, &bitsPerPixel, 0);
        
        if (imageData == nullptr) {
            std::cerr << "Failed to load texture: " << filePath << "\n";
            return false;
        }
        
        glGenTextures(1, &textureObj);
        glBindTexture(textureTarget, textureObj);
        if (textureTarget == GL_TEXTURE_2D) {
            GLenum format = GL_RGB;
            GLenum internalFormat = GL_RGB;

            if (bitsPerPixel == 1) {
                format = GL_RED;
                internalFormat = GL_RED;
            } else if (bitsPerPixel == 4) {
                format = GL_RGBA;
                internalFormat = GL_RGBA;
            }

            glTexImage2D(textureTarget, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, imageData);
        } else {
            std::cerr << "Unsupported texture target: " << textureTarget << "\n";
            stbi_image_free(imageData);
            return false;
        }

        glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glBindTexture(textureTarget, 0);
        stbi_image_free(imageData);
        return true;

    }

    void bind(GLenum textureUnit) {
        glActiveTexture(textureUnit);
        glBindTexture(textureTarget, textureObj);
    }
};

// Reference: https://ogldev.org/www/tutorial16/tutorial16.html