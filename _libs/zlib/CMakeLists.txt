cmake_minimum_required(VERSION 2.6)
project(zlib)

set(SRC_ZLIB
	${SRC_ZLIB}
	
	adler32.c
	compress.c
	crc32.c
	deflate.c
	gzio.c
	infback.c
	inffast.c
	inflate.c
	inftrees.c
	trees.c
	uncompr.c
	zutil.c
	crc32.h
	deflate.h
	inffast.h
	inffixed.h
	inflate.h
	inftrees.h
	trees.h
	zconf.h
	zlib.h
	zutil.h
)

include_directories(${CMAKE_CURRENT_LIST_DIR})
add_library( zlib STATIC ${SRC_ZLIB} )
set_target_properties(zlib PROPERTIES FOLDER "_library")