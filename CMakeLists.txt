cmake_minimum_required(VERSION 3.1)
project(example)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "~/libigl/")
list(APPEND CMAKE_MODULE_PATH "~/libigl/cmake/")
message(${CMAKE_MODULE_PATH})

# libigl
option(LIBIGL_WITH_OPENGL            "Use OpenGL"         ON)
option(LIBIGL_WITH_OPENGL_GLFW       "Use GLFW"           ON)
option(LIBIGL_WITH_VIEWER            "Use OpenGL viewer"  ON)


find_package(LIBIGL REQUIRED QUIET)
find_package(CGAL REQUIRED)


# Add your project files
file(GLOB SRCFILES *.cpp )

add_executable(${PROJECT_NAME}_bin ${SRCFILES})
target_link_libraries(${PROJECT_NAME}_bin igl::core igl::opengl_glfw)
target_link_libraries(${PROJECT_NAME}_bin /usr/local/lib/libquadprog.a)
target_link_libraries(${PROJECT_NAME}_bin /usr/local/lib/libeigen-quadprog.so)
target_link_libraries(${PROJECT_NAME}_bin /usr/local/lib/triangle.a)
include_directories("${CGAL_INCLUDE_DIRS}")
target_link_libraries(${PROJECT_NAME}_bin ${CGAL_LIBRARIES})
