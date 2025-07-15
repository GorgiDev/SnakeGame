cmake_minimum_required(VERSION 3.10)
project(SnakeGame)

set(CMAKE_CXX_STANDARD 17)

# Include headers
include_directories(include)

# Add glad source
add_library(glad src/glad.c)

# Link GLFW manually
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(SnakeGame src/main.cpp)

set_target_properties(SnakeGame PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# Tell GLFW not to use dynamic runtime
add_definitions(-DGLFW_STATIC)

# Link to static version
target_link_libraries(SnakeGame PRIVATE glad glfw3 opengl32 shlwapi)