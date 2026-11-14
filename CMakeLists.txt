cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0091 NEW)

project(DuiLibElement VERSION 1.0.0 LANGUAGES CXX C RC)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(DUILIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/duilib/DuiLib" CACHE PATH "duilib source directory")

if(NOT EXISTS "${DUILIB_ROOT}/UIlib.h")
    message(FATAL_ERROR "duilib sources not found at ${DUILIB_ROOT}")
endif()

set(DUILIB_SOURCES "")
aux_source_directory("${DUILIB_ROOT}" DUILIB_ROOT_SOURCES)
aux_source_directory("${DUILIB_ROOT}/Core" DUILIB_CORE_SOURCES)
aux_source_directory("${DUILIB_ROOT}/Control" DUILIB_CONTROL_SOURCES)
aux_source_directory("${DUILIB_ROOT}/Layout" DUILIB_LAYOUT_SOURCES)
aux_source_directory("${DUILIB_ROOT}/Utils" DUILIB_UTILS_SOURCES)
list(APPEND DUILIB_SOURCES ${DUILIB_ROOT_SOURCES} ${DUILIB_CORE_SOURCES}
    ${DUILIB_CONTROL_SOURCES} ${DUILIB_LAYOUT_SOURCES} ${DUILIB_UTILS_SOURCES})

add_library(duilib_static STATIC ${DUILIB_SOURCES})
target_compile_definitions(duilib_static PUBLIC UNICODE _UNICODE UILIB_STATIC)
target_include_directories(duilib_static PUBLIC
    "${DUILIB_ROOT}"
    "${DUILIB_ROOT}/Core"
    "${DUILIB_ROOT}/Control"
    "${DUILIB_ROOT}/Layout"
    "${DUILIB_ROOT}/Utils")
target_compile_options(duilib_static PRIVATE
    /source-charset:.936
    /W3
    /Zi
    /wd4996 /wd4267 /wd4244 /wd4838 /wd4018)
target_link_libraries(duilib_static PUBLIC comctl32 ole32 oleaut32 uuid imm32 shlwapi)

set(APP_SOURCES
    src/main.cpp
    src/ElementWindow.cpp
    src/ElementControls.cpp
    src/UiScale.cpp
    src/app.rc)

add_executable(DuiLibElement WIN32 ${APP_SOURCES})
target_include_directories(DuiLibElement PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/res")
target_compile_definitions(DuiLibElement PRIVATE UNICODE _UNICODE UILIB_STATIC NOMINMAX)
target_compile_options(DuiLibElement PRIVATE
    /utf-8
    /W3
    /Zi
    /wd4828 /wd4996 /wd4267 /wd4244)
target_link_libraries(DuiLibElement PRIVATE
    duilib_static
    comctl32 ole32 oleaut32 uuid imm32 shlwapi
    gdi32 user32 advapi32 shell32 psapi dbghelp)

set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} /nologo /I\"${CMAKE_CURRENT_SOURCE_DIR}/src\" /I\"${CMAKE_CURRENT_SOURCE_DIR}/res\"")

target_link_options(DuiLibElement PRIVATE /MANIFEST:NO /DEBUG)

set_target_properties(DuiLibElement PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    PDB_NAME "DuiLibElement"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
