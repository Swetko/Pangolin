get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/vars.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/varstate.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/var/var.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/var/varextra.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/var/varvalue.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/var/varvaluet.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/var/varinit.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/var/varvaluegeneric.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/var/varwrapper.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/var/varstate.h

    CMakeLists.txt

)

target_link_libraries(${COMPONENT} PUBLIC pango_core)
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

if(BUILD_TESTS)
    add_executable(test_vars ${CMAKE_CURRENT_LIST_DIR}/tests/test_all.cpp)
    target_link_libraries(test_vars PRIVATE Catch2::Catch2 ${COMPONENT})
    catch_discover_tests(test_vars)
endif()
