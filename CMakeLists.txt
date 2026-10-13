cmake_minimum_required(VERSION 3.31.10)

project("voxide" LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
endif()

message("Building in ${CMAKE_BUILD_TYPE} mode")

set(EXTERNAL_SOURCES
    external/glad/gl.c
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/backends/imgui_impl_sdl3.cpp
)

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/window.cpp
    src/engine.cpp
    src/shader_program.cpp
    src/camera.cpp
    src/chunk.cpp
    src/chunk_mesh.cpp
    src/world.cpp
    src/stb_image.cpp
    src/world_renderer.cpp
    src/entity.cpp
    src/player.cpp

    ${EXTERNAL_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ./include)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE 
    external
    external/glad
    external/imgui
    external/imgui/backends
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD 23
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF

    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

set_source_files_properties(${EXTERNAL_SOURCES} PROPERTIES COMPILE_OPTIONS "-w")

target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Werror

        # no optimization + sanitizers for debug build
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
)

target_link_options(${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:${PROJECT_NAME_UPPER}_DEBUG>
)

find_package(SDL3 REQUIRED CONFIG)
find_package(glm CONFIG REQUIRED)
find_package(spdlog REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 glm::glm spdlog::spdlog)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
endif()

