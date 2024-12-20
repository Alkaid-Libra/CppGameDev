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
#include <chrono>
#include <unistd.h>
#include <string>
// #include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "shader.h"
#include "enemy.h"
#include "audio_player.h"

double mouseX, mouseY;
bool game_over = false;
bool button_pressed = false;
bool inRange = false;

glm::vec3 player_pos(0.0f, 0.0f, 0.0f);
const float PLAYER_SPEED = 0.01;

const int PLAYER_WIDTH = 80;
const int PLAYER_HEIGHT = 80;
const int SHADOW_WIDTH = 32;

bool facing_left = false;

struct timespec current_time;


// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;

// circle whole
void setupCircleVAO(float x, float y, float radius, int segments, std::vector<glm::vec2>& vertices, glm::vec3 fillColor, glm::vec3 borderColor, GLuint& VAO, GLuint& colorVBO, GLuint& VBO)
{
    // 生成顶点和颜色数据
    std::vector<glm::vec3> colors;
        
    // 中心点
    vertices.push_back(glm::vec2(x, y));
    colors.push_back(fillColor); // 内部颜色

    // 圆周上的点（边缘）
    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * 3.14159265359f * i / segments;
        float cx = x + radius * cos(angle);
        float cy = y + radius * sin(angle);

        if (aspect > 1.0f)
            cx /= aspect;
        else
            cy /= aspect;

        vertices.push_back(glm::vec2(cx, cy));
        colors.push_back(borderColor); // 边缘颜色
    }

    glBindVertexArray(VAO);

    // 设置顶点数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // 设置颜色数据
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



void tryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
    const int interval = 100;
    static int counter = 0;
    if ((++counter) % interval == 0)// && enemy_list.size() < 10)
        enemy_list.push_back(new Enemy());
}
// update bullet location
void updateBullets(std::vector<Bullet>& bullet_list, const glm::vec3& player_pos)
{
    const float radial_speed = 0.00045;
    const float tangent_speed = 0.0055;
    float radian_interval = 2 * 3.14159 / bullet_list.size();
    auto now = std::chrono::steady_clock::now();
    auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    // std::cout << current_time << std::endl;
    float radius = 0.25 + 0.1 * sin(current_time * radial_speed);
    for (int i = 0; i < bullet_list.size(); i++)
    {
        float radian = current_time * tangent_speed + radian_interval * i;
        bullet_list[i].position.x = player_pos.x + radius * sin(radian) / aspect;
        bullet_list[i].position.y = player_pos.y + radius * cos(radian);
        // std::cout << bullet_list[i].position.x << std::endl;
    }
}

