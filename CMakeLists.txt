cmake_minimum_required(VERSION 3.9)

project(opengl_engine)

# Lib files
	# qc_graphic
set(QC_GRAPHIC_DIR lib/qc_graphic)

file(GLOB_RECURSE
	QC_GRAPHIC_SRC_FILES
	${QC_GRAPHIC_DIR}/src/*.c
	${QC_GRAPHIC_DIR}/src/*.cpp
)

file(
	GLOB_RECURSE
	QC_GRAPHIC_INCLUDE_FILES
	${QC_GRAPHIC_DIR}/include/qc_graphic/*.h
	${QC_GRAPHIC_DIR}/include/qc_graphic/*.hpp
)

file(
	GLOB_RECURSE
	QC_GRAPHIC_SHADER_FILES
	${QC_GRAPHIC_DIR}/shaders/*.glsl
)

add_library(
	qc_graphic
	${QC_GRAPHIC_SRC_FILES}
	${QC_GRAPHIC_INCLUDE_FILES}
	${QC_GRAPHIC_SHADER_FILES}
)

source_group(
	"Shaders"
	FILES
	${QC_GRAPHIC_SHADER_FILES}
)

set(LIBRARIES ${LIBRARIES} qc_graphic)
add_subdirectory(${QC_GRAPHIC_DIR})
include_directories(${QC_GRAPHIC_DIR}/include)

	# third-parties
		# OpenGL
find_package(OpenGL REQUIRED)
set(LIBRARIES ${LIBRARIES} ${OPENGL_LIBRARIES})
include_directories(${OPENGL_INCLUDE_DIRS})

		# assimp
set(ASSIMP_DIR third-party/assimp)
set(LIBRARIES ${LIBRARIES} assimp)
add_subdirectory(${ASSIMP_DIR})
include_directories(
	${ASSIMP_DIR}/include
	${ASSIMP_DIR}/code
)

# Add the temporary output directories to the library path to make sure the
# Assimp library can be found, even if it is not installed system-wide yet.
#link_directories( ${Assimp_BINARY_DIR} ${AssetImporter_BINARY_DIR}/lib )

		# glad
set(GLAD_DIR third-party/glad)
#set(LIBRARIES ${LIBRARIES} glad)
include_directories(${GLAD_DIR}/include)

		# glfw
set(GLFW_DIR third-party/glfw)
set(LIBRARIES ${LIBRARIES} glfw)
add_subdirectory(${GLFW_DIR})
include_directories(${GLFW_DIR}/include)

		# glm
set(GLM_DIR third-party/glm)
#set(LIBRARIES ${LIBRARIES} glm)
include_directories(${GLM_DIR})

		# imgui
set(IMGUI_DIR third-party/imgui)
file(GLOB
	IMGUI_SRC_FILES
	${IMGUI_DIR}/*.cpp
)

file(
	GLOB
	IMGUI_INCLUDE_FILES
	${IMGUI_DIR}/*.h
)

add_library(
	imgui
	${IMGUI_SRC_FILES}
	${IMGUI_INCLUDE_FILES}
)
#set(LIBRARIES ${LIBRARIES} imgui)
include_directories(${IMGUI_DIR})

		# stb
set(STB_DIR third-party/stb)
#set(LIBRARIES ${LIBRARIES} stb)
include_directories(${STB_DIR})

# App files
set(APP_DIR "app/engine")

file(GLOB_RECURSE APP_SRC_FILES ${APP_DIR}/*.c ${APP_DIR}/*.cpp)
file(GLOB_RECURSE APP_INCLUDE_FILES ${APP_DIR}/*.h ${APP_DIR}/*.hpp)

# Executable
	# Create
add_executable(engine ${APP_SRC_FILES} ${APP_INCLUDE_FILES}) 

	# Link
target_link_libraries(engine ${LIBRARIES})

	# Copy useful file
install(
	DIRECTORY ${APP_DIR}/shaders/
	DESTINATION shaders/${APP}
	FILES_MATCHING PATTERN "*.glsl"
)