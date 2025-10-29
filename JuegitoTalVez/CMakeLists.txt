cmake_minimum_required(VERSION 3.16)
project(Platformer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Definir Paquetes ---
# (Esto estaba perfecto)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

message(STATUS "--- DEBUGGING VARIABLES ---")
message(STATUS "GLFW Includes: ${glfw3_INCLUDE_DIRS}")
message(STATUS "GLFW Libraries: ${glfw3_LIBRARIES}")
message(STATUS "GLEW Libraries: ${GLEW_LIBRARIES}")
message(STATUS "GLM Libraries: ${glm_LIBRARIES}")
message(STATUS "OpenGL Libraries: ${OPENGL_LIBRARIES}")
message(STATUS "-----------------------------")

# --- Listar Archivos Fuente ---
# (Seguimos usando la lista explícita, es más seguro)
set(SOURCES
    src/main.cpp
    src/core/Game.cpp
    src/core/Window.cpp
    src/core/Renderer.cpp
    src/core/Shader.cpp       # Asumo que tienes este
    src/entities/Player.cpp
    src/entities/Platform.cpp
    src/entities/Rectangle.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GLEW_INCLUDE_DIRS}
    ${glm_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)


target_link_libraries(${PROJECT_NAME} PUBLIC
    ${GLEW_LIBRARIES}
    ${OPENGL_gl_LIBRARY} glfw
    ${glm_LIBRARIES}
    ${OPENGL_LIBRARIES}
)