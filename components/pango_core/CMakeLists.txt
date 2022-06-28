get_filename_component(COMPONENT ${CMAKE_CURRENT_LIST_DIR} NAME)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(${COMPONENT} PUBLIC "_OSX_")
elseif(WIN32 OR WIN64)
    target_compile_definitions(${COMPONENT} PUBLIC "_WIN_")
elseif(EMSCRIPTEN)
    target_compile_definitions(${COMPONENT} PUBLIC "_EMSCRIPTEN_")
else()
    target_compile_definitions(${COMPONENT} PUBLIC "_LINUX_")
endif()

target_sources( ${COMPONENT}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/file_extension.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/file_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/sigstate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/threadedfilebuf.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/avx_math.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/uri.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/param_set.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/factory/factory_registry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/factory/factory_help.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/include/sigslot/signal.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/dynalo/dynalo.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/dynalo/symbol_helper.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/dynalo/detail/macos/dynalo.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/dynalo/detail/config.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/dynalo/detail/linux/dynalo.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/dynalo/detail/windows/dynalo.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/compat/glutbitmap.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/compat/type_traits.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/platform.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/pangolin.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/factory/factory.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/factory/factory_registry.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/factory/factory_help.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/range.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/posix/shared_memory_buffer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/posix/semaphore.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/posix/condition_variable.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/sigstate.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/param_set.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/memstreambuf.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/format_string.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/params.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/parse.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/simple_math.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/transform.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/threadedfilebuf.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/file_extension.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/assert.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/log.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/argagg.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/true_false_toggle.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/variadic_all.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/file_utils.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/uri.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/fix_size_buffer_queue.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/xml/rapidxml_utils.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/xml/rapidxml_iterators.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/xml/rapidxml_print.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/xml/rapidxml.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/timer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/avx_math.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/picojson.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/is_streamable.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/type_convert.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/compontent_cast.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/signal_slot.h
    ${CMAKE_CURRENT_LIST_DIR}/include/pangolin/utils/bitmask.h
    ${CMAKE_CURRENT_LIST_DIR}/include/NaturalSort/natural_sort.hpp
    
    CMakeLists.txt
)

if (UNIX)
    target_sources( ${COMPONENT} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/posix/condition_variable.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/posix/semaphore.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/posix/shared_memory_buffer.cpp
    )
    if (NOT APPLE)
        target_link_libraries(${COMPONENT} PUBLIC rt)
    endif()
endif()


target_compile_features(${COMPONENT} PUBLIC cxx_decltype_auto )
target_include_directories(${COMPONENT} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

find_package(Threads QUIET)
if(Threads_FOUND)
    target_link_libraries(${COMPONENT} PUBLIC Threads::Threads)
endif()

## Generate symbol export helper header on MSVC
IF(MSVC)
    include(GenerateExportHeader)
    GENERATE_EXPORT_HEADER( ${COMPONENT}
        BASE_NAME PANGOLIN
        EXPORT_MACRO_NAME PANGOLIN_EXPORT
        EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/pangolin/pangolin_export.h"
        STATIC_DEFINE PANGOLIN_BUILT_AS_STATIC
    )
    target_include_directories(${COMPONENT} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include"
      DESTINATION ${CMAKE_INSTALL_PREFIX}
    )
ENDIF()

if(BUILD_TESTS)
    add_executable(test_uris ${CMAKE_CURRENT_LIST_DIR}/tests/tests_uri.cpp)
    target_link_libraries(test_uris PRIVATE Catch2::Catch2 ${COMPONENT})
    catch_discover_tests(test_uris)
endif()
