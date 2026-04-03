cmake_minimum_required(VERSION 3.5)
project(OpenGL_Viewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    ./src/main.cpp
    ./src/meshHandler.h
    ./src/object.h
    ./src/camera.h
    ./src/stb_image.h
)

find_package(OpenGL REQUIRED)

include(FetchContent)

# GLM
FetchContent_Declare(
	glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(glm)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY  https://github.com/glfw/glfw.git
    GIT_TAG         3.4
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(glfw)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    glm::glm 
    glfw 
    GLEW 
    OpenGL::GL
)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)