// load and create a texture
GLuint loadTexture(const char* filePath) {
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

void loadAnimation(int totalFrames, GLuint* img_player_left, GLuint* img_player_right, std::string type) {
    for (int i = 0; i < totalFrames; i++) {
        std::string path = std::string(WORKSPACE_DIR) + "/resources/img/" + type + "_left_" + std::to_string(i) + ".png";
        img_player_left[i] = loadTexture(path.c_str());
    }
    for (int i = 0; i < totalFrames; i++) {
        std::string path = std::string(WORKSPACE_DIR) + "/resources/img/" + type + "_right_" + std::to_string(i) + ".png";
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
        player_pos.y += PLAYER_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        player_pos.y -= PLAYER_SPEED;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        player_pos.x -= PLAYER_SPEED;
        facing_left = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        player_pos.x += PLAYER_SPEED;
        facing_left = false;
    }
}

// glfw: whenever the window size changed this callback function executes
// ----------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions;
    // note that width and height will be significantly larger than specified on retina displays
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

        // button_pressed = true;
    }
    if (inRange && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        button_pressed = true;
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
    glfwSwapInterval(1); // enable chuizhi sync

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    // 初始化 OpenGL loader，这里是 glad 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        throw std::runtime_error("Failed to initialize  Glad");
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // IO device callback
    // --------------------
    // set the key callback
    glfwSetKeyCallback(window, key_callback);
    // set the mouse callback
    glfwSetMouseButtonCallback(window, mouse_callback);

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
    std::string backgroundVertShaderPath = std::string(WORKSPACE_DIR) + "/shaders/background.vert";
    std::string backgroundFragShaderPath = std::string(WORKSPACE_DIR) + "/shaders/background.frag";
    Shader backgroundShader(backgroundVertShaderPath.c_str(), backgroundFragShaderPath.c_str());

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
    static bool freezeScreen = false;

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

    // Set animation frame
    // ----------------------------
    int idx_current_anim = 0;
    const int PLAYER_ANIM_NUM = 6;
    GLuint img_player_left[PLAYER_ANIM_NUM];
    GLuint img_player_right[PLAYER_ANIM_NUM];

    loadAnimation(PLAYER_ANIM_NUM, img_player_left, img_player_right, "player");

    // build and compile shader
    // vertex shader
    std::string characterVertShaderPath = std::string(WORKSPACE_DIR) + "/shaders/character.vert";
    std::string characterFragShaderPath = std::string(WORKSPACE_DIR) + "/shaders/character.frag";
    Shader characterShader(characterVertShaderPath.c_str(), characterFragShaderPath.c_str());

    float verticesCharacter[] = {
    // 位置                     // 纹理坐标
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

    // // load shadow
    // std::string imgPath_playerShadow = std::string(WORKSPACE_DIR) + "/resources/img/shadow_player.png";
    // GLuint texture_playerShadow = loadTexture(imgPath_playerShadow.c_str());

    // characterShader.use();
    // characterShader.setInt("texture1", 0);
    // characterShader.setInt("texture2", 1);
    // backgroundShader.use();
    // backgroundShader.setInt("texture1", 2);

    // Enemy
    std::vector<Enemy*> enemy_list;
    const int ENEMY_ANIM_NUM = 6;
    GLuint img_enemy_left[ENEMY_ANIM_NUM];
    GLuint img_enemy_right[ENEMY_ANIM_NUM];
    loadAnimation(ENEMY_ANIM_NUM, img_enemy_left, img_enemy_right, "enemy");

    // Bullet
    std::vector<Bullet> bullet_list(3);
    // 使用着色器并绘制圆形
    std::string basicTriangleVertShaderPath = std::string(WORKSPACE_DIR) + "/shaders/basicTriangle.vert";
    std::string basicTriangleFragShaderPath = std::string(WORKSPACE_DIR) + "/shaders/basicTriangle.frag";
    Shader basicTriangleShader(basicTriangleVertShaderPath.c_str(), basicTriangleFragShaderPath.c_str());
    // basicTriangleShader.use();
    // 设置VAO、VBO、颜色VBO等
    GLuint circleVAO, colorVBO, circleVBO;
    std::vector<glm::vec2> verticesCircle;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glGenBuffers(1, &colorVBO);
    setupCircleVAO(0.0, 0.0, 0.05, 100, verticesCircle ,glm::vec3(1.0, 0.0, 0.0), glm::vec3(1.0, 1.0, 0.0), circleVAO, colorVBO, circleVBO);
    // float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    glm::mat4 projection = glm::mat4(1.0f);
    // if (aspect > 1.0)
    //     projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
    // else
    //     projection = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -1.0f, 1.0f);
    basicTriangleShader.activate();
    basicTriangleShader.setMat4("projection", projection);

    // game score
    int score = 0;

    // load music
    AudioPlayer audioPlayer;
    const std::string bgm_audioFile = std::string(WORKSPACE_DIR) + "/resources/mus/bgm.mp3";
    const std::string hit_audioFile = std::string(WORKSPACE_DIR) + "/resources/mus/hit.wav";
    audioPlayer.loadSound(bgm_audioFile);
    audioPlayer.loadSound(hit_audioFile);
    bool music_played = false;
    // audioPlayer.play(0, true);

    // main menu
    bool is_game_started = false;
    std::string menuBackgroundVertShaderPath = std::string(WORKSPACE_DIR) + "/shaders/background.vert";
    std::string menuBackgroundFragShaderPath = std::string(WORKSPACE_DIR) + "/shaders/background.frag";
    Shader menuBackgroundShader(menuBackgroundVertShaderPath.c_str(), menuBackgroundFragShaderPath.c_str());
    std::string menu_imgPath = std::string(WORKSPACE_DIR) + "/resources/img/menu.png";
    GLuint menu_texture = loadTexture(menu_imgPath.c_str());
    std::string buttonVertShaderPath = std::string(WORKSPACE_DIR) + "/shaders/button.vert";
    std::string buttonFragShaderPath = std::string(WORKSPACE_DIR) + "/shaders/button.frag";
    Shader buttonShader(buttonVertShaderPath.c_str(), buttonFragShaderPath.c_str());
    glm::vec3 scale(0.5f, 0.5f, 1.0f);
    glm::vec3 offset(0.0f, 0.0f, 0.0f);
    std::string startButtonIdlePath = std::string(WORKSPACE_DIR) + "/resources/img/ui_start_idle.png";
    GLuint start_button_idle_texture = loadTexture(startButtonIdlePath.c_str());
    std::string startButtonHoveredPath = std::string(WORKSPACE_DIR) + "/resources/img/ui_start_hovered.png";
    GLuint start_button_hovered_texture = loadTexture(startButtonHoveredPath.c_str());
    std::string startButtonPushedPath = std::string(WORKSPACE_DIR) + "/resources/img/ui_start_pushed.png";
    GLuint start_button_pushed_texture = loadTexture(startButtonPushedPath.c_str());
    std::string quitButtonIdlePath = std::string(WORKSPACE_DIR) + "/resources/img/ui_quit_idle.png";
    GLuint quit_button_idle_texture = loadTexture(quitButtonIdlePath.c_str());
    std::string quitButtonHoveredPath = std::string(WORKSPACE_DIR) + "/resources/img/ui_quit_hovered.png";
    GLuint quit_button_hovered_texture = loadTexture(quitButtonHoveredPath.c_str());
    std::string quitButtonPushedPath = std::string(WORKSPACE_DIR) + "/resources/img/ui_quit_pushed.png";
    GLuint quit_button_pushed_texture = loadTexture(quitButtonPushedPath.c_str());

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // deal with events
        // poll IO events(keys pressed/released, mouse moved etc.)
        glfwPollEvents();  

        // start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

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

        if (!freezeScreen || !showWindow)
        {
            // render
            // ------
            // clear screen
            glClearColor(0.45f, 0.55f, 0.60f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            if (is_game_started)
            {
                ImGui::Begin("Scoreboard", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
                ImGui::SetWindowPos(ImVec2(10, 10), ImGuiCond_Always);  // 设置窗口位置在左上角
                ImGui::Text("Score: %d", score);  // 显示得分
                ImGui::End();

                if (!music_played)
                {
                    audioPlayer.play(0, true);
                    music_played = true;
                }

                // use shaderProgram
                backgroundShader.activate();
                // bind texture on corresponding texture units
                // glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, texture);

                // render container
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                characterShader.activate();
                glActiveTexture(GL_TEXTURE0);
                // glBindTexture(GL_TEXTURE_2D, texture_playerShadow);
                // glActiveTexture(GL_TEXTURE1);
                if (facing_left)
                    glBindTexture(GL_TEXTURE_2D, img_player_left[idx_current_anim]);
                else
                    glBindTexture(GL_TEXTURE_2D, img_player_right[idx_current_anim]);

                if (player_pos.x < -1.0f) player_pos.x = -1.0f;
                if (player_pos.x > 1.0f) player_pos.x = 1.0f;
                if (player_pos.y < -1.0f) player_pos.y = -1.0f;
                if (player_pos.y > 1.0f) player_pos.y = 1.0f;

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, player_pos);
                // set the texture value in the shader
                characterShader.setMat4("model", model);

                glBindVertexArray(VAO1);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                // draw game

                // enemy
                tryGenerateEnemy(enemy_list);
                for (Enemy* enemy : enemy_list)
                {
                    if (enemy->facing_left)
                        glBindTexture(GL_TEXTURE_2D, img_enemy_left[idx_current_anim]);
                    else 
                        glBindTexture(GL_TEXTURE_2D, img_enemy_right[idx_current_anim]);

                    enemy->move(player_pos);
                    glm::mat4 model_enemy = glm::mat4(1.0f);
                    model_enemy = glm::translate(model_enemy, enemy->position);
                    characterShader.setMat4("model", model_enemy);
                    glBindVertexArray(VAO1);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }

                // bullet
                updateBullets(bullet_list, player_pos);
                basicTriangleShader.activate();
                for (const Bullet& bullet : bullet_list)
                {
                    glm::mat4 model_bullet = glm::mat4(1.0f);
                    model_bullet = glm::translate(model_bullet, bullet.position);
                    characterShader.setMat4("model", model_bullet);
                    glBindVertexArray(circleVAO);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, verticesCircle.size());
                    glBindVertexArray(0);      
                }
                // check bullet with enemy collision
                for (Enemy* enemy : enemy_list)
                {
                    for (const Bullet& bullet : bullet_list)
                    {
                        if (enemy->checkBulletCollision(bullet))
                        {
                            enemy->hurted();
                            score++;
                            audioPlayer.play(1, false);
                        }
                    }
                }
                // remove enemey hurted
                for (int i = 0; i < enemy_list.size(); i++)
                {
                    Enemy* enemy = enemy_list[i];
                    if (!enemy->checkAlive())
                    {
                        std::swap(enemy_list[i], enemy_list.back());
                        enemy_list.pop_back();
                        delete enemy;
                    }
                }
            }
            else
            {
                // menu background
                menuBackgroundShader.activate();
                glBindTexture(GL_TEXTURE_2D, menu_texture);
                
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
                // button
                scale = glm::vec3(0.12f, 0.13f, 1.0f);
                offset = glm::vec3(-3.0f, -2.2f, 0.0f);
                glm::mat4 button_model = glm::mat4(1.0f);
                button_model = glm::scale(button_model, scale);
                button_model = glm::translate(button_model, offset);
                buttonShader.activate();
                glBindTexture(GL_TEXTURE_2D, start_button_idle_texture);
                buttonShader.setMat4("model", button_model);
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

                scale = glm::vec3(0.12f, 0.13f, 1.0f);
                offset = glm::vec3(3.0f, -2.2f, 0.0f);
                button_model = glm::mat4(1.0f);
                button_model = glm::scale(button_model, scale);
                button_model = glm::translate(button_model, offset);
                buttonShader.activate();
                glBindTexture(GL_TEXTURE_2D, quit_button_idle_texture);
                buttonShader.setMat4("model", button_model);
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);     

                double currentX, currentY;
                glfwGetCursorPos(window, &currentX, &currentY);
                // std::cout << currentX << " " << currentY << std::endl;
                if (currentX >= 388 && currentX <= 484 && currentY >= 422 && currentY <= 499)
                {
                    scale = glm::vec3(0.12f, 0.13f, 1.0f);
                    offset = glm::vec3(-3.0f, -2.2f, 0.0f);
                    button_model = glm::mat4(1.0f);
                    button_model = glm::scale(button_model, scale);
                    button_model = glm::translate(button_model, offset);
                    buttonShader.activate();
                    glBindTexture(GL_TEXTURE_2D, start_button_hovered_texture);
                    buttonShader.setMat4("model", button_model);
                    glBindVertexArray(VAO);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                    if (button_pressed)
                    {
                        is_game_started = true;
                    }
                    inRange = true;
                }
                else if (currentX >= 799 && currentX <= 943 && currentY >= 421 && currentY <= 495)
                {
                    scale = glm::vec3(0.12f, 0.13f, 1.0f);
                    offset = glm::vec3(3.0f, -2.2f, 0.0f);
                    button_model = glm::mat4(1.0f);
                    button_model = glm::scale(button_model, scale);
                    button_model = glm::translate(button_model, offset);
                    buttonShader.activate();
                    glBindTexture(GL_TEXTURE_2D, quit_button_hovered_texture);
                    buttonShader.setMat4("model", button_model);
                    glBindVertexArray(VAO);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                    if (button_pressed)
                    {
                        glfwSetWindowShouldClose(window, true);
                    }
                    inRange = true;
                }
                else             
                    inRange = false;
            }
        }         


        // check if collision
        for (Enemy* enemy : enemy_list)
        {
            if (enemy->checkPlayerCollision(player_pos) && showWindow)
            {
                ImGui::Begin("Game Over!", &showWindow);

                ImGui::SetCursorPos(ImVec2(150, 50));
                ImGui::Text("Score: %d", score);

                ImGui::SetCursorPos(ImVec2(150, 75));
                if (ImGui::Button("Close")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::End();

                freezeScreen = true;
            }
        }

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

    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    glDeleteBuffers(1, &EBO1);

    glDeleteBuffers(1, &circleVBO);
    glDeleteBuffers(1, &colorVBO);
    glDeleteVertexArrays(1, &circleVAO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}