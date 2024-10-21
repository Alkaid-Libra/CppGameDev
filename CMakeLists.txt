# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.5.0)
# 设置项目名称
project(CppGameDev VERSION 0.1.0 LANGUAGES C CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


# 查找 OpenGL 库
find_package(OpenGL REQUIRED)
# 查找 GLUT 库
find_package(GLUT REQUIRED)

# 添加可执行文件
add_executable(CppGameDev src/main.cpp)

# 链接 OpenGL 和 GLUT 库
target_link_libraries(CppGameDev ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})