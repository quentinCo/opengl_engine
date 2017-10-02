cmake_minimum_required(VERSION 3.9)

#[[
project(qc_graphic)

file(GLOB_RECURSE QC_GRAPHIC_SRC_FILES src/*.c src/*.cpp)
file(GLOB_RECURSE QC_GRAPHIC_INCLUDE_FILES include/qc_graphic/*.h include/qc_graphic/*.hpp)

include_directories(include/qc_graphic/)

add_library(qc_graphic ${SRC_FILES} ${INCLUDE_FILES})

install
(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/shaders/
    DESTINATION shaders/qc
    FILES_MATCHING PATTERN "*.glsl"
)

target_include_directories(qc_graphic PUBLIC .)
]]