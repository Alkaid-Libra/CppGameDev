#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "glad/glad.h"
#include "shader.h"

class Rectangle
{
private:
    GLuint VBO;
    GLuint EBO;
    GLuint VAO;
public:
    Rectangle(/* args */)
    {
        // set vertex array object and vertex buffer object and element buffer object
        //---------------------------------------------------------------------------
        // setup VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // setup VBO;
        // set up vertex data(and buffers) and configure vertex attributes 
        // ---------------------------------------------------------------
        float vertices[] = {
        // 位置             // 纹理坐标
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // 左下
        1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  // 右下
        1.0f,  1.0f, 0.0f,  1.0f, 1.0f,  // 右上
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f   // 左上
        };
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        // setup EBO
        unsigned int indices[] = {
            0, 1, 2, // 第一个三角形
            0, 2, 3  // 第二个三角形
        };
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texture coordinate attributes
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // unbind VAO, VBO, EBO
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    ~Rectangle() = default;

    void destroy()
    {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void draw(const Shader& shader) const
    {
        shader.activate();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        shader.deactivate();
    }
};


#endif