get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/display.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/process.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/pangolin_gl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler_image.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/handler_glbuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ConsoleView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/default_font.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/image_view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ConsoleView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/default_font.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/console/InterpreterInterface.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/console/ConsoleView.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/user_app.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/widgets.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/image_view.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/display.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/view.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/display.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/default_font.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/process.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/display/attach.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/handler/handler_image.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/handler/handler.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/handler/handler_glbuffer.h
    
    CMakeLists.txt
)

target_link_libraries(${COMPONENT} PUBLIC pango_core pango_opengl pango_windowing pango_vars )

target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
