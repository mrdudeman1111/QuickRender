CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)

find_program(GLSLC glslc)

file(GLOB sourceGLSL 
  ${CMAKE_CURRENT_SOURCE_DIR}/*.glsl)

foreach(source IN LISTS sourceGLSL)
  get_filename_component(sourceName ${source} NAME_WLE)
  set(output "${CMAKE_BINARY_DIR}/Shaders/${sourceName}.spv")
  message("${GLSLC} ${source} -o ${output}")
  add_custom_command(
    COMMAND ${GLSLC} ${source} -o ${output}
    OUTPUT ${output}
    COMMENT "Compiling ${source}"
    DEPENDS ${source}
    )
  list(APPEND shaderBin ${output})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${shaderBin})

