get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/renderable.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/scene/tree.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/scene/renderable.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/scene/axis.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/scene/interactive.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/scene/scenehandler.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/scene/interactive_index.h

    CMakeLists.txt

)

target_link_libraries(${COMPONENT} PUBLIC pango_opengl)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
