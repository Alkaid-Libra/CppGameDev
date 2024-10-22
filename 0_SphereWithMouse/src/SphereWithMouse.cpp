#include <GL/glut.h>
#include <iostream>
#include <cmath>

// 窗口大小
const float width = 1280.;
const float height = 1280.;

// 定义圆的半径和圆心坐标
const float radius = 0.1f;
const int num_segments = 100; // 圆周分割的段数，越大圆越平滑
int mouseX = 0, mouseY = 0;


// 处理鼠标移动事件的回调函数
void mouseMove(int x, int y) {
    mouseX = x;
    mouseY = y;

    glutPostRedisplay(); // 强制重新绘制窗口
}
// 处理鼠标点击事件回调函数
void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        std::cout << "mouse clicked at : (" << x << ", " << y << ")" << std::endl;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制三角形
    // glBegin(GL_TRIANGLES);
    //     glColor3f(1.0, 0.0, 0.0);
    //     glVertex2f(-0.5, -0.5);
    //     glColor3f(0.0, 1.0, 0.0);
    //     glVertex2f(0.5, -0.5);
    //     glColor3f(0.0, 0.0, 1.0);
    //     glVertex2f(0.0, 0.5);
    // glEnd();

    float centerX = float(mouseX) * 2.0f / width - 1.0f;
    float centerY = 1.0f - float(mouseY) * 2.0f / height;


    // 绘制三角形扇组成圆
    glBegin(GL_TRIANGLE_FAN);

    glColor3f(1., 1., 1.);
    glVertex2f(centerX , centerY);

    glColor3f(1., 1., 1.);
    for (int i = 0; i <= num_segments; i++)
    {
        float theta = 2. * 3.1415f * float(i) / float(num_segments);//当前角度
        float x = radius * cosf(theta); //计算圆周上的x坐标
        float y = radius * sinf(theta); //计算圆周上的y坐标
        glVertex2f(centerX + x, centerY + y);
    }
    glEnd();


    glFlush();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    // 设置显示模式
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    // 设置窗口大小

    glutInitWindowSize(width, height);
    glutCreateWindow("Hello OpenGL");

    // 设置清屏颜色为黑色
    glClearColor(0., 0., 0., 1.);

    // 设置回调函数，在需要渲染时调用
    glutDisplayFunc(display);

    // 注册鼠标点击事件回调函数
    glutMouseFunc(mouseClick);

    // 注册鼠标移动事件回调函数
    glutPassiveMotionFunc(mouseMove); // mouse move without press
    
    glutMainLoop();

    return 0;
}