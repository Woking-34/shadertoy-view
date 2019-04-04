cmake_minimum_required(VERSION 2.6)
project(shadertoy-view)

set( CMAKE_VERBOSE_MAKEFILE ON )
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/shadertoy-bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/shadertoy-lib)

IF(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
ENDIF()

string(TOUPPER ${CMAKE_CXX_COMPILER_ID} CXX_COMPILER_ID_UPPER)
if(${CXX_COMPILER_ID_UPPER} STREQUAL MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP")
endif()

add_definitions(-DGLEW_STATIC)
add_definitions(-DFREEGLUT_STATIC)

IF(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
	add_definitions(-D_USE_MATH_DEFINES)

	include_directories(
		_libs/freeglut
	)
	
	add_subdirectory(_libs/freeglut)
ENDIF(WIN32)

include_directories(
	_libs/glew
	_libs/zlib
	_libs/libpng
	_libs/libjpeg
	_libs/imago2
)

add_subdirectory(_libs/glew)
add_subdirectory(_libs/zlib)
add_subdirectory(_libs/libpng)
add_subdirectory(_libs/libjpeg)
add_subdirectory(_libs/imago2)

add_executable(shadertoy ./src/shadertoy.cc)

IF(WIN32)
	target_link_libraries(shadertoy freeglut)
ENDIF(WIN32)
IF(UNIX AND NOT APPLE)
	target_link_libraries(shadertoy glut GL)
ENDIF(UNIX AND NOT APPLE)

target_link_libraries(shadertoy glew)
target_link_libraries(shadertoy zlib)
target_link_libraries(shadertoy libpng)
target_link_libraries(shadertoy libjpeg)
target_link_libraries(shadertoy imago2)