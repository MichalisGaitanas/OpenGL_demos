cmake_minimum_required(VERSION 3.10)
project(OpenGL_demos)

# Use C++17 standard, for example
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set warning flags (for GNU/Clang)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wpedantic -Wextra -Werror)
endif()

# Find packages: GLFW, GLEW, etc.
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW3 REQUIRED glfw3)
pkg_check_modules(GLEW REQUIRED glew)

include_directories(${GLFW3_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${GLFW3_LIBRARY_DIRS} ${GLEW_LIBRARY_DIRS})

# If ImGui is a static library already built:
# set(IMGUI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/imgui/libimgui.a)
# Alternatively, if building ImGui from source, add its source files:
file(GLOB IMGUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

# Gather all demos
file(GLOB DEMO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/demos/*.cpp)

foreach(demo_file ${DEMO_SOURCES})
    get_filename_component(demo_name ${demo_file} NAME_WE)
    add_executable(${demo_name} ${demo_file})
    target_link_libraries(${demo_name} PRIVATE OpenGL::GL imgui ${GLFW3_LIBRARIES} ${GLEW_LIBRARIES})
endforeach()