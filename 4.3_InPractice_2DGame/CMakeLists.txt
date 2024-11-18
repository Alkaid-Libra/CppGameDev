# 指定CMake的最低版本要求
cmake_minimum_required(VERSION 3.5.0)

# 设置OpenGL选择的策略为 GLVND
cmake_policy(SET CMP0072 NEW)

# 设置项目名称
project(CppGameDev VERSION 0.1.1 LANGUAGES C CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 查找 OpenGL 库
find_package(OpenGL REQUIRED)
# 查找 GLUT 库
# find_package(GLUT REQUIRED)
# 查找 glfw 库
find_package(X11 REQUIRED)
find_package(glfw3 REQUIRED)
# find_library(GLFW_LIB glfw HINTS ${CMAKE_SOURCE_DIR}/../external/glfw/lib)
# include_directories(${CMAKE_SOURCE_DIR}/../external/glfw/include)
# 查找GLM库
find_package(glm REQUIRED)
# find OpenAL
find_package(OpenAL REQUIRED)
if(NOT OpenAL_FOUND)
    message(FATAL_ERROR "OpenAL not found. Please install OpenAL.")
endif()
# find libsndfile（手动查找库和头文件）
find_path(SNDFILE_INCLUDE_DIR sndfile.h)
find_library(SNDFILE_LIBRARY sndfile)
# find libmpg123
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(MPG123 REQUIRED mpg123)
# include_directories(${MPG123_INCLUDE_DIRS})
# link_libraries(${MPG123_LIBRARIES})
# 手动设置路径(因为mpg123在非标准路径)
include_directories(/usr/include)
link_directories(/usr/lib/x86_64-linux-gnu)


# set env variable
set(WORKSPACE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# define WORKSPACE_DIR micro to cpp
add_definitions(-DWORKSPACE_DIR="${WORKSPACE_DIR}")
# define the root directory path
set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}")
# configure the root_directory.h.in template to generate root_directory.h
configure_file(
    ${CMAKE_SOURCE_DIR}/../includes/root_directory.h.in
    # ${CMAKE_BINARY_DIR}/root_directory.h
    ${CMAKE_SOURCE_DIR}/includes/root_directory.h
    @ONLY
)

# 设置irrKlang头文件和库路径
set(IRRKLANG_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../external/irrKlang/include")
set(IRRKLANG_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/../external/irrKlang/lib")

# 设置freetype库的路径
set(FREETYPE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../external/freetype/include/freetype2)
set(FREETYPE_LIBRARIES ${CMAKE_CURRENT_SOURCE_DIR}/../external/freetype/lib)
# 查找freetype库
# find_package(Freetype REQUIRED)

# 源文件
set(GLAD_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/include)
set(GLAD_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/src/glad.c)
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

set(SOURCES
    src/2dgame.cpp
    ${IMGUI_SOURCES}
    ${GLAD_SOURCES}
)

# add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../external/imageloader
    ${CMAKE_CURRENT_SOURCE_DIR}/../includes
    ${CMAKE_CURRENT_SOURCE_DIR}/includes
    ${SNDFILE_INCLUDE_DIR}
)
# 包含irrKlang头文件
include_directories(${IRRKLANG_INCLUDE_DIR})
# 添加irrKlang库
link_directories(${IRRKLANG_LIBRARY_DIR})

# 添加可执行文件
add_executable(main ${SOURCES})

# 链接 OpenGL 和 glfw 库
target_link_libraries(main PRIVATE
    glfw3
    X11  # 添加 X11 库
    ${OPENGL_LIBRARIES}
    openal
    ${SNDFILE_LIBRARY}
    # ${MPG123_LIBRARIES}
    mpg123
    IrrKlang
    # ${FREETYPE_LIBRARIES}
    freetype
)

# 确保运行时找到 ikpMP3.so
set_target_properties(main PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 将 ikpMP3.so 复制到运行时目录
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${IRRKLANG_LIBRARY_DIR}/ikpMP3.so
            $<TARGET_FILE_DIR:main>
)

# 如果有使用GLM，添加
# target_link_libraries(1_OXGame PRIVATE glm::glm)

# 设定包含目录
target_include_directories(main PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLFW_INCLUDE_DIRS}
    ${GLM_INCLUDE_DIRS}
    ${GLAD_INCLUDE}
    ${FREETYPE_INCLUDE_DIRS}
)