get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

file(GLOB BINARY_FILES_TO_EMBED "${CMAKE_CURRENT_LIST_DIR}/src/fonts/*.ttf")
include(EmbedBinaryFiles)
embed_binary_files_rule(fonts.cpp ${BINARY_FILES_TO_EMBED})

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/glchar.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gldraw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/glfont.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gltext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/glsl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/glpangoglu.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gltexturecache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/viewport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/opengl_render_state.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/stb_truetype.h
    ${CMAKE_CURRENT_BINARY_DIR}/fonts.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glfont.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glinclude.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/compat/gl_es_compat.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/compat/gl2engine.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/gldraw.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glsl.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glchar.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/opengl_render_state.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glplatform.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/viewport.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glpixformat.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/gltext.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glvbo.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/gl.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/colour.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/cg.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glpangoglu.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/gltexturecache.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glstate.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/gl.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glcuda.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/gl/glformattraits.h

    ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt

)

find_package (Eigen3 REQUIRED QUIET)
message(STATUS "Found Eigen: '${EIGEN3_INCLUDE_DIRS}'")
target_compile_definitions(${COMPONENT} PUBLIC HAVE_EIGEN HAVE_GLEW)

target_link_libraries(${COMPONENT} PUBLIC pango_core pango_image Eigen3::Eigen)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

if(EMSCRIPTEN)
    target_compile_definitions(${COMPONENT} PUBLIC HAVE_GLES HAVE_GLES_2)
    target_sources( ${COMPONENT} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/compat/gl2engine.cpp)
else()
    if(_LINUX_)
        set(OpenGL_GL_PREFERENCE "GLVND")
    endif()
    find_package(OpenGL REQUIRED QUIET)
    find_package(GLEW REQUIRED QUIET)
    target_include_directories( ${COMPONENT} PUBLIC
        $<BUILD_INTERFACE:${OPENGL_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${GLEW_INCLUDE_DIR}>
    )
    target_link_libraries( ${COMPONENT} PUBLIC
        ${GLEW_LIBRARY} ${OPENGL_LIBRARIES}
    )
endif()
