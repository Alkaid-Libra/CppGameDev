#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <glad/glad.h>
// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <map>
#include <string>

#include "texture.h"
#include "shader.h"

class ResourceManager
{
public:
    // resource storage
    static std::map<std::string, Shader> shaders;
    static std::map<std::string, Texture2D> textures;
    // loads (and generates) a shader program from file loading vertex, fragment (and geometry) shader's source code. If gShaderFile is not nullptr, it also loads a geometry shader
    static Shader loadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, std::string name)
    {
        shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
        return shaders[name];
    }
    // retrieves(检索) a stored shader
    static Shader getShader(std::string name)
    {
        return shaders[name];
    }
    // loads and generates a texture from file
    static Texture2D loadTexture(const char *file, bool alpha, std::string name)
    {
        textures[name] = loadTextureFromFile(file, alpha);
        return textures[name];
    }
    // retrieves a stored texture
    static Texture2D getTexture(std::string name)
    {
        return textures[name];
    }
    // properly de-allocates all loaded resources
    static void clear()
    {
        // (properly) delete all shader
        for (auto iter : shaders)
            glDeleteProgram(iter.second.ID);
        // (properly) delete all textures
        for (auto iter : textures)
            glDeleteTextures(1, &iter.second.ID);
    }
private:
    // private constructor, that is we do not want any actual resource manager objects. Its members and functions should be publicly available (static).
    ResourceManager(){}
    ~ResourceManager() = default;
    // load and generate a shader from file
    static Shader loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile = nullptr)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        try
        {
            // open file
            std::ifstream vertexShaderFile(vShaderFile);
            std::ifstream fragmentShaderFile(fShaderFile);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vertexShaderFile.rdbuf();
            fShaderStream << fragmentShaderFile.rdbuf();
            // close file handles
            vertexShaderFile.close();
            fragmentShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            // if geometry shader path is present, also load a geometry shader
            if (gShaderFile != nullptr)
            {
                std::ifstream geometryShaderFile(gShaderFile);
                std::stringstream gShaderStream;
                gShaderStream << geometryShaderFile.rdbuf();
                geometryShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch(const std::exception& e)
        {
            // std::cerr << e.what() << '\n';
            std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
        }
        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();
        const char *gShaderCode = geometryCode.c_str();
        // 2. now create shader object from source code
        Shader shader;
        shader.compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
        return shader;
    }
    // load a single texture from file
    static Texture2D loadTextureFromFile(const char *file, bool alpha)
    {
        // create texture object
        Texture2D texture;
        if (alpha)
        {
            texture.internal_format = GL_RGBA;
            texture.image_format = GL_RGBA;
        }
        // load image
        int width, height, nrChannels;
        // stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(file, &width, &height, &nrChannels, 0);
        // now generate texture
        texture.generate(width, height, data);
        // and finally free image data
        stbi_image_free(data);
        return texture;
    }
};


#endif