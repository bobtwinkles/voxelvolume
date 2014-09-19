cmake_minimum_required(VERSION 2.0)

project(srp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
list(APPEND CMAKE_CXX_DEBUG_FLAGS "-g")

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(SYSTEM ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})

add_executable(main Main.cpp DataStore.cpp RenderChunk.cpp)
add_executable(data_render DataRender.cpp DataStore.cpp)

target_link_libraries(main ${GLUT_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})
target_link_libraries(data_render ${GLUT_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})

install(DIRECTORY data DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
install(FILES gen.py DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
