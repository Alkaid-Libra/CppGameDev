#ifndef ANIMATION_H
#define ANIMATION_H


#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include "shader.h"

class Animation
{
private:
    std::vector<unsigned int> frame_list;
    int interval_ms = 0;
    int timer = 0;
    int idx_frame = 0;
public:
    Animation(const char* path, int num, int interval)
    {
        interval_ms = interval;

        char path_file[256];
        for (int i = 0; i < num; i++)
        {
            sprintf(path_file, "%s/%s", WORKSPACE_DIR, path);
            sprintf(path_file, path_file, i);
            std::cout << path_file << std::endl;
            unsigned int frame = loadImage(path_file);
            frame_list.push_back(frame);
        }

    }
    ~Animation()
    {
    }


    void play(Shader& shader, glm::vec3& player_pos, int delta)
    {
        timer += delta;
        if (timer >= interval_ms)
        {
            idx_frame = (idx_frame + 1) % frame_list.size();
            timer = 0;
        }

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, player_pos);
        // set the texture value in the shader
        shader.setMat4("model", model);

    }

    // load and create a texture
    GLuint loadImage(const char* filePath) {
        // load image
        int width, height, channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(filePath, &width, &height, &channels, 0);

        // create texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        if (data == nullptr) {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
            stbi_image_free(data);
        } else {
            // set the texture wrapping parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // horizontal wrapping method
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // vertical wrapping method
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // min filter method
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // magnify filter method

            // generate texture
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB; // format from channels nums
            // target texture|texture mipmap level|texture internal format|texture width and height|border(usually 0)|pixel data format|pixel data type|texture data
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);
        }

        return textureID;
    }

    void loadAnimation(int totalFrames, GLuint* img_player_left, GLuint* img_player_right) {
        for (int i = 0; i < totalFrames; i++) {
            std::string path = std::string(WORKSPACE_DIR) + "/resources/img/player_left_" + std::to_string(i) + ".png";
            img_player_left[i] = loadImage(path.c_str());
        }
        for (int i = 0; i < totalFrames; i++) {
            std::string path = std::string(WORKSPACE_DIR) + "/resources/img/player_right_" + std::to_string(i) + ".png";
            img_player_right[i] = loadImage(path.c_str());
        }
    }
};

#endif
