file(READ "${CMAKE_CURRENT_LIST_DIR}/pack.json" PACK_JSON)
string(JSON PACK_NAME GET "${PACK_JSON}" name)

add_library(${PACK_NAME} STATIC
	src/rigImage.cpp
	src/rigImage.h
	src/Image.cpp
	src/Image.h
	src/Raster.h
	src/RasterPaint.cpp
	src/RasterPaint.h
)
target_link_libraries(${PACK_NAME} PUBLIC rigkit)
target_include_directories(${PACK_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
)
target_include_directories(${PACK_NAME} PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/third_party/stb
)
message(STATUS "Configured pack: ${PACK_NAME}")
