CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("QuickGame")

find_package(Sail REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${sail_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS} ${assimp_INCLUDE_DIRS} ${glm_INCLUDE_DIRS})

file(GLOB sources
    ${CMAKE_SOURCE_DIR}/*.cpp)

add_subdirectory(${CMAKE_SOURCE_DIR}/Shaders)
add_subdirectory(${CMAKE_SOURCE_DIR}/Meshes)

add_compile_definitions(MODELDIR="${CMAKE_BINARY_DIR}/Meshes/")

add_executable(game ${sources})

target_link_libraries(game vulkan sail::sail ${glfw3_LIBRARIES} ${assimp_LIBRARIES} ${glm_LIBRARIES})

