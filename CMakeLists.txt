cmake_minimum_required(VERSION 2.8)

project(opengl)

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(
        -std=c++14
    )
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(GLMLV_USE_BOOST_FILESYSTEM "Use boost for filesystem library instead of experimental std lib" OFF)

if (GLMLV_USE_BOOST_FILESYSTEM)
    add_definitions(
        -DGLMLV_USE_BOOST_FILESYSTEM
    )
endif()

set(IMGUI_DIR imgui-1.49)
set(GLFW_DIR glfw-3.2.1)
set(GLM_DIR glm-0.9.8.3)
set(GLAD_DIR glad)
set(JSON_DIR json-2.0.9)
set(STB_DIR stb)
set(TINY_OBJ_LOADER_DIR tinyobjloader-1.0.5)
set(GLMLV_DIR glmlv)

add_subdirectory(third-party/${GLFW_DIR})

find_package(OpenGL REQUIRED)

if(GLMLV_USE_BOOST_FILESYSTEM)
    find_package(Boost COMPONENTS system filesystem REQUIRED)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(c2ba-glsl-shaders)
include(c2ba-copy-dll-post-build)
include(c2ba-assets-directory)

if(MSVC)
    set(SHADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/shaders)
    set(ASSET_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/\$\(Configuration\)/assets)
else()
    set(SHADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/shaders)
    set(ASSET_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin/assets)
endif()

include_directories(
    ${OPENGL_INCLUDE_DIRS}
    third-party/${GLFW_DIR}/include
    third-party/${GLM_DIR}
    third-party/${GLAD_DIR}/include
    third-party/${JSON_DIR}
    third-party/${IMGUI_DIR}
    third-party/${STB_DIR}
    third-party/${TINY_OBJ_LOADER_DIR}
	third-party/${GLMLV_DIR}/include
    lib/include
)

if(GLMLV_USE_BOOST_FILESYSTEM)
    include_directories (
        ${Boost_INCLUDE_DIRS}
    )
endif()

file(
    GLOB_RECURSE
    GLMLV_LIB_FILES
    third-party/${GLMLV_DIR}/*.cpp third-party/${GLMLV_DIR}/*.hpp
)


add_library(
    glmlv
    ${GLMLV_LIB_FILES}
)

file(
    GLOB_RECURSE
    QC_LIB_FILES
    lib/*.cpp lib/*.hpp lib/*.glsl lib/assets/*
)

add_library(
	qc
	${QC_LIB_FILES}
)


target_link_libraries(
    qc
    glmlv
)

c2ba_add_shader_directory(${CMAKE_CURRENT_SOURCE_DIR}/lib/shaders ${SHADER_OUTPUT_PATH}/qc)
c2ba_add_assets_directory(${CMAKE_CURRENT_SOURCE_DIR}/lib/assets ${ASSET_OUTPUT_PATH}/qc)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/shaders/
    DESTINATION shaders/qc
    FILES_MATCHING PATTERN "*.glsl"
)

set(
    IMGUI_SRC_FILES
    third-party/${IMGUI_DIR}/imgui.cpp
    third-party/${IMGUI_DIR}/imgui_demo.cpp
    third-party/${IMGUI_DIR}/imgui_draw.cpp
)

set(
    THIRD_PARTY_SRC_FILES
    ${IMGUI_SRC_FILES}
    third-party/${GLAD_DIR}/src/glad.c
)

set(
    LIBRARIES
    ${OPENGL_LIBRARIES}
    glfw
    glmlv
	qc
)

if(CMAKE_COMPILER_IS_GNUCXX AND NOT GLMLV_USE_BOOST_FILESYSTEM)
    set(LIBRARIES ${LIBRARIES} stdc++fs)
endif()

if (GLMLV_USE_BOOST_FILESYSTEM)
    set(LIBRARIES ${LIBRARIES} ${Boost_SYSTEM_LIBRARY}  ${Boost_FILESYSTEM_LIBRARY})
endif()

source_group ("glsl" REGULAR_EXPRESSION "*/*.glsl")
source_group ("third-party" REGULAR_EXPRESSION "third-party/*.*")

file(GLOB APP_DIRECTORIES "apps/engine")
get_filename_component(APP ${APP_DIRECTORIES} NAME)

file(
    GLOB_RECURSE
    SRC_FILES
    apps/${APP}/*.cpp apps/${APP}/*.hpp apps/${APP}/*.glsl apps/${APP}/assets/*
)

add_executable(
    ${APP}
    ${SRC_FILES}
    ${THIRD_PARTY_SRC_FILES}
)

target_link_libraries(
    ${APP}
    ${LIBRARIES}
)

install(
    TARGETS ${APP}
    DESTINATION .
)

c2ba_add_shader_directory(${APP_DIRECTORIES}/shaders ${SHADER_OUTPUT_PATH}/${APP})
c2ba_add_assets_directory(${APP_DIRECTORIES}/assets ${ASSET_OUTPUT_PATH}/${APP})

install(
    DIRECTORY ${APP_DIRECTORIES}/shaders/
    DESTINATION shaders/${APP}
    FILES_MATCHING PATTERN "*.glsl"
)

if(EXISTS ${APP_DIRECTORIES}/assets/)
    install(
        DIRECTORY ${APP_DIRECTORIES}/assets/
        DESTINATION assets/${APP}
    )
endif()