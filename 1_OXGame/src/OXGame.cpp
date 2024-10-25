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

double mouseX, mouseY;
bool game_over = false;

// circle line
void drawCircle(float centerX, float centerY, float radius, int num_segments) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= num_segments; i++) {
        float angle = 2.0f * M_PI * float(i) / float(num_segments);
        float x = centerX + radius * cosf(angle);
        float y = centerY + radius * sinf(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

// circle whole
// void drawCircle(float centerX, float centerY, float radius) {
//     glBegin(GL_TRIANGLE_FAN);
//     glColor3f(1.0f, 0.0f, 0.0f);
//     glVertex2f(centerX, centerY);

//     for (int i = 0; i <= numSegments; i++) {
//         float angle = 2.0f * M_PI * i / numSegments;
//         float x = centerX + radius * cosf(angle);
//         float y = centerY + radius * sinf(angle);
//         glVertex2f(x, y);
//     }

//     glEnd();
// }

// draw line
void drawLine(float startPosX, float startPosY, float endPosX, float endPosY) {
    glBegin(GL_LINES);
    glVertex2f(startPosX, startPosY);
    glVertex2f(endPosX, endPosY);
    glEnd();
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

char board_data[3][3] = 
{
    { '-', '-', '-' },
    { '-', '-', '-' },
    { '-', '-', '-' },
};

char current_piece = 'O';

// Check the player if win
bool checkWin(char c) {
    if (board_data[0][0] == c && board_data[0][1] == c && board_data[0][2] == c) 
        return true;
    if (board_data[1][0] == c && board_data[1][1] == c && board_data[1][2] == c) 
        return true;
    if (board_data[2][0] == c && board_data[2][1] == c && board_data[2][2] == c) 
        return true;
    if (board_data[0][0] == c && board_data[1][0] == c && board_data[2][0] == c) 
        return true;
    if (board_data[0][1] == c && board_data[1][1] == c && board_data[2][1] == c) 
        return true;
    if (board_data[0][2] == c && board_data[1][2] == c && board_data[2][2] == c) 
        return true;
    if (board_data[0][0] == c && board_data[1][1] == c && board_data[2][2] == c) 
        return true;
    if (board_data[0][2] == c && board_data[1][1] == c && board_data[2][0] == c) 
        return true;

    return false;
}

// Check draw or not at the moment
bool checkDraw() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board_data[i][j] == '-') {
                return false;
            }
        }
    }

    return true;
}

// draw chess board
void drawBoard() {
    glBegin(GL_LINES);
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(-1.0, -1.0/3.0); glVertex2f(1.0, -1.0/3.0);
    glVertex2f(-1.0, 1.0/3.0); glVertex2f(1.0, 1.0/3.0);
    glVertex2f(-1.0/3.0, -1.0); glVertex2f(-1.0/3.0, 1.0);
    glVertex2f(1.0/3.0, -1.0); glVertex2f(1.0/3.0, 1.0);
    glEnd();
}

// draw piece
void drawPiece(GLFWwindow* window) {
    glfwGetCursorPos(window, &mouseX, &mouseY);

    int index_x = mouseX / 200;
    int index_y = mouseY / 200;

    // put piece
    if (board_data[index_y][index_x] == '-' && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !game_over) {
        board_data[index_y][index_x] = current_piece;
        // switch piece type
        if (current_piece == 'O') {
            current_piece = 'X';
        } else {
            current_piece = 'O';
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            switch(board_data[i][j]) {
                case 'O':
                    // set the color for circle
                    glColor3f(1.0f, 0.0f, 0.0f);
                    drawCircle((j - 1) * 2.0 / 3.0, -(i - 1) * 2.0 / 3.0, 1.0 / 3.0, 100);
                    break;
                case 'X':
                    // set the color for circle
                    glColor3f(1.0f, 1.0f, 1.0f);
                    drawLine(j * 2.0 / 3.0 - 1, -(i * 2.0 / 3.0 - 1.0 / 3.0), j * 2.0 / 3.0 - 1.0 / 3.0, -(i * 2.0 / 3.0 - 1));
                    drawLine(j * 2.0 / 3.0 - 1, -(i * 2.0 / 3.0 - 1.0), j * 2.0 / 3.0 - 1.0 / 3.0, -(i * 2.0 / 3.0 - 1.0 / 3.0));
                    break;
                case '-':
                    break;
                default:
                    break;
            }
        }
    }
}

// draw tip
void drawTipText() {
    static char str[64];
    sprintf(str, "Current Piece Type: %c", current_piece);

    // render text
    renderText(str, ImVec2(10, 10), ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
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
        std::cout << xpos << '.' << ypos << std::endl;
    }
}

void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

void initialize(GLFWwindow* window) {
    // 初始化 OpenGL loader，这里是 glad 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        throw std::runtime_error("Failed to initialize  Glad");
    }

    // initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // initialize GLFW and OpenGL3 of ImGui
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); // 适应 OpenGL 版本
}

int main() {
    // 设置错误回调
    glfwSetErrorCallback(error_callback);

    // initialize GLFW
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
    GLFWwindow* window = glfwCreateWindow(600, 600, "OXGame", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // set the context as the current window
    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1); // enable chuizhi sync

    // set the key callback
    glfwSetKeyCallback(window, key_callback);
    // set the mouse callback
    glfwSetMouseButtonCallback(window, mouse_callback);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // initialize OpenGL and Imgui(imgui initial should be after mouse callback)
    initialize(window);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // // set the style of ImGui
    // ImGui::StyleColorsDark();

    // set the clear color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    static bool showWindow = true;

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // deal with events
        glfwPollEvents();   

        // clear screen
        // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // draw game
        drawBoard();
        drawTipText();
        drawPiece(window);

        if (showWindow) {
            if (checkWin('X')) {
                // ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
                // ImGui::SetNextWindowPos(ImVec2(200, 150), ImGuiCond_FirstUseEver);

                ImGui::Begin("Game Over!", &showWindow);

                ImGui::SetCursorPos(ImVec2(150, 50));
                ImGui::Text("X Win!");

                ImGui::SetCursorPos(ImVec2(150, 75));
                if (ImGui::Button("Close")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::End();

                game_over = true;
            } else if (checkWin('O')) {

                ImGui::Begin("Game Over!", &showWindow);

                ImGui::SetCursorPos(ImVec2(150, 50));
                ImGui::Text("O Win!");

                ImGui::SetCursorPos(ImVec2(150, 75));
                if (ImGui::Button("Close")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::End();

                game_over = true;
            } else if (checkDraw()) {
                ImGui::Begin("Game Over!", &showWindow);

                ImGui::SetCursorPos(ImVec2(150, 50));
                ImGui::Text("Draw!");

                ImGui::SetCursorPos(ImVec2(150, 75));
                if (ImGui::Button("Close")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::End();

                game_over = true;
            }
        }

        // int display_w, display_h;
        // glfwGetFramebufferSize(window, &display_w, &display_h);
        // glViewport(0, 0, display_w, display_h);

        // 渲染 ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 交换缓冲区
        glfwSwapBuffers(window);
    }

    // 清理
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}