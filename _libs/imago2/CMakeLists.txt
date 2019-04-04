cmake_minimum_required(VERSION 2.6)
project(imago2)

set(SRC_LIBPNG
	${SRC_LIBPNG}
	
	modules.c
	imago2.h
	imago2.c
	imago_gl.c
	ftype_module.h
	ftype_module.c
	file_tga.c
	file_rgbe.c
	file_ppm.c
	file_png.c
	file_jpeg.c
	conv.c
)

include_directories(${CMAKE_CURRENT_LIST_DIR})
add_library( imago2 STATIC ${SRC_LIBPNG} )
target_link_libraries(imago2 zlib)
target_link_libraries(imago2 libpng)
target_link_libraries(imago2 libjpeg)
set_target_properties(imago2 PROPERTIES FOLDER "_library")