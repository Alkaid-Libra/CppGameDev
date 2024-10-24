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

// const int numSegments = 100; // circle segment num

bool shouldDrawCircle = false;
double mouseX, mouseY;


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


char board_data[3][3] = 
{
    { '-', '-', '-' },
    { '-', '-', '-' },
    { '-', '-', '-' },
};

char current_piece = 'O';

bool running = true;

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
    if (board_data[0][1] == c && board_data[1][1] == c && board_data[1][1] == c) 
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
void drawPiece() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            switch(board_data[i][j]) {
                case 'O':
                    drawCircle((j - 1) * 2.0 / 3.0, -(i - 1) * 2.0 / 3.0, 1.0 / 3.0, 100);
                    break;
                case 'X':
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

}

void renderScene() {
    // clear the screen
    // glClear(GL_COLOR_BUFFER_BIT);
    if (shouldDrawCircle)
        drawCircle(0., 0., 0.4, 100);

    shouldDrawCircle = false;
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

        int index_x = xpos / 200;
        int index_y = ypos / 200;

        // put piece
        if (board_data[index_y][index_x] == '-') {
            board_data[index_y][index_x] = current_piece;
            // switch piece type
            if (current_piece == 'O') {
                current_piece = 'X';
            } else {
                current_piece = 'O';
            }

            shouldDrawCircle = true;

            // drawCircle(0., 0., 0.4, 100);
            // std::cout << "xxxxx"<<std::endl;
            // drawCircle((0 - 1) * 2.0 / 3.0, (0 - 1) * 2.0 / 3.0, 1.0 / 3.0, 100);
            drawPiece();
            // glfwSwapBuffers(window);
        }
    }
}

void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
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

    // 初始化 OpenGL loader，这里是 glad 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // initialize ImGui
    // IMGUI_CHECKVERSION();
    // ImGui::CreateContext();
    // ImGuiIO& io = ImGui::GetIO(); (void)io;
    // // set the style of ImGui
    // ImGui::StyleColorsDark();
    // // initialize GLFW and OpenGL3 of ImGui
    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    // ImGui_ImplOpenGL3_Init("#version 330"); // 适应 OpenGL 版本

    // set the clear color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // set the key callback
    glfwSetKeyCallback(window, key_callback);
    // set the mouse callback
    glfwSetMouseButtonCallback(window, mouse_callback);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // clear screen
        // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        // glClear(GL_COLOR_BUFFER_BIT);

        // start new ImGui frame
        // ImGui_ImplOpenGL3_NewFrame();
        // ImGui_ImplGlfw_NewFrame();
        // ImGui::NewFrame();

        // set the color for circle
        glColor3f(1.0f, 0.0f, 0.0f);
        // drawCircle(0.0f, 0.0f, 0.5f, 100);
        // drawCircle(0., 0., 0.4, 100);

        // drawCircle(0.0f, 0.0f, 0.5f);
        drawBoard();
        // drawLine(0.0f, 0.0f, 0.3f, 0.3f);
        // renderScene();

        // while (running) {
        //     if (CheckWin('X')) {
        //         // 创建一个简单的 ImGui 窗口
        //         ImGui::Begin("Hello, ImGui!");
        //         ImGui::Text("This is a simple ImGui window.");
        //         ImGui::SliderFloat("Float slider", &io.DeltaTime, 0.0f, 1.0f);
        //         if (ImGui::Button("Close")) {
        //             glfwSetWindowShouldClose(window, true);
        //         }
        //         ImGui::End();


        //     } else if (CheckWin('O')) {
        //         ImGui::Begin("Hello, ImGui!");
        //         ImGui::Text("This is a simple ImGui window.");
        //         ImGui::SliderFloat("Float slider", &io.DeltaTime, 0.0f, 1.0f);
        //         if (ImGui::Button("Close")) {
        //             glfwSetWindowShouldClose(window, true);
        //         }
        //         ImGui::End();
        //     } else if (CheckDraw()) {
        //         ImGui::Begin("Hello, ImGui!");
        //         ImGui::Text("This is a simple ImGui window.");
        //         ImGui::SliderFloat("Float slider", &io.DeltaTime, 0.0f, 1.0f);
        //         if (ImGui::Button("Close")) {
        //             glfwSetWindowShouldClose(window, true);
        //         }
        //         ImGui::End();        
        //     }

            // 渲染 ImGui
            // ImGui::Render();
            // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // drawBoard();
            // drawPiece();
            // drawTipText();

            // int display_w, display_h;
            // glfwGetFramebufferSize(window, &display_w, &display_h);
            // glViewport(0, 0, display_w, display_h);
            // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);

        // }

        // 交换缓冲区
        glfwSwapBuffers(window);
        // deal with events
        glfwPollEvents();
    }

    // 清理
    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}