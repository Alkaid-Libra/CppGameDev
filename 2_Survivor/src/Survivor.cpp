#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>
// #include <chrono>
#include <unistd.h>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

double mouseX, mouseY;
bool game_over = false;

glm::vec3 characterPosition(0.0f, 0.0f, 0.0f);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D texture1;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(texture1, TexCoord);\n"
    "}\n\0";

const char *vertexShaderCharacterSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;

void main() {
    gl_Position = model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";
const char *fragmentShaderCharacterSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    FragColor = texture(texture1, TexCoord);
}
)";


// load and create a texture
GLuint loadTexture(const char* filePath) {
    // load image
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filePath, &width, &height, &channels, 0);

    if (data == nullptr) {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        return 0;
    }

    // create texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

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

    return texture;
}


void loadAnimation(int totalFrames, GLuint* img_player_left, GLuint* img_player_right) {
    for (int i = 0; i < totalFrames; i++) {
        std::string path = std::string(WORKSPACE_DIR) + "/resources/img/player_left_" + std::to_string(i) + ".png";
        img_player_left[i] = loadTexture(path.c_str());
    }
    for (int i = 0; i < totalFrames; i++) {
        std::string path = std::string(WORKSPACE_DIR) + "/resources/img/player_right_" + std::to_string(i) + ".png";
        img_player_right[i] = loadTexture(path.c_str());
    }
}

void renderText(const char* text, ImVec2 position, ImVec4 color) {
    // set the style of ImGui
    ImGui::StyleColorsDark();

    ImGui::SetNextWindowPos(position);
    ImGui::Begin("Text overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::SetWindowFontScale(1.5f);
    ImGui::TextColored(color, "%s", text);
    ImGui::End();
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        characterPosition.y += 0.01f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        characterPosition.y -= 0.01f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        characterPosition.x -= 0.01f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        characterPosition.x += 0.01f;
    }
}

// glfw: whenever the window size changed this callback function executes
// ----------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions;
    // note that width and height will be significantly larget than specified on retina displays
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // mouse press loc
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        std::cout << xpos << ',' << ypos << std::endl;
    }
}

void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

int main() {
    // 设置错误回调
    glfwSetErrorCallback(error_callback);

    // glfw:initialize and configure
    // -----------------------------
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置 OpenGL 版本 (例如 3.3 核心版本)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //核心模式，核心模式下不能使用glBegin()/glEnd()
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE); //兼容核心模式

    // create GLFW window
    // ------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Survivor", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // set the context as the current window
    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1); // enable chuizhi sync
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // IO device callback
    // --------------------
    // set the key callback
    glfwSetKeyCallback(window, key_callback);
    // set the mouse callback
    glfwSetMouseButtonCallback(window, mouse_callback);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    // 初始化 OpenGL loader，这里是 glad 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        throw std::runtime_error("Failed to initialize  Glad");
    }

    // ImGui:initialize ImGui(imgui initial should be after mouse callback)
    // -------------------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // initialize GLFW and OpenGL3 of ImGui
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); // 适应 OpenGL 版本

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // // set the style of ImGui
    // ImGui::StyleColorsDark();

    // set OpenGL state
    // ----------------
    // enable blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile our shader zprogram
    // -------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    // delete shader
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data(and buffers) and configure vertex attributes 
    // ---------------------------------------------------------------
    float vertices[] = {
    // 位置             // 纹理坐标
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // 左下
     1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  // 右下
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,  // 右上
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f   // 左上
    };
    unsigned int indices[] = {
        0, 1, 2, // 第一个三角形
        0, 2, 3  // 第二个三角形
    };

    // set vertex array object and vertex buffer object and element buffer object
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coordinate attributes
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // // set 0 unbind
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);

    static bool showWindow = true;

    // load and create texture
    // -----------------------
    std::string imgPath = std::string(WORKSPACE_DIR) + "/resources/img/background.png";
    GLuint texture = loadTexture(imgPath.c_str());

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    // don't forget to activate/use the shader before setting uniforms!
    // ourShader.use();
    // either set it manually like so:
    // glUniform1i(glGetUniformLocation(ourshader.ID, "texture1"), 0);
    // or set it via the texture class
    // ourShader.setInt("texture2", 1);

    // Set animotion frame
    // ----------------------------
    int idx_current_anim = 0;
    const int PLAYER_ANIM_NUM = 6;
    GLuint img_player_left[PLAYER_ANIM_NUM];
    GLuint img_player_right[PLAYER_ANIM_NUM];

    loadAnimation(PLAYER_ANIM_NUM, img_player_left, img_player_right);

    // build and compile shader
    // vertex shader
    GLuint vertexShaderCharacter = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderCharacter, 1, &vertexShaderCharacterSource, nullptr);
    glCompileShader(vertexShaderCharacter);
    // fragment shader
    GLuint fragmentShaderCharacter = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderCharacter, 1, &fragmentShaderCharacterSource, nullptr);
    glCompileShader(fragmentShaderCharacter);
    // link shaders
    GLuint shaderCharacterProgram = glCreateProgram();
    glAttachShader(shaderCharacterProgram, vertexShaderCharacter);
    glAttachShader(shaderCharacterProgram, fragmentShaderCharacter);
    glLinkProgram(shaderCharacterProgram);
    // delete shader
    glDeleteShader(vertexShaderCharacter);
    glDeleteShader(fragmentShaderCharacter);

    float verticesCharacter[] = {
    // 位置             // 纹理坐标
    -0.0625f, -0.1111f, 0.0f,  0.0f, 0.0f,  // 左下
     0.0625f, -0.1111f, 0.0f,  1.0f, 0.0f,  // 右下
     0.0625f,  0.1111f, 0.0f,  1.0f, 1.0f,  // 右上
    -0.0625f,  0.1111f, 0.0f,  0.0f, 1.0f   // 左上
    };
    unsigned int indicesCharacter[] = {
        0, 1, 2, // 第一个三角形
        0, 2, 3  // 第二个三角形
    };
    unsigned int VAO1, VBO1, EBO1;
    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    glGenBuffers(1, &EBO1);

    glBindVertexArray(VAO1);

    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCharacter), verticesCharacter, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCharacter), indicesCharacter, GL_STATIC_DRAW);

    // position attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coordinate attributes
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);




    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // deal with events
        // poll IO events(keys pressed/released, mouse moved etc.)
        glfwPollEvents();   

        // Sequential frames animal
        static int counter = 0;
        if (++counter % 5 == 0) {
            idx_current_anim++;
        }
        // let anim looping
        idx_current_anim = idx_current_anim % PLAYER_ANIM_NUM;

        // input
        // ------
        processInput(window);

        // render
        // ------
        // clear screen
        glClearColor(0.45f, 0.55f, 0.60f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // use shaderProgram
        glUseProgram(shaderProgram);
        // bind texture on corresponding texture units
        glBindTexture(GL_TEXTURE_2D, texture);

        // set the texture value in the shader
        // ourShader.setFloat("", );

        // render container
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glUseProgram(shaderCharacterProgram);
        glBindTexture(GL_TEXTURE_2D, img_player_left[idx_current_anim]);
        glBindVertexArray(VAO1);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, characterPosition);
        GLuint modelLoc = glGetUniformLocation(shaderCharacterProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // draw game


        if (showWindow) {
        }

        // int display_w, display_h;
        // glfwGetFramebufferSize(window, &display_w, &display_h);
        // glViewport(0, 0, display_w, display_h);

        // 渲染 ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // glfw: swap buffers
        // ------------------
        // 交换缓冲区
        glfwSwapBuffers(window);

    }

    // clean up
    // --------
    // optional:de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}