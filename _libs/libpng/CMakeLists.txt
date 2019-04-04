cmake_minimum_required(VERSION 2.6)
project(libpng)

set(SRC_LIBPNG
	${SRC_LIBPNG}
	
	png.c
	pngerror.c
	pnggccrd.c
	pngget.c
	pngmem.c
	pngpread.c
	pngread.c
	pngrio.c
	pngrtran.c
	pngrutil.c
	pngset.c
	pngtrans.c
	pngvcrd.c
	pngwio.c
	pngwrite.c
	pngwtran.c
	pngwutil.c
	png.h
	pngconf.h
)

include_directories(${CMAKE_CURRENT_LIST_DIR})
add_library( libpng STATIC ${SRC_LIBPNG} )
target_link_libraries(libpng zlib)
set_target_properties(libpng PROPERTIES FOLDER "_library")