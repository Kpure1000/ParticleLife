cmake_minimum_required(VERSION 3.8)
set(PROJECT_NAME ParticleLife)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED FALSE)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# define _CRT_SECURE_NO_WARNINGS with MSVC
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories(vendor/CSUGL/vendor/GLFW)
include_directories(vendor/CSUGL/vendor/glad/include)
include_directories(vendor/CSUGL/vendor/glm)
include_directories(vendor/CSUGL/vendor/stb)
include_directories(vendor/CSUGL/vendor/glog/src)
include_directories(vendor/CSUGL/vendor/imgui)

# csugl config
set(CSUGL_BUILD_MODEL_LOADER OFF)
add_subdirectory(vendor/CSUGL)
include_directories(vendor/CSUGL/src)

# project configure
include_directories(src/)
if (MSVC)
	# Set the startup project as AssignmentTest
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif (MSVC)

set(SOURCE_FILES 
    src/main.cpp
    src/particle.cpp
	src/quadtree.cpp
)

set(HEADER_FILES 
    src/particle.h
    src/quadtree.h
)

set(SHADER_FILES 
    assets/shader/particle2d.vert 
    assets/shader/particle.frag 
    assets/shader/particle2d.geom
    assets/shader/tree.vert
    assets/shader/tree.frag
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})
target_link_libraries(${PROJECT_NAME} CSUGL)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(ASSETS_PATH ${CMAKE_SOURCE_DIR}/assets)
	set(TARGET_PATH ${CMAKE_BINARY_DIR})
	message("copy " ${ASSETS_PATH} " to " ${TARGET_PATH})
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_PATH} ${TARGET_PATH}/assets
		DEPENDS ${PROJECT_SHADERS})
endif()