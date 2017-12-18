cmake_minimum_required(VERSION 2.8)

project(gjk)

SET(GFRAME_DIR ${PROJECT_SOURCE_DIR}/GFrame)
SET(GFRAME_STATIC_LIB ${GFRAME_DIR}/lib/libGFrame.a)
SET(SRC_FILES ${GFRAME_DIR}/src)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(GLFW REQUIRED glfw3)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${SRC_FILES})

add_executable(gjk gjk.cpp gjk.h)

target_link_libraries(gjk ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ${GFRAME_STATIC_LIB} "-framework Cocoa -framework IOKit -framework CoreVideo")

SET(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")

# install files (shaders/models) used by GJK project
INSTALL(DIRECTORY ${GFRAME_DIR}/assets DESTINATION ${PROJECT_SOURCE_DIR}/build)
INSTALL(FILES ${GFRAME_DIR}/shaders/vertex_shader_simple.glsl DESTINATION ${PROJECT_SOURCE_DIR}/build/shaders)
INSTALL(FILES ${GFRAME_DIR}/shaders/fragment_shader_color.glsl DESTINATION ${PROJECT_SOURCE_DIR}/build/shaders)
