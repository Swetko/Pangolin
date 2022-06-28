get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/packetstream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/packetstream_reader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/packetstream_writer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/playback_session.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/log/packetstream_writer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/log/packetstream_source.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/log/sync_time.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/log/playback_session.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/log/packetstream.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/log/packet.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/log/packetstream_tags.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/log/packetstream_reader.h

    CMakeLists.txt

)

target_compile_definitions(${COMPONENT} PRIVATE "PANGOLIN_VERSION_STRING=\"${PANGOLIN_VERSION}\"")
target_link_libraries(${COMPONENT} PUBLIC pango_core)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
