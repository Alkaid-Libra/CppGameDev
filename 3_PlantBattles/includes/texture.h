#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

class Texture2D
{
public:
    // id of the texture object
    GLuint ID;
    // texture image dimensions
    unsigned int width, height; // width and height of loaded image in pixels
    // texture format 
    unsigned int internal_format; // format of texture object
    unsigned int image_format; // format of loaded image
    // texture configuration
    unsigned int wrap_s; // wrapping mode on S axis
    unsigned int wrap_t; // wrapping mode on T axis
    unsigned int filter_min; // filter mode if texture pixels < screen pixels
    unsigned int filter_max; // filter mode if ... > ...

public:
    Texture2D() : width(0), height(0), internal_format(GL_RGB), image_format(GL_RGB), wrap_s(GL_REPEAT), wrap_t(GL_REPEAT), filter_max(GL_LINEAR), filter_min(GL_LINEAR)
    {
        glGenTextures(1, &this->ID);
    }
    ~Texture2D() = default;

    // generates texture from image data
    void generate(unsigned int width, unsigned int height, unsigned char* data)
    {
        this->width = width;
        this->height = height;
        // create texture
        glBindTexture(GL_TEXTURE_2D, this->ID);
        // target texture|texture mipmap level|texture internal format|texture width and height|border(usually 0)|pixel data format|pixel data type|texture data
        glTexImage2D(GL_TEXTURE_2D, 0, this->internal_format, width, height, 0, this->image_format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        // set texture wrap and filter modes
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrap_s); // horizontal wrapping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrap_t); // vertical wrapping method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->filter_min); // min filter method
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->filter_max); // magnify filter method
        // unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // bind the texture as the current active GL_TEXTURE_2D texture object
    void bind() const
    {
        glBindTexture(GL_TEXTURE_2D, this->ID);
    }
};

#endif