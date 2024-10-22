# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.5.0)

# 设置OpenGL选择的策略为 GLVND
cmake_policy(SET CMP0072 NEW)

# 设置项目名称
project(CppGameDev VERSION 0.1.1 LANGUAGES C CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

# 查找 OpenGL 库
find_package(OpenGL REQUIRED)
# 查找 GLUT 库
find_package(GLUT REQUIRED)

# 添加可执行文件
add_executable(1_OXGame src/OXGame.cpp)

# 链接 OpenGL 和 GLUT 库
target_link_libraries(1_OXGame ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})